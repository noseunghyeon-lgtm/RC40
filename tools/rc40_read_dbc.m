function msgs = rc40_read_dbc(dbcPath)
%RC40_READ_DBC  Minimal, dependency-free DBC parser.
%
%   msgs = RC40_READ_DBC(dbcPath) parses CAN message (BO_) and signal (SG_)
%   definitions from a .dbc file WITHOUT requiring Vehicle Network Toolbox.
%
%   Returned struct array (one element per message):
%     .id        numeric CAN id (raw, as in DBC; bit31 flag stripped)
%     .extended  true if 29-bit (extended) id
%     .name      message name
%     .dlc       data length (bytes)
%     .txNode    transmitter node name
%     .signals   struct array: .name .startBit .len .byteOrder(0/1) .signed
%                              .factor .offset .min .max .unit .receivers{}
%
%   Only the fields needed for port generation are extracted.

    msgs = struct('id',{},'extended',{},'name',{},'dlc',{},'txNode',{},'signals',{});
    txt = fileread(dbcPath);
    lines = regexp(txt, '\r\n|\r|\n', 'split');

    cur = [];
    for i = 1:numel(lines)
        ln = strtrim(lines{i});
        if startsWith(ln,'BO_ ')
            if ~isempty(cur), msgs(end+1) = cur; end %#ok<AGROW>
            tok = regexp(ln, '^BO_\s+(\d+)\s+(\w+)\s*:\s*(\d+)\s+(\w+)', 'tokens','once');
            cur = struct('id',[],'extended',false,'name','','dlc',[], ...
                         'txNode','','signals',struct('name',{},'startBit',{}, ...
                         'len',{},'byteOrder',{},'signed',{},'factor',{}, ...
                         'offset',{},'min',{},'max',{},'unit',{},'receivers',{}));
            if ~isempty(tok)
                rawId = str2double(tok{1});
                cur.extended = rawId >= 2^31;                 % bit31 = extended flag
                cur.id  = bitand(uint32(rawId), uint32(2^31-1));
                cur.name = tok{2};
                cur.dlc  = str2double(tok{3});
                cur.txNode = tok{4};
            end
        elseif startsWith(ln,'SG_ ') && ~isempty(cur)
            % SG_ Name : start|len@order sign (factor,offset) [min|max] "unit" Recv
            tok = regexp(ln, ['^SG_\s+(\w+)\s*:\s*(\d+)\|(\d+)@(\d)([+-])\s*' ...
                              '\(([^,]+),([^\)]+)\)\s*\[([^|]*)\|([^\]]*)\]\s*' ...
                              '"([^"]*)"\s*(.*)$'], 'tokens','once');
            if ~isempty(tok)
                s.name      = tok{1};
                s.startBit  = str2double(tok{2});
                s.len       = str2double(tok{3});
                s.byteOrder = str2double(tok{4});   % 1=little-endian(Intel), 0=big-endian(Motorola)
                s.signed    = strcmp(tok{5},'-');
                s.factor    = str2double(tok{6});
                s.offset    = str2double(tok{7});
                s.min       = str2double(tok{8});
                s.max       = str2double(tok{9});
                s.unit      = tok{10};
                s.receivers = strtrim(regexp(strtrim(tok{11}), '[,\s]+', 'split'));
                cur.signals(end+1) = s; %#ok<AGROW>
            end
        end
    end
    if ~isempty(cur), msgs(end+1) = cur; end
end
