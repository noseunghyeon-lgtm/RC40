function pins = rc40_read_pinmap(xlsxPath)
%RC40_READ_PINMAP  Read RC40_Pinmap.xlsx into a struct array of pin definitions.
%
%   pins = RC40_READ_PINMAP(xlsxPath)
%
%   The sheet has three stacked sections, each introduced by a title row
%   ("Signal", "Power", "Communication") followed by a header row
%   (Name | Type | In/Out | Description | Pin Assignment (MASAR) | Connect to).
%
%   Each returned element has fields:
%     .section   'Signal' | 'Power' | 'Communication'
%     .name      pin name (e.g. 'A01', 'K88', 'A43_VI')
%     .type      Excel Type (e.g. 'PWMSignal', 'CAN1', 'Power Suppy')
%     .inout     'IN' | 'OUT' | 'High' | 'Low' | '24V' | 'GND' | 'P' | 'M' | ...
%     .desc      Description text
%     .masar     Pin Assignment (MASAR) text (may be empty)
%     .connectTo 'Connect to' text (may be empty)
%
%   Uses readcell when available (Simulink/MATLAB always ships it). No
%   Vehicle Network Toolbox or other add-on is required.

    if exist('readcell','file') == 2
        raw = readcell(xlsxPath);            % cell array, all sections stacked
    else
        [~,~,raw] = xlsread(xlsxPath);       %#ok<XLSRD> % legacy fallback
    end

    % --- auto-detect the base column ---------------------------------------
    % readcell may or may not keep a leading empty column, so the data may
    % start at column 1 or 2. Find the "Name"/"Type" header row and use the
    % column where 'Name' sits as the base (col offset for all fields).
    base = detectBaseColumn(raw);

    section = '';
    pins = struct('section',{},'name',{},'type',{},'inout',{}, ...
                  'desc',{},'masar',{},'connectTo',{});

    for r = 1:size(raw,1)
        c = @(k) strip1(cellval(raw, r, base-1+k));   % k=1..7 relative to base
        c1 = c(1); c2 = c(2); c3 = c(3); c4 = c(4); c5 = c(5); c6 = c(6); c7 = c(7);

        % Section title row: a lone label ('Signal'/'Power'/'Communication')
        % in the base column, nothing in the next.
        if any(strcmpi(c1, {'Signal','Power','Communication'})) && isempty(c2)
            section = c1; continue;
        end
        % Header row
        if strcmpi(c1,'Name') && strcmpi(c2,'Type')
            continue;
        end

        name = c1;
        if isempty(name), continue; end       % skip blank rows

        p.section   = section;
        p.name      = name;
        p.type      = c2;
        p.inout     = c3;
        p.desc      = c4;
        p.masar     = c5;
        p.connectTo = c6;
        pins(end+1) = p; %#ok<AGROW>
    end
end

function base = detectBaseColumn(raw)
    % Find the header row ('Name' + 'Type' in adjacent cells) and return the
    % column index of 'Name'. Falls back to 1 if not found.
    base = 1;
    for r = 1:size(raw,1)
        for k = 1:max(1,size(raw,2)-1)
            if strcmpi(strip1(cellval(raw,r,k)),'Name') && ...
               strcmpi(strip1(cellval(raw,r,k+1)),'Type')
                base = k; return;
            end
        end
    end
end

function v = cellval(raw, r, k)
    if k <= size(raw,2)
        v = raw{r,k};
    else
        v = '';
    end
end

function s = strip1(v)
    s = tostr(v);
    s = strtrim(s);
end

function s = tostr(v)
    if ismissing(v)
        s = '';
    elseif ischar(v)
        s = v;
    elseif isstring(v)
        s = char(v);
    elseif isnumeric(v)
        if isnan(v), s=''; else, s = num2str(v); end
    else
        s = char(string(v));
    end
end
