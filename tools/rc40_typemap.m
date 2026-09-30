function m = rc40_typemap(excelType, inout, description)
%RC40_TYPEMAP  Map an Excel pin definition to Simulink port characteristics
%              and MASAR HwInp/HwOutp references.
%
%   m = RC40_TYPEMAP(excelType, inout, description) returns a struct describing
%   how a pin with the given Excel 'Type', 'In/Out' and 'Description' must be
%   turned into a Simulink port.
%
%   Returned fields:
%     .valid        - true if the (Type,In/Out) combination is recognized
%     .direction    - 'In' | 'Out'
%     .blockType    - 'Inport' | 'Outport'
%     .dataType     - Simulink OutDataTypeStr (e.g. 'uint16','boolean','uint32')
%     .unit         - physical unit string (e.g. 'mV','uA','Ohm','0.1Hz','permil'/'mA')
%     .physMin/.physMax - physical range (from Hw*.h), [] if not applicable
%     .hwArray      - MASAR struct-array name ('AnU','Dig','FrqStd','DigSig',
%                     'PropPwr','PropSig','AbsltU', ...)
%     .hwClass      - MASAR HW class ('HwInpAnU', 'HwOutpPropPwr', ...)
%     .side         - 'HS' | 'LS' | '' (from Description; OUTPUT power only)
%
%   Sources of truth:
%     - Excel Type column values observed: DigitalSignal, PWMSignal, AnalogSignal,
%       Frequency, ResistanceMeasurementSignal, SENTSignal
%     - Header data types:
%         HwInpAnU   u_mV_u16    uint16  0..40000 mV
%         HwInpAnI   i_uA_u16    uint16  0..25000 uA
%         HwInpR     r_Ohm_u32   uint32  0..700000 Ohm
%         HwInpFrqStd frq_p1Hz_u32 uint32 0..20000 Hz (unit 0.1 Hz)
%         HwOutpPropPwr iSp_mA_u16 / dutyCycSp_perml_u16  uint16
%         HwOutpAbsltU  uAbslt_mV_u16 uint16 0..10000 mV
%         HwInpDig / HwOutpDigSig  boolean
%     - MASAR naming convention from HwInp/HwOutp screenshots.

    t  = lower(strtrim(excelType));
    io = upper(strtrim(inout));
    d  = lower(description);

    m = struct('valid',false,'direction','','blockType','', ...
               'dataType','','unit','','physMin',[],'physMax',[], ...
               'hwArray','','hwClass','','side','');

    % --- side (HS/LS) from Description (OUTPUT power pins). MASAR-internal name. ---
    if contains(d,'high side') || contains(d,'highside')
        m.side = 'HS';
    elseif contains(d,'low side') || contains(d,'lowside')
        m.side = 'LS';
    end

    isOut = strcmp(io,'OUT');
    isIn  = strcmp(io,'IN');

    switch t
        % ---------------- INPUTS ----------------
        case 'analogsignal'
            % Analog voltage input (default). Current variant exists in HW but the
            % Excel "AnalogSignal" pins map to voltage (AnU) in MASAR screenshots.
            m.valid=true; m.direction='In'; m.blockType='Inport';
            m.dataType='uint16'; m.unit='mV'; m.physMin=0; m.physMax=40000;
            m.hwArray='AnU'; m.hwClass='HwInpAnU';

        case 'digitalsignal'
            if isIn
                m.valid=true; m.direction='In'; m.blockType='Inport';
                m.dataType='boolean'; m.unit='state';
                m.hwArray='Dig'; m.hwClass='HwInpDig';
            elseif isOut
                m.valid=true; m.direction='Out'; m.blockType='Outport';
                m.dataType='boolean'; m.unit='state';
                m.hwArray='DigSig'; m.hwClass='HwOutpDigSig';
            end

        case 'frequency'
            m.valid=true; m.direction='In'; m.blockType='Inport';
            m.dataType='uint32'; m.unit='0.1Hz'; m.physMin=0; m.physMax=200000; % 0..20000Hz in 0.1Hz
            m.hwArray='FrqStd'; m.hwClass='HwInpFrqStd';

        case 'resistancemeasurementsignal'
            m.valid=true; m.direction='In'; m.blockType='Inport';
            m.dataType='uint32'; m.unit='Ohm'; m.physMin=0; m.physMax=700000;
            m.hwArray='R'; m.hwClass='HwInpR';

        case 'sentsignal'
            m.valid=true; m.direction='In'; m.blockType='Inport';
            m.dataType='uint16'; m.unit='sent';
            m.hwArray='Sent'; m.hwClass='HwInpSent';

        % ---------------- PWM (output) ----------------
        case 'pwmsignal'
            % PWM can be:
            %  - analog voltage output ("Analog output" A12/K25/K62) -> AbsltU (HwOutpAbsltU)
            %  - low-power signal PWM ("Low side ... switching outputs", 200 mA,
            %    NO "power") -> PropSig (HwOutpPropSig)
            %  - proportional power output (everything else, incl. empty "-" desc
            %    such as K17) -> PropPwr (HwOutpPropPwr)
            m.direction='Out'; m.blockType='Outport';
            isLowPowerSignal = (contains(d,'low side') || contains(d,'lowside')) ...
                && contains(d,'switching output') && ~contains(d,'power');
            if contains(d,'analog output') || contains(d,'analog voltage output')
                m.valid=true; m.dataType='uint16'; m.unit='mV'; m.physMin=0; m.physMax=10000;
                m.hwArray='AbsltU'; m.hwClass='HwOutpAbsltU';
            elseif isLowPowerSignal
                % 200 mA low-power PWM-capable switching outputs (K80..K83) -> signal
                m.valid=true; m.dataType='uint16'; m.unit='permil'; m.physMin=0; m.physMax=1000;
                m.hwArray='PropSig'; m.hwClass='HwOutpPropSig';
            else
                % default: proportional power output (current controlled / switching
                % power / PWM-capable power / empty description). K17 (desc '-') lands here.
                m.valid=true; m.dataType='uint16'; m.unit='mA'; m.physMin=0; m.physMax=4000;
                m.hwArray='PropPwr'; m.hwClass='HwOutpPropPwr';
            end

        otherwise
            m.valid=false;
    end
end
