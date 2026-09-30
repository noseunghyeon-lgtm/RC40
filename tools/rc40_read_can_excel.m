function sigs = rc40_read_can_excel(xlsmPath, sheetName)
%RC40_READ_CAN_EXCEL  Read the MASAR CANdbaseEditor "Dataset" sheet.
%
%   sigs = RC40_READ_CAN_EXCEL(xlsmPath)               uses sheet 'Dataset'
%   sigs = RC40_READ_CAN_EXCEL(xlsmPath, sheetName)
%
%   MASAR manages CAN via this workbook. Each signal row carries an explicit
%   direction in the "Tx or Rx" column, so CAN Tx/Rx is taken from the Excel
%   (NOT inferred from DBC nodes).
%
%   Header row is row 8; signal rows start at row 9. Relevant columns:
%     A  Message Name      B  Node Name        C  Signal name
%     E  Data Type (u8/u16/u32/i8/i16/i32/u64) F  ID [hex]
%     P  Message format (StandardCAN / ExtendedCAN / ExtendedCAN_FD)
%     AA Start bit  AB Length  AD Signed  AE Byte order (Intel/Motorola)
%     AF Fac DBC  AH Offset  AI Min  AJ Max  Z Unit DBC
%     AM Tx or Rx   <-- direction (the MASAR concept)
%
%   Returned struct array fields:
%     .message .node .name .dataType .idHex .id .extended .format
%     .startBit .length .signed .byteOrder .factor .offset .min .max .unit
%     .dir  ('Tx'|'Rx')

    if nargin < 2 || isempty(sheetName), sheetName = 'Dataset'; end

    raw = readcell(xlsmPath, 'Sheet', sheetName);   % requires MATLAB (Simulink ships readcell)

    C = @colLetterToNum;
    hdrRow = 8;                                     % MASAR layout
    sigs = struct([]);
    for r = hdrRow+1 : size(raw,1)
        name = getc(raw,r,C('C'));
        if isempty(strtrim(name)), continue; end    % signal rows only

        s.message   = getc(raw,r,C('A'));
        s.node      = getc(raw,r,C('B'));
        s.name      = name;
        s.dataType  = getc(raw,r,C('E'));
        s.idHex     = getc(raw,r,C('F'));
        s.format    = getc(raw,r,C('P'));
        s.startBit  = num(getc(raw,r,C('AA')));
        s.length    = num(getc(raw,r,C('AB')));
        s.signed    = strcmpi(getc(raw,r,C('AD')),'Signed');
        s.byteOrder = getc(raw,r,C('AE'));           % 'Intel' | 'Motorola'
        s.factor    = num(getc(raw,r,C('AF')));
        s.offset    = num(getc(raw,r,C('AH')));
        s.min       = num(getc(raw,r,C('AI')));
        s.max       = num(getc(raw,r,C('AJ')));
        s.unit      = getc(raw,r,C('Z'));
        s.dir       = strtrim(getc(raw,r,C('AM')));  % 'Tx' | 'Rx'  (MASAR direction)

        s.extended  = contains(lower(s.format),'extended');
        s.id        = hex2decSafe(s.idHex);

        if isempty(fieldnames(sigs)), sigs = s; else, sigs(end+1) = s; end %#ok<AGROW>
    end
end

% --------------------------------------------------------------------------
function v = getc(raw,r,c)
    if r<=size(raw,1) && c<=size(raw,2)
        v = raw{r,c};
        if ismissing(v), v=''; elseif isnumeric(v), if isnan(v), v=''; else, v=num2str(v); end
        elseif ~ischar(v), v=char(string(v)); end
    else
        v = '';
    end
end
function x = num(v)
    if isnumeric(v), x=v; return; end
    x = str2double(v); if isnan(x), x=0; end
end
function n = colLetterToNum(L)
    n=0; for i=1:numel(L), n=n*26+(double(L(i))-64); end
end
function d = hex2decSafe(h)
    h = regexprep(char(h),'[^0-9A-Fa-f]','');
    if isempty(h), d=0; else, d=hex2dec(h); end
end
