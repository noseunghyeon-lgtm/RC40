function s = rc40_zero_literal(dataType)
%RC40_ZERO_LITERAL  Typed zero as MATLAB source text for a Simulink data type.
%   boolean -> 'false';  uint16 -> 'uint16(0)';  single -> 'single(0)' ...
    if strcmp(dataType, 'boolean')
        s = 'false';
    else
        s = sprintf('%s(0)', dataType);
    end
end
