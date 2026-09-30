function build_rc40_model(cfg)
%BUILD_RC40_MODEL  Generate a Simulink model of the RC40 controller with
%                  Input/Output ports auto-created from RC40_Pinmap.xlsx,
%                  each port carrying the data-type characteristics of its
%                  signal, plus CAN Tx/Rx ports from DBC file(s).
%
%   build_rc40_model()        uses rc40_config()
%   build_rc40_model(cfg)     uses the supplied config struct
%
%   The controller is the "Top": one subsystem per signal group holds the
%   Inport/Outport blocks; each port's Name, direction and data type are
%   set from the pin definition (Type, In/Out, Description) via RC40_TYPEMAP.
%
%   Requires: MATLAB + Simulink. No other toolbox.
%   See README.md for the full workflow.

    if nargin < 1 || isempty(cfg)
        cfg = rc40_config();
    end

    fprintf('== RC40 model builder ==\n');
    assert(exist(cfg.pinmapXlsx,'file')==2, 'Pinmap not found: %s', cfg.pinmapXlsx);

    % ---- 1. read pin definitions ------------------------------------------------
    pins = rc40_read_pinmap(cfg.pinmapXlsx);
    fprintf('Read %d pin rows from %s\n', numel(pins), cfg.pinmapXlsx);

    % ---- 2. (re)create model ----------------------------------------------------
    name = cfg.modelName;
    if bdIsLoaded(name),  close_system(name,0); end
    if exist(cfg.outputSlx,'file')==2, delete(cfg.outputSlx); end
    new_system(name);  load_system(name);

    % Group -> subsystem path (created lazily)
    subs = containers.Map('KeyType','char','ValueType','char');
    counters = containers.Map('KeyType','char','ValueType','double'); % y position per subsystem

    nMade = 0; nSkipped = 0; report = {};

    % ---- 3. signal ports --------------------------------------------------------
    for i = 1:numel(pins)
        p = pins(i);

        includeSection = strcmp(p.section,'Signal') || ...
            (cfg.includePowerPins && strcmp(p.section,'Power')) || ...
            (cfg.includeCommPins  && strcmp(p.section,'Communication'));
        if ~includeSection, continue; end

        % Communication (CAN/LIN/Ethernet) handled separately from DBC below.
        if strcmp(p.section,'Communication'), continue; end
        if strcmp(p.section,'Power'), 
            % Simple pass-through: treat supply/ground as unrecognized unless enabled
            % (kept minimal; power pins rarely become model ports)
            continue;
        end

        m = rc40_typemap(p.type, p.inout, p.desc);
        m = applyOverride(m, p.name, cfg);   % per-pin corrections (Excel gaps / MASAR reconfig)
        if ~m.valid
            msg = sprintf('Unrecognized Type/IO: %s (Type=%s, IO=%s)', p.name, p.type, p.inout);
            if strcmp(cfg.onUnknownType,'error')
                error('rc40:UnknownType','%s', msg);
            else
                warning('rc40:UnknownType','%s -- skipped', msg); nSkipped=nSkipped+1;
                report{end+1}=['SKIP  ' msg]; %#ok<AGROW>
                continue;
            end
        end

        % --- port selection policy ---
        portName = choosePortName(p, cfg);
        if strcmp(cfg.portSelection,'assigned') && isempty(strtrim(p.masar))
            nSkipped=nSkipped+1; continue;
        end

        % --- subsystem group (e.g. Input_AnU, Output_PropPwr) ---
        grp = sprintf('%s_%s', ternary(m.direction,"In","Input","Output"), m.hwArray);
        sub = ensureSubsystem(name, grp, subs);

        % --- create the port block (robust across releases) ---
        blk = sprintf('%s/%s', sub, portName);
        addPortBlock(blk, m.blockType);

        % apply characteristics
        set_param(blk, 'OutDataTypeStr', m.dataType);
        % MASAR reference as a description annotation on the port
        masarRef = masarReference(p, m, cfg);
        set_param(blk, 'Description', sprintf(['Pin=%s\nType=%s\nMASAR=%s\n' ...
            'HW=%s (%s)\nUnit=%s'], p.name, p.type, masarRef, m.hwClass, m.hwArray, m.unit));

        % auto layout
        placePort(blk, m.direction, cfg, counters, sub);

        nMade = nMade + 1;
        report{end+1} = sprintf('%-4s %-6s -> %-10s %-8s %-14s %s', ...
            m.direction, p.name, portName, m.dataType, m.hwClass, masarRef); %#ok<AGROW>
    end

    % ---- 4. CAN ports (MASAR Excel primary; DBC fallback) -----------------------
    if cfg.includeCommPins
        canSigs = loadCanSignals(cfg);   % normalized list regardless of source
        fprintf('CAN signals loaded (%s): %d\n', cfg.canSource, numel(canSigs));
        for i = 1:numel(canSigs)
            S = canSigs(i);
            % Tx/Rx decision delegated to the single pluggable function.
            [dir, grp] = rc40_can_direction(S, cfg);
            if isempty(dir), continue; end
            if strcmp(dir,'Out'), blockType='Outport'; else, blockType='Inport'; end
            sub = ensureSubsystem(name, grp, subs);
            portName = sanitize(sprintf('%s_%s', S.message, S.name));
            blk = sprintf('%s/%s', sub, portName);
            addPortBlock(blk, blockType);
            dt = S.dataType;                 % already a Simulink type string
            set_param(blk,'OutDataTypeStr',dt);
            set_param(blk,'Description',sprintf(['CAN msg=%s id=0x%X %s node=%s dir=%s\n' ...
                'sig=%s start=%d len=%d factor=%g offset=%g [%g..%g] %s'], ...
                S.message, S.id, tern(S.extended,'ext','std'), S.node, dir, ...
                S.name, S.startBit, S.length, S.factor, S.offset, S.min, S.max, S.unit));
            placePort(blk, dir, cfg, counters, sub);
            nMade=nMade+1;
            report{end+1}=sprintf('CAN  %-4s %-28s -> %-8s (id 0x%X)', dir, portName, dt, S.id); %#ok<AGROW>
        end
    end

    % ---- 5. save + report -------------------------------------------------------
    save_system(name, cfg.outputSlx);
    fprintf('\nSaved %s  (ports created: %d, skipped: %d)\n', cfg.outputSlx, nMade, nSkipped);
    fprintf('---- generation report ----\n%s\n', strjoin(report, sprintf('\n')));
    close_system(name, 1);
end

% ============================================================================
% helpers
% ============================================================================
function sigs = loadCanSignals(cfg)
%LOADCANSIGNALS  Return a normalized CAN signal list from the configured source.
%   Each element: .message .node .name .dataType(Simulink) .id .extended
%                 .startBit .length .signed .byteOrder .factor .offset
%                 .min .max .unit .dir('Tx'|'Rx' when known)
    sigs = struct('message',{},'node',{},'name',{},'dataType',{},'id',{}, ...
        'extended',{},'startBit',{},'length',{},'signed',{},'byteOrder',{}, ...
        'factor',{},'offset',{},'min',{},'max',{},'unit',{},'dir',{});

    switch lower(cfg.canSource)
        case 'excel'
            raw = rc40_read_can_excel(cfg.canExcel, cfg.canExcelSheet);
            for i=1:numel(raw)
                r = raw(i); s = r;
                s.dataType = masarTypeToSimulink(r.dataType, r.factor, r.offset);
                sigs(end+1) = s; %#ok<AGROW>
            end
        case 'dbc'
            list = resolveDbcList(cfg);
            for k=1:numel(list)
                msgs = rc40_read_dbc(list{k});
                for mi=1:numel(msgs)
                    M=msgs(mi);
                    for si=1:numel(M.signals)
                        S=M.signals(si);
                        s.message=M.name; s.node=M.txNode; s.name=S.name;
                        s.id=M.id; s.extended=M.extended;
                        s.startBit=S.startBit; s.length=S.len; s.signed=S.signed;
                        if S.byteOrder==1, s.byteOrder='Intel'; else, s.byteOrder='Motorola'; end
                        s.factor=S.factor; s.offset=S.offset; s.min=S.min; s.max=S.max;
                        s.unit=S.unit; s.dir='';                 % unknown -> node mode decides
                        s.dataType = canSignalDataType(S);
                        s.receivers = S.receivers; s.txNode = M.txNode; %#ok<STRNU>
                        sigs(end+1) = s; %#ok<AGROW>
                    end
                end
            end
        otherwise
            error('rc40:CanSource','Unknown cfg.canSource: %s', cfg.canSource);
    end
end

function dt = masarTypeToSimulink(masarType, factor, offset)
    t = lower(strtrim(masarType));
    switch t
        case 'u8',  dt='uint8';  case 'u16', dt='uint16'; case 'u32', dt='uint32'; case 'u64', dt='uint64';
        case 'i8',  dt='int8';   case 'i16', dt='int16';  case 'i32', dt='int32';  case 'i64', dt='int64';
        otherwise,  dt='uint16';
    end
    if (factor~=1 || offset~=0), dt='single'; end   % scaled -> physical value
end

function m = applyOverride(m, pinName, cfg)
    if ~isfield(cfg,'overrides') || ~isKey(cfg.overrides, pinName), return; end
    o = cfg.overrides(pinName);
    f = fieldnames(o);
    for i = 1:numel(f)
        m.(f{i}) = o.(f{i});
        if strcmp(f{i},'direction')
            if strcmp(o.direction,'In'), m.blockType='Inport'; else, m.blockType='Outport'; end
        end
    end
    m.valid = true;
end

function sub = ensureSubsystem(model, grp, subs)
    if isKey(subs, grp)
        sub = subs(grp); return;
    end
    sub = sprintf('%s/%s', model, grp);
    add_block('built-in/Subsystem', sub);
    % clear the default In1->Out1 that Simulink adds
    try, delete_line(sub,'In1/1','Out1/1'); catch, end
    try, delete_block([sub '/In1']); catch, end
    try, delete_block([sub '/Out1']); catch, end
    subs(grp) = sub; %#ok<NASGU>
end

function addPortBlock(blk, blockType)
    % robust add for Inport/Outport across releases
    try
        add_block(['built-in/' blockType], blk);
    catch
        add_block(['simulink/Ports & Subsystems/' blockType], blk);
    end
end

function placePort(blk, direction, cfg, counters, sub)
    key = [sub '|' direction];
    if isKey(counters,key), n = counters(key); else, n = 0; end
    y = cfg.layout.y0 + n*cfg.layout.dy;
    if strcmp(direction,'In'), x = cfg.layout.x_in; else, x = cfg.layout.x_out; end
    set_param(blk,'Position',[x y x+cfg.layout.w y+cfg.layout.h]);
    counters(key) = n+1; %#ok<NASGU>
end

function nm = choosePortName(p, cfg)
    if strcmp(cfg.portSelection,'all')
        if ~isempty(strtrim(p.masar)), nm = sanitize(p.masar); else, nm = p.name; end
    else
        nm = sanitize(p.masar);
    end
end

function ref = masarReference(p, m, cfg)
    % Build the MASAR struct-array reference string, e.g.
    %   AnU_as[DevInp_A13_D]        (input)
    %   PropPwr_as[DevOutp_A31HS_D] (output, HS/LS suffix per convention)
    pin = p.name;
    if strcmp(m.direction,'In')
        ref = sprintf('%s_as[DevInp_%s_D]', m.hwArray, pin);
    else
        % HS/LS suffix is a MASAR-internal convention applied ONLY to
        % proportional POWER outputs (PropPwr). Signal/low-power outputs
        % (PropSig, DigSig, AbsltU) never carry the suffix.
        suffix = '';
        if cfg.useHsLsSuffix && strcmp(m.hwArray,'PropPwr') && ~isempty(m.side)
            suffix = m.side;
        end
        ref = sprintf('%s_as[DevOutp_%s%s_D]', m.hwArray, pin, suffix);
    end
end

function dt = canSignalDataType(S)
    bits = S.len;
    if S.signed
        if bits<=8, dt='int8'; elseif bits<=16, dt='int16'; elseif bits<=32, dt='int32'; else, dt='int64'; end
    else
        if bits<=8, dt='uint8'; elseif bits<=16, dt='uint16'; elseif bits<=32, dt='uint32'; else, dt='uint64'; end
    end
    if S.factor ~= 1 || S.offset ~= 0, dt='single'; end % scaled -> physical
end

function list = resolveDbcList(cfg)
    if ~isempty(cfg.dbcFiles)
        list = cfg.dbcFiles; return;
    end
    list = {};
    if isfolder(cfg.dbcFolder)
        d = dir(fullfile(cfg.dbcFolder,'*.dbc'));
        for i=1:numel(d), list{end+1}=fullfile(d(i).folder,d(i).name); end %#ok<AGROW>
    end
end

function s = sanitize(s)
    s = regexprep(strtrim(s), '[^\w]', '_');
    if isempty(s), s='port'; end
end

function out = ternary(val, testTrue, a, b)
    if strcmp(val, testTrue), out=a; else, out=b; end
end
function out = tern(c,a,b), if c, out=a; else, out=b; end, end
