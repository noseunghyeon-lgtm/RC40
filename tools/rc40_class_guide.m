function txt = rc40_class_guide(hwArray)
%RC40_CLASS_GUIDE  Short usage guide text for a HW class subsystem.
%
%   txt = RC40_CLASS_GUIDE(hwArray) returns a brief, multi-line annotation
%   describing how to use the pins of a given HW class. It is placed as an
%   annotation inside the corresponding HwInp/<class> or HwOutp/<class>
%   subsystem so a user knows the data type, unit and range at a glance.
%
%   Values are taken from the RC40 Hw*.h headers and the RC40 datasheet:
%     AnU     u16 = 0~40000 mV      (analog voltage in)
%     Dig     boolean (0/1)         (digital in)
%     FrqStd  u32 = 0~20000 (0.1Hz) (frequency in)  -> physical 0..20000 Hz
%     R       u32 = 0~700000 Ohm    (resistance in)
%     Sent    u16                   (SENT / SAE J2716 in)
%     PropPwr u16 = 0~4000 mA       (proportional power out; setpoint current)
%     DigSig  boolean (0/1)         (digital signal out)
%     PropSig u16 = 0~1000          (proportional signal out; duty in 0.1%)
%     AbsltU  u16 = 0~10000 mV      (analog voltage out)

    switch hwArray
        case 'AnU'
            txt = sprintf('AnU  (Analog Voltage In)\nu16 = 0~40000 mV');
        case 'Dig'
            txt = sprintf('Dig  (Digital In)\nboolean (0/1)');
        case 'FrqStd'
            txt = sprintf('FrqStd  (Frequency In)\nu32 = 0~20000  (unit 0.1 Hz)');
        case 'R'
            txt = sprintf('R  (Resistance In)\nu32 = 0~700000 Ohm');
        case 'Sent'
            txt = sprintf('Sent  (SENT In, SAE J2716)\nu16');
        case 'PropPwr'
            txt = sprintf('PropPwr  (Proportional Power Out)\nu16 = 0~4000 mA  (setpoint current)');
        case 'DigSig'
            txt = sprintf('DigSig  (Digital Signal Out)\nboolean (0/1)');
        case 'PropSig'
            txt = sprintf('PropSig  (Proportional Signal Out)\nu16 = 0~1000  (duty 0.1%%)');
        case 'AbsltU'
            txt = sprintf('AbsltU  (Analog Voltage Out)\nu16 = 0~10000 mV');
        otherwise
            txt = '';
    end
end
