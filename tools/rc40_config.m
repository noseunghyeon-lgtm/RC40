function cfg = rc40_config()
%RC40_CONFIG  Central configuration for the RC40 pinmap -> Simulink generator.
%
%   Edit the paths and options here, then run BUILD_RC40_MODEL.
%
%   All rules encoded below are derived from:
%     - RC40_Pinmap.xlsx        (pin definitions: Signal / Power / Communication)
%     - Hw*.h header files       (data types and physical ranges per signal type)
%     - MASAR naming convention  (HwInp/HwOutp array element references)
%
%   See README.md for details.

% ----------------------------------------------------------------------------
% Paths
% ----------------------------------------------------------------------------
here            = fileparts(mfilename('fullpath'));
repoRoot        = fileparts(here);                       % .../RC40

cfg.pinmapXlsx  = fullfile(repoRoot, 'RC40_Pinmap.xlsx'); % input pin definition
% CAN definitions. MASAR manages CAN in the CANdbaseEditor workbook (the
% "Dataset" sheet with an explicit "Tx or Rx" column). That is the primary
% source. A raw .dbc is kept as an optional fallback.
cfg.canExcel    = fullfile(repoRoot, 'CANdbaseEditor.xlsm'); % MASAR CAN Excel
cfg.canExcelSheet = 'Dataset';
cfg.dbcFolder   = fullfile(repoRoot, 'can');              % folder scanned for *.dbc (fallback)
cfg.dbcFiles    = {fullfile(repoRoot,'KIA_ADUS_CAN_v0_6.dbc')}; % explicit .dbc (fallback)
cfg.outputSlx   = fullfile(repoRoot, 'RC27_18_Model.slx'); % model to (re)create
cfg.modelName   = 'RC27_18_Model';

% Target controller variant. This pinmap matches the RC27-18/40 (BODAS RC
% series 40): 45 power outputs (21 high-side / 24 low-side) + 11 low-power
% outputs + 58 multi-functional inputs + 4 CAN. Kept for documentation and
% for variant-specific checks.
cfg.ecuType     = 'RC27-18/40';

% CAN node name of THIS controller inside the DBC network database.
cfg.ecuNode     = 'RC27_18';

% CAN direction logic selector (decision lives in rc40_can_direction.m):
%   'masar' - use the explicit "Tx or Rx" column from the MASAR CAN Excel
%             (Dataset sheet). This is how MASAR Tool manages CAN. DEFAULT.
%   'node'  - infer from DBC transmitter/receiver relative to cfg.ecuNode
%             (fallback when only a raw .dbc is available).
cfg.canDirectionMode = 'masar';

% CAN source selector:
%   'excel' - read CAN signals from cfg.canExcel (MASAR Dataset sheet). DEFAULT.
%   'dbc'   - read CAN signals from cfg.dbcFiles / cfg.dbcFolder (*.dbc).
cfg.canSource = 'excel';

% ----------------------------------------------------------------------------
% Generation options
% ----------------------------------------------------------------------------
% Which pins get a Simulink port:
%   'all'      - every Signal/Comm pin becomes a port (MASAR name used when present,
%                otherwise the physical pin name such as A01). RECOMMENDED while the
%                "Pin Assignment (MASAR)" column is still being filled in.
%   'assigned' - only pins whose "Pin Assignment (MASAR)" cell is non-empty.
cfg.portSelection      = 'all';

% Include the Power section (Power Supply / Ground / SensorSupply / SensorGND)
% as ports. OFF - supply/ground pins are not model I/O.
cfg.includePowerPins   = false;

% HW port grouping (Signal section pins):
%   'nested'  - two-level hierarchy: top HwInp / HwOutp subsystem, and under
%               it one child subsystem per HW class chosen from the pin's
%               setting (Type). e.g. HwInp/AnU, HwInp/Dig, HwOutp/PropPwr.
%               Mirrors the header structure (HwInp_s.AnU_as[], ...). DEFAULT.
%   'hwinout' - flat: all HW inputs in one HwInp, all HW outputs in one HwOutp.
%   'byclass' - one subsystem per HW class (Input_AnU, Output_PropPwr, ...).
cfg.hwGrouping         = 'nested';

% Include the Communication section (LIN / CAN / Ethernet) as CAN Tx/Rx ports.
cfg.includeCommPins     = true;

% Append the data-type suffix to port names, matching the model convention
% (e.g. SteeringAngle -> SteeringAngle_u16, HydraulicFailure -> ..._l,
% MC_EPSO1 -> MC_EPSO1_u8). Mapping in rc40_type_suffix.m.
cfg.appendTypeSuffix   = true;

% Prefix analog input port names with PD_ / PU_ based on the Description
% (Pull-Down / Pull-Up), e.g. PD_K38_APP_Sig1. PD_ = Pull-Down.
cfg.usePullPrefix      = true;

% CAN signal port data type:
%   'raw'    - use the raw integer type (u8->uint8, u16->uint16, ...) as sent
%              on the bus, ignoring factor/offset. Matches the manual model
%              (e.g. SteeringAngle_u16). Scaling is done downstream. DEFAULT.
%   'phys'   - use single when factor~=1 or offset~=0 (physical value).
cfg.canValueMode       = 'raw';

% Port dimensions to stamp on every generated port (PortDimensions param).
% Manual model uses scalar signals -> 1.
cfg.portDimensions     = '1';

% CAN port naming:
%   'signal'  - use the signal name only (e.g. SteeringAngle_u16). If two
%               signals share a name, a numeric suffix (_2,...) is appended
%               to keep block names unique. DEFAULT.
%   'message' - prefix with the message name (e.g. VDC2_SteerWheelAngle_u16),
%               guaranteeing uniqueness and matching the manual model style
%               where the message context is part of the name.
cfg.canPortNaming      = 'signal';

% HS/LS suffix on OUTPUT index names (e.g. DevOutp_A03LS_D).
% NOTE: HS/LS is a MASAR-internal naming convenience (not a hardware property),
% derived from "High Side" / "Low Side" in the Description column.
% Set false to emit plain DevOutp_<pin>_D.
cfg.useHsLsSuffix       = true;

% Per-pin overrides for cases the Excel cannot express, e.g. a pin whose
% Description is empty ('-') so HS/LS can't be inferred, or a pin that MASAR
% reconfigures away from the Excel Type. Each entry: pin -> struct with any of
% the fields {hwArray, hwClass, dataType, side, direction}. Fields left out
% fall back to the value derived from the Excel Type via rc40_typemap.
%   Sources: RC5-6/40 header note "K12/K13 can be reconfigured as high side";
%            RC40 datasheet (K17 = HighSide 4A current-controlled power output).
ov = containers.Map('KeyType','char','ValueType','any');
ov('K17') = struct('side','HS');                          % desc '-' in Excel; HS per datasheet
% ov('K13') = struct('hwArray','PropPwr','hwClass','HwOutpPropPwr','side','LS'); % if MASAR reconfigures K13 to power
cfg.overrides = ov;

% Validation policy when a pin's Type is not recognized:
%   'error' - stop and report (strict, recommended)
%   'warn'  - warn and skip the pin
cfg.onUnknownType      = 'error';

% Layout spacing (pixels) for auto-placed ports.
cfg.layout.x_in        = 40;    % x of input ports column
cfg.layout.x_out       = 640;   % x of output ports column
cfg.layout.y0          = 40;    % first port y
cfg.layout.dy          = 40;    % vertical spacing
cfg.layout.w           = 30;    % port width
cfg.layout.h           = 14;    % port height

end
