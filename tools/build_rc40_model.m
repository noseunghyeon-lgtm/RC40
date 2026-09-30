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
        if strcmp(cfg.portSelection,'assigned') && isempty(strtrim(p.masar))
            nSkipped=nSkipped+1; continue;
        end
        portName = choosePortName(p, cfg);
        portName = appendTypeSuffix(portName, m.dataType, cfg);   % e.g. _u16, _l

        % --- subsystem group (e.g. Input_AnU, Output_PropPwr) ---
        grp = sprintf('%s_%s', ternary(m.direction,"In","Input","Output"), m.hwArray);
        sub = ensureSubsystem(name, grp, subs);

        % --- create the port block (robust across releases) ---
        blk = sprintf('%s/%s', sub, portName);
        blk = addPortBlock(blk, m.blockType);   % may append _N if name clashes

        % apply characteristics
        set_param(blk, 'OutDataTypeStr', m.dataType);
        setPortDims(blk, cfg);                 % PortDimensions (e.g. '1')
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
            if isfield(cfg,'canPortNaming') && strcmpi(cfg.canPortNaming,'message')
                portName = sanitize(sprintf('%s_%s', S.message, S.name));
            else
                portName = sanitize(S.name);                      % signal-level name
            end
            portName = appendTypeSuffix(portName, S.dataType, cfg); % e.g. _u8, _u16
            blk = sprintf('%s/%s', sub, portName);
            blk = addPortBlock(blk, blockType);   % may append _N if name clashes
            dt = S.dataType;                 % already a Simulink type string
            set_param(blk,'OutDataTypeStr',dt);
            setPortDims(blk, cfg);             % PortDimensions (e.g. '1')
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
function setPortDims(blk, cfg)
    % Stamp PortDimensions on the port (matches the manual model's
    % "포트 차원" field). Applies to Inport; Outport ignores gracefully.
    if ~(isfield(cfg,'portDimensions') && ~isempty(cfg.portDimensions)), return; end
    try
        set_param(blk, 'PortDimensions', cfg.portDimensions);
    catch
        % Outport (or a release without the param): ignore.
    end
end

function sigs = loadCanSignals(cfg)
%LOADCANSIGNALS  Return a normalized CAN signal list from the configured source.
%   Each element: .message .node .name .dataType(Simulink) .id .extended
%                 .startBit .length .signed .byteOrder .factor .offset
%                 .min .max .unit .dir('Tx'|'Rx' when known)
    % Fixed field set. Every element is built via makeSig() so struct-array
    % assignment never fails on mismatched/extra fields.
    sigs = makeSig();   % 0x0 template

    switch lower(cfg.canSource)
        case 'excel'
            if exist(cfg.canExcel,'file')~=2
                warning('rc40:CanExcelMissing', ...
                    'CAN Excel not found: %s -- skipping CAN ports. Set cfg.canExcel or cfg.includeCommPins=false.', ...
                    cfg.canExcel);
                return;
            end
            raw = rc40_read_can_excel(cfg.canExcel, cfg.canExcelSheet);
            for i=1:numel(raw)
                r = raw(i);
                if strcmpi(cfg.canValueMode,'raw')
                    dt = masarTypeToSimulink(r.dataType, 1, 0);          % raw integer type
                else
                    dt = masarTypeToSimulink(r.dataType, r.factor, r.offset);
                end
                s = makeSig(r.message, r.node, r.name, dt, r.id, r.extended, ...
                    r.startBit, r.length, r.signed, r.byteOrder, ...
                    r.factor, r.offset, r.min, r.max, r.unit, r.dir, {});
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
                        if S.byteOrder==1, bo='Intel'; else, bo='Motorola'; end
                        dt = canSignalDataType(S, cfg);
                        s = makeSig(M.name, M.txNode, S.name, dt, M.id, M.extended, ...
                            S.startBit, S.len, S.signed, bo, ...
                            S.factor, S.offset, S.min, S.max, S.unit, '', S.receivers);
                        sigs(end+1) = s; %#ok<AGROW>
                    end
                end
            end
        otherwise
            error('rc40:CanSource','Unknown cfg.canSource: %s', cfg.canSource);
    end
end

function s = makeSig(message,node,name,dataType,id,extended,startBit,length_, ...
                     signed,byteOrder,factor,offset,mn,mx,unit,dir,receivers)
%MAKESIG  Build a normalized CAN-signal struct with a FIXED field set.
%   makeSig() with no args returns a 0x0 struct template (for preallocation).
    if nargin==0
        s = struct('message',{},'node',{},'name',{},'dataType',{},'id',{}, ...
            'extended',{},'startBit',{},'length',{},'signed',{},'byteOrder',{}, ...
            'factor',{},'offset',{},'min',{},'max',{},'unit',{},'dir',{},'receivers',{});
        return;
    end
    s = struct('message',message,'node',node,'name',name,'dataType',dataType, ...
        'id',id,'extended',extended,'startBit',startBit,'length',length_, ...
        'signed',signed,'byteOrder',byteOrder,'factor',factor,'offset',offset, ...
        'min',mn,'max',mx,'unit',unit,'dir',dir,'receivers',{receivers});
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

function blk = addPortBlock(blk, blockType)
    % Add an Inport/Outport using the always-valid built-in path.
    % If the name already exists in this subsystem (possible with 450+ CAN
    % signals sharing a name across messages), append _2, _3, ... to keep it
    % unique instead of erroring out.
    orig = blk; n = 1;
    while getSimulinkBlockExists(blk)
        n = n + 1;
        blk = sprintf('%s_%d', orig, n);
    end
    add_block(['built-in/' blockType], blk);
end

function tf = getSimulinkBlockExists(blk)
    tf = false;
    try
        tf = getSimulinkBlockHandle(blk) > 0;   % R2018a+
    catch
        try
            find_system(blk,'SearchDepth',0);  %#ok<FNDSB>
            tf = true;
        catch
            tf = false;
        end
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
    % base name: MASAR assignment when present, else physical pin name
    if strcmp(cfg.portSelection,'all')
        if ~isempty(strtrim(p.masar)), base = sanitize(p.masar); else, base = p.name; end
    else
        base = sanitize(p.masar);
    end
    % optional Pull-Down / Pull-Up prefix from Description (PD_ / PU_)
    if isfield(cfg,'usePullPrefix') && cfg.usePullPrefix
        d = lower(p.desc);
        if contains(d,'pull-down') || contains(d,'pull down') || contains(d,'pulldown')
            base = ['PD_' base];
        elseif contains(d,'pull-up') || contains(d,'pull up') || contains(d,'pullup')
            base = ['PU_' base];
        end
    end
    nm = base;
end

function nm = appendTypeSuffix(nm, dataType, cfg)
    % Append data-type suffix to a port name, e.g. SteeringAngle -> SteeringAngle_u16.
    if ~(isfield(cfg,'appendTypeSuffix') && cfg.appendTypeSuffix), return; end
    sfx = rc40_type_suffix(dataType);
    if ~isempty(sfx) && ~endsWith(nm, ['_' sfx])
        nm = [nm '_' sfx];
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

function dt = canSignalDataType(S, cfg)
    bits = S.len;
    if S.signed
        if bits<=8, dt='int8'; elseif bits<=16, dt='int16'; elseif bits<=32, dt='int32'; else, dt='int64'; end
    else
        if bits<=8, dt='uint8'; elseif bits<=16, dt='uint16'; elseif bits<=32, dt='uint32'; else, dt='uint64'; end
    end
    % 'phys' mode -> scaled signals become single; 'raw' keeps integer type.
    if nargin>1 && strcmpi(cfg.canValueMode,'phys') && (S.factor~=1 || S.offset~=0)
        dt='single';
    end
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
