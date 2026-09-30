function [dir, group] = rc40_can_direction(item, cfg)
%RC40_CAN_DIRECTION  Decide Tx/Rx (Outport/Inport) for a CAN signal.
%
%   [dir, group] = RC40_CAN_DIRECTION(item, cfg)
%     dir   : 'In' (Rx) | 'Out' (Tx) | '' (skip)
%     group : subsystem group name ('CAN_Rx' / 'CAN_Tx')
%
%   Direction source is selected by cfg.canDirectionMode:
%
%   'masar'  (default, matches how MASAR Tool works)
%       item is a signal struct from RC40_READ_CAN_EXCEL and carries an
%       explicit .dir = 'Tx' | 'Rx' (the "Tx or Rx" column of the MASAR
%       CANdbaseEditor Dataset sheet). We use it verbatim.
%           Tx (controller transmits) -> Outport
%           Rx (controller receives)  -> Inport
%
%   'node'   (fallback when only a raw DBC is available)
%       item is a struct with fields .txNode and .receivers (from
%       RC40_READ_DBC); direction is inferred relative to cfg.ecuNode.

    switch lower(cfg.canDirectionMode)

        case 'masar'
            d = upper(strtrim(item.dir));
            switch d
                case 'TX', dir='Out'; group='CAN_Tx';
                case 'RX', dir='In';  group='CAN_Rx';
                otherwise
                    dir=''; group='';    % unspecified -> skip (or warn upstream)
            end

        case 'node'
            if strcmpi(item.txNode, cfg.ecuNode)
                dir='Out'; group='CAN_Tx';
            elseif isfield(item,'receivers') && any(strcmpi(item.receivers, cfg.ecuNode))
                dir='In';  group='CAN_Rx';
            else
                dir=''; group='';
            end

        otherwise
            error('rc40:CanDirMode','Unknown cfg.canDirectionMode: %s', cfg.canDirectionMode);
    end
end
