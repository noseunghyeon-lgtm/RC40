function [types, isMulti] = rc40_pin_functions(desc, curType)
%RC40_PIN_FUNCTIONS  Selectable functions (Excel Type values) of a pin.
%
%   [types, isMulti] = RC40_PIN_FUNCTIONS(desc, curType)
%     desc    : Description column (e.g. 'Digital, Voltage, Current, Resistance')
%     curType : Type kept at the front of the list ('' = Description-derived only,
%               which is what the builder uses to check the selection)
%     types   : cell array of allowed Type values for the C-column dropdown
%     isMulti : true if the Description lists several functions (multi-port pin)
%
%   A pin is "multi-port" when its Description lists several functions
%   separated by ',' or '/' (RC40 multi-functional inputs). Each function
%   keyword maps to an Excel Type value:
%     Digital     -> DigitalSignal
%     Voltage     -> AnalogSignal
%     Current     -> CurrentSignal               (HwInpAnI)
%     Resistance  -> ResistanceMeasurementSignal
%     Frequency / DSM1/DST1 -> Frequency
%     SENT        -> SENTSignal
%     Analog output -> PWMSignal                 (HwOutpAbsltU, output)
%
%   Keep this list consistent with tools/add_pinmap_dropdowns.py, which writes
%   the same lists into the Excel dropdowns.

    d = lower(desc);
    types = {};
    if ~isempty(strtrim(curType)), types = {strtrim(curType)}; end
    isMulti = contains(d, ',') || ...
              (contains(d, '/') && ~contains(d, 'dsm1/dst1') && ~startsWith(d,'pull'));
    if contains(d, 'output') && ~contains(d, 'analog output')
        isMulti = false;          % power/switching output description, not a function list
    end
    if ~isMulti, return; end

    rules = { 'digital',       'DigitalSignal'
              'voltage',       'AnalogSignal'
              'analog/',       'AnalogSignal'
              'current',       'CurrentSignal'
              'resistance',    'ResistanceMeasurementSignal'
              'frequency',     'Frequency'
              'dsm1/dst1',     'Frequency'
              'sent',          'SENTSignal'
              'analog output', 'PWMSignal' };
    for k = 1:size(rules,1)
        if contains(d, rules{k,1}) && ~any(strcmpi(types, rules{k,2}))
            types{end+1} = rules{k,2}; %#ok<AGROW>
        end
    end
end
