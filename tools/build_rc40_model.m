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

    nMade = 0; nSkipped = 0; nUnused = 0; report = {};
    hwPins = struct('dir',{},'hwArray',{},'index',{},'portName',{});  % for BSW snippets

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

        % --- multi-port: the C-column (Type) selects the pin function ---
        % Warn if the selected Type is not one of the functions this pin supports,
        % and derive the direction from the selected function for multi-port pins
        % (e.g. K62: Digital/Voltage -> IN, Analog output -> OUT).
        [allowed, isMulti] = rc40_pin_functions(p.desc, '');   % Description-derived only
        io = p.inout;
        if isMulti
            if strcmpi(p.type,'PWMSignal'), io = 'OUT'; else, io = 'IN'; end
            if ~any(strcmpi(allowed, p.type))
                warning('rc40:FuncNotSupported', ...
                    '%s: Type "%s" is not a function of this pin (allowed: %s)', ...
                    p.name, p.type, strjoin(allowed, ', '));
            end
        end

        % --- Use flag (H column): OFF -> generated into the Unused subsystem ---
        isUsed = ~strcmpi(strtrim(getfielddef(p,'use','ON')), 'OFF');

        m = rc40_typemap(p.type, io, p.desc);
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
        portName = choosePortName(p, m, cfg);
        portName = appendTypeSuffix(portName, m.dataType, cfg);   % e.g. _u16, _l

        % --- subsystem group (path relative to model) ---
        top = ternary(m.direction,"In","HwInp","HwOutp");
        switch lower(getfielddef(cfg,'hwGrouping','nested'))
            case 'byclass'
                % flat, one subsystem per HW class (Input_AnU, Output_PropPwr, ...)
                grp = sprintf('%s_%s', ternary(m.direction,"In","Input","Output"), m.hwArray);
            case 'hwinout'
                % flat, single HwInp / HwOutp for everything
                grp = top;
            otherwise % 'nested'
                % two-level: HwInp/<class> or HwOutp/<class>, chosen by the
                % pin setting (Type -> HW class), mirroring the header layout.
                grp = sprintf('%s/%s', top, m.hwArray);
        end
        if ~isUsed
            grp = sprintf('%s/%s', getfielddef(cfg,'unusedGroup','Unused'), grp);
        end
        sub = ensureSubsystem(name, grp, subs, cfg);

        % --- create the port block (robust across releases) ---
        blk = sprintf('%s/%s', sub, portName);
        blk = addPortBlock(blk, m.blockType);   % may append _N if name clashes

        % apply characteristics
        set_param(blk, 'OutDataTypeStr', m.dataType);
        setPortDims(blk, cfg);                 % PortDimensions (e.g. '1')
        % MASAR reference as a description annotation on the port
        masarRef = masarReference(p, m, cfg);          % e.g. PropSig_as[DevOutp_K81_D]
        set_param(blk, 'Description', sprintf(['Pin=%s\nType=%s\nMASAR=%s\n' ...
            'HW=%s (%s)\nUnit=%s'], p.name, p.type, masarRef, m.hwClass, m.hwArray, m.unit));

        % remember for BSW snippet generation (used pins only)
        if isUsed
            idx = regexp(masarRef, '\[(.*)\]', 'tokens', 'once');
            hwPins(end+1) = struct('dir',m.direction,'hwArray',m.hwArray, ...
                'index',idx{1},'portName',portName); %#ok<AGROW>
        else
            nUnused = nUnused + 1;
        end

        % auto layout
        placePort(blk, m.direction, cfg, counters, sub);

        nMade = nMade + 1;
        report{end+1} = sprintf('%-4s %-6s -> %-22s %-8s %-14s %s%s', ...
            m.direction, p.name, portName, m.dataType, m.hwClass, masarRef, ...
            tern(isUsed,'','  [UNUSED]')); %#ok<AGROW>
    end

    % ---- 4. CAN ports (MASAR Excel primary; DBC fallback) -----------------------
    if cfg.includeCommPins
        canSigs = loadCanSignals(cfg);   % normalized list regardless of source
        fprintf('CAN signals loaded (%s): %d\n', cfg.canSource, numel(canSigs));
        canSigs = sortCanSignals(canSigs, cfg);   % per-message ordering (e.g. alphabetical)
        for i = 1:numel(canSigs)
            S = canSigs(i);
            % Tx/Rx decision delegated to the single pluggable function.
            [dir, grp] = rc40_can_direction(S, cfg);
            if isempty(dir), continue; end
            if strcmp(dir,'Out'), blockType='Outport'; else, blockType='Inport'; end
            if ~(isfield(cfg,'canGrouping') && strcmpi(cfg.canGrouping,'flat'))
                grp = sprintf('%s/%s', grp, canMessageGroupName(S, cfg));
            end
            sub = ensureSubsystem(name, grp, subs, cfg);
            if isfield(cfg,'canPortNaming') && strcmpi(cfg.canPortNaming,'message')
                portName = sanitize(sprintf('%s_%s', S.message, S.name));
            else
                portName = sanitize(S.name);                      % signal-level name
            end
            portName = appendTypeSuffix(portName, S.dataType, cfg, 'canAppendTypeSuffix'); % e.g. _u8, _I
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

    % ---- 4b. BSW integration snippets ------------------------------------------
    if isfield(cfg,'genBswSnippets') && cfg.genBswSnippets
        writeBswSnippets(hwPins, cfg);
    end

    % ---- 5. save + report -------------------------------------------------------
    save_system(name, cfg.outputSlx);
    fprintf('\nSaved %s  (ports created: %d, of which unused: %d, skipped: %d)\n', ...
        cfg.outputSlx, nMade, nUnused, nSkipped);
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

function nm = canMessageGroupName(S, cfg)
    % Subsystem name for one CAN message, e.g. 'VC_Streaming_Status_0x18FF61BC'.
    nm = sanitize(S.message);
    if isfield(cfg,'canSubsystemIncludeId') && cfg.canSubsystemIncludeId
        nm = sprintf('%s_0x%X', nm, S.id);
    end
end

function sigs = sortCanSignals(sigs, cfg)
%SORTCANSIGNALS  Order CAN signals within each message.
%   The ASW Bus Assignment blocks list a message's signals alphabetically
%   (case-insensitive) by signal name, regardless of their DBC/Excel
%   declaration order. Sorting here makes the generated Inport/Outport
%   stack match that Bus Assignment order 1:1.
%   cfg.canSignalOrder: 'alpha' (default) | 'declared' (keep source order).
    if isempty(sigs), return; end
    mode = lower(getfielddef(cfg,'canSignalOrder','alpha'));
    if strcmp(mode,'declared'), return; end

    messages = {sigs.message};
    names    = lower({sigs.name});
    % stable sort: primary key = message (keeps messages grouped together in
    % their first-seen order), secondary key = signal name (alphabetical).
    [~,~,msgIdx] = unique(messages,'stable');
    [~, nameOrder] = sort(names);
    [~, order] = sort(msgIdx(nameOrder));
    order = nameOrder(order);
    sigs = sigs(order);
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
                    dt = masarTypeToSimulink(r.dataType, 1, 0, r.length, r.signed, r.name);   % raw integer type
                else
                    dt = masarTypeToSimulink(r.dataType, r.factor, r.offset, r.length, r.signed, r.name);
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

function dt = masarTypeToSimulink(masarType, factor, offset, bitLen, isSigned, sigName)
    % masarType gives the WIDTH (8/16/32/64 bits, from its trailing digits);
    % the AD "Signed"/"Unsigned" column (isSigned) is the AUTHORITY on sign,
    % per user requirement -- do not infer sign from the masarType letter
    % prefix alone (the Dataset sheet uses both 'i' and 's' prefixes for
    % signed, e.g. i16 and s16, and 'otherwise' silently defaulted to
    % uint16, which is the bug this fixes).
    %
    % bitLen (optional): DBC/MASAR signal bit length. A 8-bit unsigned signal
    % with bitLen==1 is a single-bit flag -> treated as boolean (suffix '_I').
    % isSigned (optional, default false): value of the Dataset "Signed" column.
    % sigName (optional): used only to make warnings traceable to a signal.

    t = lower(strtrim(masarType));
    widthTok = regexp(t, '(\d+)$', 'tokens', 'once');
    if isempty(widthTok)
        warning('rc40:CanDataType', '%sUnrecognized MASAR Data Type "%s" -- defaulting to uint16.', ...
            ternary2(nargin>=6 && ~isempty(sigName), [sigName ': '], ''), masarType);
        dt = 'uint16';
        return;
    end
    width = widthTok{1};

    if nargin < 5, isSigned = false; end
    % Flag an inconsistent sheet (Data Type letter disagrees with the Signed
    % column) instead of silently picking one; the Signed column still wins.
    typeSaysSigned = startsWith(t,'i') || startsWith(t,'s');
    if typeSaysSigned ~= logical(isSigned)
        warning('rc40:CanSignedMismatch', ...
            '%sData Type "%s" and Signed column ("%s") disagree -- using Signed column.', ...
            ternary2(nargin>=6 && ~isempty(sigName), [sigName ': '], ''), masarType, ...
            ternary2(isSigned,'Signed','Unsigned'));
    end

    if isSigned, dt = ['int' width]; else, dt = ['uint' width]; end
    if ~any(strcmp(dt, {'int8','int16','int32','int64','uint8','uint16','uint32','uint64'}))
        warning('rc40:CanDataType', '%sUnsupported width "%s" (from "%s") -- defaulting to uint16.', ...
            ternary2(nargin>=6 && ~isempty(sigName), [sigName ': '], ''), width, masarType);
        dt = 'uint16';
        return;
    end

    if nargin>=4 && strcmp(dt,'uint8') && bitLen==1
        dt='boolean';
    elseif (factor~=1 || offset~=0)
        dt='single';   % scaled -> physical value
    end
end

function s = ternary2(cond, a, b)
    if cond, s = a; else, s = b; end
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

function sub = ensureSubsystem(model, grp, subs, cfg)
    % grp may be a nested path relative to the model, e.g. 'HwInp/AnU'.
    % Each level is created once; parents are created before children.
    if isKey(subs, grp)
        sub = subs(grp); return;
    end
    parts = strsplit(grp, '/');
    path = model;
    for i = 1:numel(parts)
        rel = strjoin(parts(1:i), '/');           % path relative to model
        if ~isKey(subs, rel)
            full = sprintf('%s/%s', model, rel);
            add_block('built-in/Subsystem', full);
            % clear the default In1->Out1 that Simulink adds to a new Subsystem
            try, delete_line(full,'In1/1','Out1/1'); catch, end
            try, delete_block([full '/In1']); catch, end
            try, delete_block([full '/Out1']); catch, end
            subs(rel) = full; %#ok<NASGU>
            % add a short usage guide annotation for HW class leaf subsystems
            if nargin>=4 && isfield(cfg,'addClassGuide') && cfg.addClassGuide
                addClassGuide(full, parts{i});
            end
        end
        path = subs(rel); %#ok<NASGU>
    end
    sub = subs(grp);
end

function writeBswSnippets(hwPins, cfg)
%WRITEBSWSNIPPETS  Emit ready-to-use BSW access code for every HW pin.
%   Groups by direction+class and writes <modelName>_bsw_integration.c next
%   to the .slx. Outputs write to .inp_s; inputs read from .outp_s.
    [d,~,~] = fileparts(cfg.outputSlx);
    outFile = fullfile(d, [cfg.modelName '_bsw_integration.c']);
    fid = fopen(outFile,'w');
    if fid<0, warning('rc40:BswFile','Cannot write %s', outFile); return; end
    c = onCleanup(@() fclose(fid));

    fprintf(fid, '/* Auto-generated BSW integration snippets for %s\n', cfg.modelName);
    fprintf(fid, '   Outputs: write setpoints to .inp_s   Inputs: read measured from .outp_s\n');
    fprintf(fid, '   Replace <...> placeholders with application signals. */\n\n');

    % stable order: outputs first then inputs, grouped by class
    dirs = {'Out','In'};
    for di = 1:numel(dirs)
        D = dirs{di};
        classes = uniqueClasses(hwPins, D);
        for ci = 1:numel(classes)
            cls = classes{ci};
            fprintf(fid, '/* ===== %s / %s ===== */\n', ternary(D,'Out','HwOutp','HwInp'), cls);
            for i = 1:numel(hwPins)
                if ~strcmp(hwPins(i).dir,D) || ~strcmp(hwPins(i).hwArray,cls), continue; end
                emitPinSnippet(fid, hwPins(i));
            end
            fprintf(fid, '\n');
        end
    end
    fprintf('BSW snippets written: %s\n', outFile);
end

function emitPinSnippet(fid, hp)
    % Match the exact pattern used in Os10msProc.c:
    %   <base>.stErrReactn_e = ErrReactn_s.outp_s.ErrReactn_s.<arr>_as_<idx>_stErrReactn_e;
    %   <base>.flgSp_l = ...;
    %   <base>.<value field> = <application source>;
    idx = hp.index;                                   % e.g. DevOutp_K81_D
    if strcmp(hp.dir,'Out')
        base = sprintf('HwOutp_s.%s_as[%s].inp_s', hp.hwArray, idx);
        errR = sprintf('ErrReactn_s.outp_s.ErrReactn_s.%s_as_%s_stErrReactn_e', hp.hwArray, idx);
        % application source signals: PropSig/PropPwr duty -> PO_s, digital -> DO_s
        srcPO = sprintf('Veh_s.GW1_Core_s.outp_s.PO_s.%s', hp.portName);   % e.g. LS_K81_APP_...
        srcDO = sprintf('Veh_s.GW1_Core_s.outp_s.DO_s.%s', hp.portName);   % e.g. HS_A31_...
        switch hp.hwArray
            case 'PropPwr'
                % Current-controlled / PWM power output.
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.flgSp_l = (bool)%s;\n', base, srcDO);
                fprintf(fid, '%s.iSp_mA_u16 = /* <%s current mA, or dead value if Fct*Ctrl mode> */;\n', base, hp.portName);
                fprintf(fid, '%s.dutyCycSp_perml_u16 = /* <%s duty 0.1%%, or dead value> */;\n', base, hp.portName);
            case 'PropSig'
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.flgSp_l = TRUE;\n', base);
                fprintf(fid, '%s.dutyCycSp_perml_u16 = %s; /* duty 0.1%% (0~1000) */\n', base, srcPO);
            case 'DigSig'
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.flgSp_l = (bool)%s;\n', base, srcDO);
            case 'AbsltU'
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.uAbslt_mV_u16 = /* <%s voltage mV, 0~10000> */;\n', base, hp.portName);
            case 'DigPwr'
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.flgSp_l = (bool)%s;\n', base, srcDO);
            case 'RelU'
                fprintf(fid, '%s.stErrReactn_e = %s;\n', base, errR);
                fprintf(fid, '%s.uRel_perml_u16 = /* <%s 0.1%% of Ubat, 0~750> */;\n', base, hp.portName);
        end
    else
        base = sprintf('HwInp_s.%s_as[%s].outp_s', hp.hwArray, idx);
        switch hp.hwArray
            case 'AnU',    fprintf(fid, '/* %s */ x = %s.u_mV_u16;\n',   hp.portName, base);
            case 'AnI',    fprintf(fid, '/* %s */ x = %s.i_uA_u16;\n',   hp.portName, base);
            case 'Dig',    fprintf(fid, '/* %s */ x = %s.flg_l;\n',      hp.portName, base);
            case 'FrqStd', fprintf(fid, '/* %s */ x = %s.frq_p1Hz_u32;\n',hp.portName, base);
            case 'R',      fprintf(fid, '/* %s */ x = %s.r_Ohm_u32;\n',  hp.portName, base);
            case 'Sent',   fprintf(fid, '/* %s */ x = %s.dataSerlMsg_u16;\n', hp.portName, base);
        end
    end
    fprintf(fid, '\n');   % blank line between pins, like the real code
end

function cs = uniqueClasses(hwPins, D)
    cs = {};
    for i=1:numel(hwPins)
        if strcmp(hwPins(i).dir,D) && ~any(strcmp(cs,hwPins(i).hwArray))
            cs{end+1}=hwPins(i).hwArray; %#ok<AGROW>
        end
    end
end

function addClassGuide(subsysPath, leafName)
    % Place a short usage-guide annotation inside a HW class subsystem
    % (e.g. inside HwOutp/PropSig -> "PropSig  (...)\nu16 = 0~1000").
    txt = rc40_class_guide(leafName);
    if isempty(txt), return; end
    try
        a = Simulink.Annotation([subsysPath '/guide']);
        a.Text = txt;
        a.Position = [180 120];          % upper-left area of the canvas
        a.FontSize = 12;
        a.BackgroundColor = 'lightBlue';
        a.DropShadow = 'on';
    catch
        % Fallback for older releases: add_block a Note-style annotation.
        try
            add_block('built-in/Note', [subsysPath '/guide'], ...
                'Text', txt, 'Position', [180 120]);
        catch
            % annotations are non-critical; ignore if unsupported
        end
    end
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

function nm = choosePortName(p, m, cfg)
    % Port name = [PD_|PU_|HS_|LS_] + <physical pin> [+ _<Pin Assignment (MASAR)>]
    %   e.g. A31 + F-column 'PWR_LED'  ->  HS_A31_PWR_LED
    % The physical pin name is always kept; the F-column name is appended.
    base = p.name;
    % INPUT: optional Pull-Down / Pull-Up prefix from Description (PD_ / PU_)
    if strcmp(m.direction,'In') && isfield(cfg,'usePullPrefix') && cfg.usePullPrefix
        d = lower(p.desc);
        if contains(d,'pull-down') || contains(d,'pull down') || contains(d,'pulldown')
            base = ['PD_' base];
        elseif contains(d,'pull-up') || contains(d,'pull up') || contains(d,'pullup')
            base = ['PU_' base];
        end
    end
    % OUTPUT: optional HS_ / LS_ prefix from High Side / Low Side (RC40 datasheet).
    % Applies to switching/power outputs; analog voltage output (AbsltU) has no side.
    if strcmp(m.direction,'Out') && isfield(cfg,'useHsLsOutPrefix') && cfg.useHsLsOutPrefix
        if ~strcmp(m.hwArray,'AbsltU') && ~isempty(m.side)
            base = [m.side '_' base];       % HS_ / LS_
        end
    end
    % append the F-column (Pin Assignment (MASAR)) name, if any
    asg = strtrim(p.masar);
    if ~isempty(asg)
        asg = sanitize(asg);
        if startsWith(asg, [base '_']) || strcmp(asg, base)
            base = asg;                       % user already wrote the full name
        else
            base = [base '_' asg];            % HS_A31 + PWR_LED -> HS_A31_PWR_LED
        end
    end
    nm = base;
end

function nm = appendTypeSuffix(nm, dataType, cfg, flagName)
    % Append data-type suffix to a port name, e.g. SteeringAngle -> SteeringAngle_u16.
    % flagName selects which cfg flag gates this call ('appendTypeSuffix' for
    % HwInp/HwOutp pins, 'canAppendTypeSuffix' for CAN signals) so the two can
    % be turned on/off independently.
    if nargin<4, flagName = 'appendTypeSuffix'; end
    if ~(isfield(cfg,flagName) && cfg.(flagName)), return; end
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
        % Analog current inputs (AnI) use the '_VI' pin id, e.g. AnI_as[DevInp_K42_VI_D]
        % (matches HwInp_getPinIdxAnI_DU16(K42_VI) in the Os*Proc pin lists).
        if strcmp(m.hwArray,'AnI')
            sfx = getfielddef(cfg,'currentIndexSuffix','_VI');
            if ~endsWith(pin, sfx), pin = [pin sfx]; end
        end
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
    % An unsigned 1-bit (u8-range) signal is a flag -> boolean (suffix '_I').
    if ~S.signed && bits==1
        dt='boolean';
    elseif nargin>1 && strcmpi(cfg.canValueMode,'phys') && (S.factor~=1 || S.offset~=0)
        dt='single';   % 'phys' mode -> scaled signals become single; 'raw' keeps integer type.
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

function v = getfielddef(s, f, def)
    if isfield(s,f) && ~isempty(s.(f)), v = s.(f); else, v = def; end
end
function out = tern(c,a,b), if c, out=a; else, out=b; end, end
