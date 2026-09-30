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

    section = '';
    pins = struct('section',{},'name',{},'type',{},'inout',{}, ...
                  'desc',{},'masar',{},'connectTo',{});

    for r = 1:size(raw,1)
        c = @(k) cellval(raw, r, k);
        c1 = c(1); c2 = c(2); c3 = c(3); c4 = c(4); c5 = c(5); c6 = c(6); c7 = c(7);

        % Column layout in this workbook is shifted right by one (col 1 blank),
        % so the meaningful data starts at column 2. Detect dynamically:
        %   title row  -> a lone label in the first non-empty column
        %   header row -> contains 'Name' and 'Type'
        rowText = strtrim(strjoin(cellfun(@tostr,{c1,c2,c3,c4,c5,c6,c7},'uni',0),'|'));

        % Section titles
        if any(strcmpi(strip1(c2), {'Signal','Power','Communication'})) && isempty(strip1(c3))
            section = strip1(c2); continue;
        end
        % Header row
        if strcmpi(strip1(c2),'Name') && strcmpi(strip1(c3),'Type')
            continue;
        end

        name = strip1(c2);
        if isempty(name), continue; end       % skip blank rows

        p.section   = section;
        p.name      = name;
        p.type      = strip1(c3);
        p.inout     = strip1(c4);
        p.desc      = strip1(c5);
        p.masar     = strip1(c6);
        p.connectTo = strip1(c7);
        pins(end+1) = p; %#ok<AGROW>
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
