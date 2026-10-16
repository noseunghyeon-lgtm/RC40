function outFile = rc40_gen_os10ms(hwPins, cfg)
%RC40_GEN_OS10MS  Generate an integration-ready Os10msProc.c from the Pinmap.
%
%   outFile = RC40_GEN_OS10MS(hwPins, cfg)
%
%   Uses the project's Os10msProc.c (cfg.os10msTemplate) as TEMPLATE and
%   regenerates only the hardware mapping, driven by RC40_Pinmap.xlsx
%   (Type / In-Out / Use columns, already resolved into hwPins by
%   build_rc40_model):
%
%     1. Os10msProc_HwInpPinList_as[]     entry per input pin; the enum and
%        getter follow the selected Type, e.g.
%          { HwInp_InpTypDig_E,  HwInp_getPinIdxDig_DU16(K38) },
%     2. Os10msProc_HwOutpPinChList_as[]  same for output pins.
%     3. HwOutp_s.<cls>_as[<idx>].inp_s.* setpoint assignments
%        (stErrReactn_e / flgSp_l / value line, as in the existing code).
%     4. HwInp_s.<cls>_as[<idx>].outp_s.* input reads (commented by default).
%
%   Rules
%     - Use = ON  -> active line;  Use = OFF -> same line commented out.
%     - Entries marked "used in 1ms" in the template stay untouched (they are
%       processed by the 1ms task, not here).
%     - Blocks guarded by HWTGT_RC5640 are copied verbatim (other target).
%     - Template entries whose pin is not in the Pinmap are kept but commented
%       out and reported; Pinmap pins missing in the template are appended.
%     - Safety channel lines, comments, #if guards and all other code are
%       copied byte-for-byte (CRLF and UTF-8 bytes preserved).
%
%   The result goes to cfg.os10msOut; the template is never modified.

    outFile = '';
    if exist(cfg.os10msTemplate,'file') ~= 2
        warning('rc40:Os10msTemplate','Os10msProc.c template not found: %s', cfg.os10msTemplate);
        return;
    end

    % ---- read template as raw bytes (keeps non-ASCII bytes untouched) ----------
    fid = fopen(cfg.os10msTemplate,'r');
    raw = fread(fid, inf, '*uint8')';
    fclose(fid);
    txt  = char(raw);
    crlf = ~isempty(strfind(txt, [char(13) char(10)])); %#ok<STREMP>
    txt  = strrep(txt, [char(13) char(10)], char(10));
    lines = strsplit(txt, char(10), 'CollapseDelimiters', false);

    % ---- locate the regions to regenerate -------------------------------------
    iIn      = findLine(lines, 'Os10msProc_HwInpPinList_as\[\]\s*=\s*\{');
    iOut     = findLine(lines, 'Os10msProc_HwOutpPinChList_as\[\]\s*=\s*\{');
    iInCall  = findLine(lines, '^\s*HwInp\(Os10msProc_HwInpPinList_as');
    iAsgnLst = findLine(lines, 'DigSig_as\[DevOutp_FltLampLed_D\]\.inp_s\.stErrReactn_e');
    iAsgnEnd = findLine(lines, 'HwOutp backward compatibility');
    iInE     = findClose(lines, iIn);
    iOutE    = findClose(lines, iOut);
    marks = [iIn iInE iOut iOutE iInCall iAsgnLst iAsgnEnd];
    if any(marks == 0) || any(diff(marks) <= 0)
        warning('rc40:Os10msLayout', ['Os10msProc.c template layout not recognized ' ...
            '(pin lists / HwOutp assignment block). Nothing generated.']);
        return;
    end

    inP  = hwPins(strcmp({hwPins.dir}, 'In'));
    outP = hwPins(strcmp({hwPins.dir}, 'Out'));

    % ---- 1+2: pin lists --------------------------------------------------------
    [newIn,  ms1In,  stIn ] = rewriteList(lines(iIn+1:iInE-1),   inP,  'In');
    [newOut, ms1Out, stOut] = rewriteList(lines(iOut+1:iOutE-1), outP, 'Out');

    hdr = '      // Generated from RC40_Pinmap.xlsx by rc40_gen_os10ms (Use=ON: active, Use=OFF: commented)';

    % ---- 3: HwOutp assignments -------------------------------------------------
    asg = buildOutpAssign(outP, ms1Out, cfg);
    legacy = lines(iAsgnLst+1:iAsgnEnd-1);
    legacy = legacy(~cellfun(@(s) isempty(strtrim(s)), legacy));
    if ~isempty(legacy)
        if cfg.os10msKeepLegacyOutp
            asg = [asg, {''}, legacy];
        else
            asg{end+1} = '      // ---- template example assignments (disabled by generator; cfg.os10msKeepLegacyOutp = true keeps them) ----';
            for k = 1:numel(legacy)
                s = strtrim(legacy{k});
                if strncmp(s, '//', 2)
                    asg{end+1} = ['      ' s]; %#ok<AGROW>
                else
                    asg{end+1} = ['      // ' s]; %#ok<AGROW>
                end
            end
        end
    end
    asg{end+1} = '';

    % ---- 4: HwInp reads --------------------------------------------------------
    inpBlock = buildInpReads(inP, ms1In, cfg);

    % ---- assemble ---------------------------------------------------------------
    outLines = [ lines(1:iIn), {hdr}, newIn, ...
            lines(iInE:iOut), {hdr}, newOut, ...
            lines(iOutE:iInCall), inpBlock, ...
            lines(iInCall+1:iAsgnLst), asg, ...
            lines(iAsgnEnd:end) ];
    outTxt = strjoin(outLines, char(10));
    if crlf, outTxt = strrep(outTxt, char(10), [char(13) char(10)]); end

    outFile = cfg.os10msOut;
    d = fileparts(outFile);
    if ~isempty(d) && exist(d,'dir') ~= 7, mkdir(d); end
    fo = fopen(outFile,'w');
    if fo < 0, warning('rc40:Os10msWrite','Cannot write %s', outFile); outFile = ''; return; end
    fwrite(fo, uint8(outTxt), 'uint8');
    fclose(fo);

    % ---- report -----------------------------------------------------------------
    fprintf('\nOs10msProc.c written: %s\n', outFile);
    fprintf('  HwInp  pin list : %d active, %d commented, %d appended\n', stIn.active,  stIn.commented,  stIn.added);
    fprintf('  HwOutp pin list : %d active, %d commented, %d appended\n', stOut.active, stOut.commented, stOut.added);
    reportList('HwInp',  stIn,  inP,  ms1In);
    reportList('HwOutp', stOut, outP, ms1Out);
end

% ============================================================================
function [newBody, ms1, st] = rewriteList(body, pins, D)
%REWRITELIST  Rewrite the entries of one pin list, keeping its structure.
    byPin = containers.Map('KeyType','char','ValueType','double');
    for i = 1:numel(pins), byPin(pins(i).pin) = i; end
    seen = false(1, numel(pins));
    newBody = {};  ms1 = {};
    st = struct('active',0,'commented',0,'added',0,'changed',{{}},'missing',{{}});
    guard = '';
    entryPat = ['^(?://)?\s*\{\s*Hw(?:Inp|Outp)_(?:Inp|Outp)Typ(\w+?)_E,\s*' ...
                'Hw(?:Inp|Outp)_get(?:PinIdx|ChIdx)(\w+?)_DU16\((\w+)\)\s*\}'];
    for k = 1:numel(body)
        L = body{k};  s = strtrim(L);
        if ~isempty(regexp(s, '^#\s*if', 'once'))
            guard = s;  newBody{end+1} = L;  continue; %#ok<AGROW>
        end
        if ~isempty(regexp(s, '^#\s*endif', 'once'))
            guard = '';  newBody{end+1} = L;  continue; %#ok<AGROW>
        end
        tok = regexp(s, entryPat, 'tokens', 'once');
        if isempty(tok) || ~isempty(strfind(guard, 'HWTGT_RC5640')) %#ok<STREMP>
            newBody{end+1} = L;  continue; %#ok<AGROW>   % verbatim
        end
        cls = tok{1};  pin = tok{3};
        if strncmp(s, '//', 2) && ~isempty(strfind(lower(s), 'used in 1ms')) %#ok<STREMP>
            ms1{end+1} = pin; %#ok<AGROW>
            if isKey(byPin, pin), seen(byPin(pin)) = true; end
            newBody{end+1} = L;  continue; %#ok<AGROW>
        end
        if isKey(byPin, pin)
            i = byPin(pin);  seen(i) = true;  hp = pins(i);
            if ~strcmp(cls, hp.hwArray)
                st.changed{end+1} = sprintf('%s: %s -> %s', pin, cls, hp.hwArray); %#ok<AGROW>
            end
            if hp.used
                newBody{end+1} = entryLine(D, hp.hwArray, pin, false, ''); %#ok<AGROW>
                st.active = st.active + 1;
            else
                newBody{end+1} = entryLine(D, hp.hwArray, pin, true, '  // Use=OFF'); %#ok<AGROW>
                st.commented = st.commented + 1;
            end
        else
            newBody{end+1} = entryLine(D, cls, pin, true, '  // not in Pinmap'); %#ok<AGROW>
            st.missing{end+1} = pin; %#ok<AGROW>
            st.commented = st.commented + 1;
        end
    end
    % Pinmap pins that the template does not know about
    extra = find(~seen);
    if ~isempty(extra)
        newBody{end+1} = '      // ---- pins from RC40_Pinmap.xlsx not present in the template list (check target / guard) ----';
        for j = 1:numel(extra)
            hp = pins(extra(j));
            if hp.used
                newBody{end+1} = entryLine(D, hp.hwArray, hp.pin, false, ''); %#ok<AGROW>
                st.active = st.active + 1;
            else
                newBody{end+1} = entryLine(D, hp.hwArray, hp.pin, true, '  // Use=OFF'); %#ok<AGROW>
                st.commented = st.commented + 1;
            end
            st.added = st.added + 1;
        end
    end
end

function s = entryLine(D, cls, pin, commented, tail)
%ENTRYLINE  One pin-list entry, formatted like the template.
    if strcmp(D, 'In')
        enum = sprintf('HwInp_InpTyp%s_E,', cls);
        if strcmp(cls, 'FrqPha')
            getter = sprintf('HwInp_getChIdxFrqPha_DU16(%s)', pin);
        else
            getter = sprintf('HwInp_getPinIdx%s_DU16(%s)', cls, pin);
        end
        w = 23;
    else
        enum   = sprintf('HwOutp_OutpTyp%s_E,', cls);
        getter = sprintf('HwOutp_getPinIdx%s_DU16(%s)', cls, pin);
        w = 25;
    end
    body = sprintf('{ %-*s%s },', w, enum, getter);
    if commented, s = ['//    ' body tail]; else, s = ['      ' body tail]; end
end

% ----------------------------------------------------------------------------
function L = buildOutpAssign(outP, ms1Out, cfg)
%BUILDOUTPASSIGN  HwOutp setpoint assignments (3-line sets like Os10msProc.c).
    L = {};
    L{end+1} = '      // ---- generated from RC40_Pinmap.xlsx by rc40_gen_os10ms ----';
    L{end+1} = '      // source signals: <appOutpBus>.PO_s.<port> (duty/current/voltage), .DO_s.<port> (digital)';
    for pass = 1:2                       % 1 = Use ON, 2 = Use OFF
        if numel(outP) == 0, break; end
        sel = outP([outP.used] == (pass == 1));
        if isempty(sel), continue; end
        if pass == 2
            L{end+1} = ''; %#ok<AGROW>
            L{end+1} = '      // ---- Use = OFF pins (commented out) ----'; %#ok<AGROW>
        end
        cs = classesOf(sel);
        for ci = 1:numel(cs)
            L{end+1} = sprintf('      // ---- %s ----', cs{ci}); %#ok<AGROW>
            for i = 1:numel(sel)
                hp = sel(i);
                if ~strcmp(hp.hwArray, cs{ci}), continue; end
                pl = outpPinLines(hp, cfg);
                inMs = any(strcmp(ms1Out, hp.pin));
                if inMs
                    L{end+1} = sprintf('      // %s: handled in the 1ms task (see pin list), not assigned here', hp.pin); %#ok<AGROW>
                end
                for j = 1:numel(pl)
                    if pass == 2 || inMs
                        L{end+1} = ['      // ' pl{j}]; %#ok<AGROW>
                    else
                        L{end+1} = ['      ' pl{j}]; %#ok<AGROW>
                    end
                end
                L{end+1} = ''; %#ok<AGROW>
            end
        end
    end
end

function L = outpPinLines(hp, cfg)
    idx  = hp.index;                                     % e.g. DevOutp_K81_D
    cls  = hp.hwArray;
    base = sprintf('HwOutp_s.%s_as[%s].inp_s', cls, idx);
    errR = sprintf('ErrReactn_s.outp_s.ErrReactn_s.%s_as_%s_stErrReactn_e', cls, idx);
    PO   = sprintf('%s.PO_s.%s', cfg.appOutpBus, hp.portName);
    DO   = sprintf('%s.DO_s.%s', cfg.appOutpBus, hp.portName);
    L = { sprintf('%s.stErrReactn_e = %s;', base, errR) };
    switch cls
        case 'PropSig'
            L{end+1} = sprintf('%s.flgSp_l = TRUE;', base);
            L{end+1} = sprintf('%s.dutyCycSp_perml_u16 = (uint16)%s;', base, PO);
        case 'PropPwr'
            if any(strcmp(hp.pin, cfg.os10msFlagPins))
                % enable-flag only (FctDigCtrl / FctPwrSply): no current/duty line
                L{end+1} = sprintf('%s.flgSp_l = (bool)%s;', base, DO);
            elseif ~isempty(strfind(hp.desc, 'current controlled')) %#ok<STREMP>
                L{end+1} = sprintf('%s.flgSp_l = TRUE;', base);
                L{end+1} = sprintf('%s.iSp_mA_u16 = (uint16)%s;', base, PO);
            else
                L{end+1} = sprintf('%s.flgSp_l = TRUE;', base);
                L{end+1} = sprintf('%s.dutyCycSp_perml_u16 = (uint16)%s;', base, PO);
            end
        case {'DigSig','DigPwr'}
            L{end+1} = sprintf('%s.flgSp_l = (bool)%s;', base, DO);
        case 'RelU'
            L{end+1} = sprintf('%s.uRel_perml_u16 = (uint16)%s;', base, PO);
        case 'AbsltU'
            L{end+1} = sprintf('%s.uAbslt_mV_u16 = (uint16)%s;', base, PO);
    end
end

% ----------------------------------------------------------------------------
function L = buildInpReads(inP, ms1In, cfg)
%BUILDINPREADS  HwInp -> application input bus (commented by default).
    L = {};
    mode = lower(cfg.os10msInputs);
    if strcmp(mode, 'off') || isempty(inP), return; end
    L{end+1} = '';
    L{end+1} = '      // ---- HwInp reads generated from RC40_Pinmap.xlsx by rc40_gen_os10ms ----';
    if strcmp(mode, 'commented')
        L{end+1} = '      // Commented out: confirm the destination bus (cfg.appInpBus), then remove the leading //';
    end
    for pass = 1:2
        sel = inP([inP.used] == (pass == 1));
        if isempty(sel), continue; end
        if pass == 2
            L{end+1} = '      // ---- Use = OFF pins ----'; %#ok<AGROW>
        end
        for i = 1:numel(sel)
            hp = sel(i);
            fld = inpField(hp.hwArray);
            if isempty(fld), continue; end
            ln = sprintf('%s.%s = HwInp_s.%s_as[%s].outp_s.%s;', ...
                cfg.appInpBus, hp.portName, hp.hwArray, hp.index, fld);
            inMs = any(strcmp(ms1In, hp.pin));
            if pass == 1 && ~inMs && strcmp(mode, 'active')
                L{end+1} = ['      ' ln]; %#ok<AGROW>
            else
                tail = '';
                if inMs, tail = '  // handled in the 1ms task'; end
                L{end+1} = ['      // ' ln tail]; %#ok<AGROW>
            end
        end
    end
end

function f = inpField(cls)
    switch cls
        case 'AnU',    f = 'u_mV_u16';
        case 'AnI',    f = 'i_uA_u16';
        case 'Dig',    f = 'flg_l';
        case 'FrqStd', f = 'frq_p1Hz_u32';
        case 'FrqShnt',f = 'frq_p1Hz_u32';
        case 'FrqPha', f = 'frq_p1Hz_u32';
        case 'R',      f = 'r_Ohm_u32';
        case 'Sent',   f = 'dataSerlMsg_u16';
        otherwise,     f = '';
    end
end

% ----------------------------------------------------------------------------
function reportList(name, st, pins, ms1)
    if ~isempty(st.changed)
        fprintf('  %s class changed vs template (Pinmap Type wins):\n', name);
        for i = 1:numel(st.changed), fprintf('    %s\n', st.changed{i}); end
    end
    if ~isempty(st.missing)
        fprintf('  %s template entries not in Pinmap (commented): %s\n', name, strjoin(st.missing, ', '));
    end
    for i = 1:numel(pins)
        if pins(i).used && any(strcmp(ms1, pins(i).pin))
            fprintf('  NOTE %s %s is Use=ON but marked "used in 1ms" in the template (left to the 1ms task)\n', name, pins(i).pin);
        end
    end
end

function cs = classesOf(pins)
    cs = {};
    for i = 1:numel(pins)
        if ~any(strcmp(cs, pins(i).hwArray)), cs{end+1} = pins(i).hwArray; end %#ok<AGROW>
    end
end

function n = findLine(lines, pat)
    hit = find(~cellfun(@isempty, regexp(lines, pat, 'once')), 1);
    if isempty(hit), n = 0; else, n = hit; end
end

function n = findClose(lines, iStart)
    n = 0;
    if iStart == 0, return; end
    for k = iStart+1:numel(lines)
        if strcmp(strtrim(lines{k}), '};'), n = k; return; end
    end
end
