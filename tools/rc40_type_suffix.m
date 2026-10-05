function sfx = rc40_type_suffix(dataType)
%RC40_TYPE_SUFFIX  Data-type suffix used in RC40/MASAR port names.
%
%   sfx = RC40_TYPE_SUFFIX(dataType) maps a Simulink data-type string to the
%   short suffix seen in the model (e.g. SteeringAngle_u16, MC_EPSO1_u8,
%   HydraulicFailure_l).
%
%   boolean -> I    (logical/flag; e.g. a CAN signal with bit length 1)
%   uint8   -> u8    int8  -> s8
%   uint16  -> u16   int16 -> s16
%   uint32  -> u32   int32 -> s32
%   uint64  -> u64   int64 -> s64
%   single  -> f32   double-> f64
%
%   Unknown types return '' (no suffix appended).

    switch lower(strtrim(dataType))
        case 'boolean', sfx = 'I';
        case 'uint8',   sfx = 'u8';
        case 'uint16',  sfx = 'u16';
        case 'uint32',  sfx = 'u32';
        case 'uint64',  sfx = 'u64';
        case 'int8',    sfx = 's8';
        case 'int16',   sfx = 's16';
        case 'int32',   sfx = 's32';
        case 'int64',   sfx = 's64';
        case 'single',  sfx = 'f32';
        case 'double',  sfx = 'f64';
        otherwise,      sfx = '';
    end
end
