# RC40 Pinmap → Simulink 포트 자동 생성기

**대상 제어기: Bosch Rexroth BODAS RC27-18/40** (`cfg.ecuType = 'RC27-18/40'`).
핀맵의 High-Side 파워 출력 21개가 RC27-18/40 사양(21 HS / 24 LS, 45 파워 출력)과
일치하여 이 변형으로 확정했습니다. DBC 내 이 제어기의 노드명은
`cfg.ecuNode = 'RC27_18'`이며, 이 노드가 송신하는 메시지는 Tx(Outport),
수신하는 메시지는 Rx(Inport)로 생성됩니다.

제어기(RC27-18/40)를 **Top**으로 두고, `RC40_Pinmap.xlsx`의 핀 정의와
(선택적으로) DBC 파일로부터 **Simulink Inport/Outport 및 CAN Tx/Rx 포트를
자동 생성**하는 MATLAB(.m) 스크립트 모음입니다.

각 포트는 단순히 만들어지는 데서 그치지 않고, **핀의 특성(방향·데이터 타입)**이
정의대로 포트 속성에 반영됩니다. 또한 각 포트에는 대응되는 **MASAR
HwInp/HwOutp 참조**(예: `AnU_as[DevInp_A13_D]`, `PropPwr_as[DevOutp_A31HS_D]`)가
주석으로 기록됩니다.

## 파일 구성

| 파일 | 역할 |
|------|------|
| `rc40_config.m` | 경로·옵션·per-pin override 등 모든 설정 |
| `rc40_read_pinmap.m` | `RC40_Pinmap.xlsx` 파서 (Signal/Power/Communication 3섹션) |
| `rc40_typemap.m` | Excel `Type`+`In/Out`+`Description` → 포트 특성 + MASAR HW 클래스 매핑 |
| `rc40_read_dbc.m` | 의존성 없는 DBC 파서 (Vehicle Network Toolbox 불필요) |
| `build_rc40_model.m` | 메인: 모델 생성·포트 배치·특성 적용·저장 |

## 사용법

```matlab
cd tools
build_rc40_model            % rc40_config() 설정으로 실행
% 또는
cfg = rc40_config;
cfg.portSelection = 'assigned';   % MASAR 열이 채워진 핀만
build_rc40_model(cfg);
```

**요구사항:** MATLAB + Simulink. 그 외 애드온 불필요.
DBC를 쓰려면 `<repo>/can/` 폴더에 `.dbc`를 두거나 `cfg.dbcFiles`에 경로 지정.

## Excel 스키마 (`RC40_Pinmap.xlsx`, 시트 GW1)

3개 섹션이 세로로 이어집니다. 각 섹션은 제목행 + 헤더행으로 시작합니다.

```
Name | Type | In/Out | Description | Pin Assignment (MASAR) | Connect to
```

- **Signal**: 실제 I/O 신호 핀 (122개)
- **Power**: Power Supply / Ground / SensorSupply / SensorGND (기본 포트화 안 함)
- **Communication**: LIN / CAN1~4(High/Low) / Ethernet

## Type → 포트 특성 매핑 (근거: Hw*.h 헤더)

| Excel Type | In/Out | 방향 | DataType | 단위/범위 | MASAR HW 클래스 |
|------------|--------|------|----------|-----------|-----------------|
| AnalogSignal | IN | In | `uint16` | mV, 0..40000 | `HwInpAnU` (`AnU_as`) |
| DigitalSignal | IN | In | `boolean` | state | `HwInpDig` (`Dig_as`) |
| Frequency | IN | In | `uint32` | 0.1Hz, 0..20kHz | `HwInpFrqStd` (`FrqStd_as`) |
| ResistanceMeasurementSignal | IN | In | `uint32` | Ω, 0..700k | `HwInpR` (`R_as`) |
| SENTSignal | IN | In | `uint16` | — | `HwInpSent` (`Sent_as`) |
| DigitalSignal | OUT | Out | `boolean` | state | `HwOutpDigSig` (`DigSig_as`) |
| PWMSignal (power) | OUT | Out | `uint16` | mA, 0..4000 | `HwOutpPropPwr` (`PropPwr_as`) |
| PWMSignal (low-power signal) | OUT | Out | `uint16` | ‰, 0..1000 | `HwOutpPropSig` (`PropSig_as`) |
| PWMSignal (analog output) | OUT | Out | `uint16` | mV, 0..10000 | `HwOutpAbsltU` (`AbsltU_as`) |

**PWMSignal 세부 분류(Description 기준):**
- `analog output` 포함 → `AbsltU` (예: A12, K25, K62)
- `low side ... switching output` + `power` 없음 → `PropSig` (예: K80~K83, 200mA 저전력)
- 그 외(현재제어/스위칭 파워/설명 없음) → `PropPwr` (예: K17은 설명 `-` 이지만 파워)

## 포트명 데이터타입 접미사 (`cfg.appendTypeSuffix`, 기본 on)

실제 모델처럼 포트 이름 끝에 데이터타입 접미사를 붙입니다
(`rc40_type_suffix.m`):

| DataType | 접미사 | 예시 |
|----------|--------|------|
| boolean | `_l` | `HydraulicFailure_l` |
| uint8 | `_u8` | `MC_EPSO1_u8` |
| uint16 | `_u16` | `SteeringAngle_u16` |
| uint32 | `_u32` | |
| int16 | `_i16` | |
| single | `_f32` | `SteerWheelAngle_f32` (스케일 신호) |

## Pull-Down / Pull-Up 접두사 (`cfg.usePullPrefix`, 기본 on)

입력 핀 Description이 `Pull-Down`/`Pull-Up`이면 포트명에 `PD_`/`PU_` 접두사를
붙입니다. `PD_`=Pull-Down. 예: `PD_A13_u16`, `PU_A14_u16`.

## HwOutp HS_/LS_ 접두사 (`cfg.useHsLsOutPrefix`, 기본 on)

출력 포트 이름 앞에 High Side / Low Side 구분을 붙입니다(RC40 데이터시트 =
Description의 High Side/Low Side 근거). 예: `HS_A31_u16`, `LS_A03_u16`.

- 적용 대상: 스위칭/파워 출력(`PropPwr`, `DigSig`, `PropSig`)
- 제외: 아날로그 전압 출력(`AbsltU`: A12, K25, K62) — HS/LS 개념 없음 → 접두사 없음
- Description에 side 표기가 없는 핀(K17)은 `cfg.overrides`로 보정(HS)

## BSW 통합 지원

코드 생성 결과를 BSW와 통합하기 쉽도록 다음을 지원합니다.

### 포트명은 타입 접미사 없이 (`cfg.appendTypeSuffix=false`)
포트 이름이 C 식별자로 그대로 쓰이므로 `_u16`/`_l` 접미사를 붙이지 않습니다.
포트명은 `LS_K81`, `HS_A31`, `PD_A13` 처럼 깨끗하게 유지되고, 데이터타입은
HW 구조체 필드(`dutyCycSp_perml_u16`)에 이미 있습니다.

### 클래스 가이드 주석 (`cfg.addClassGuide`, 기본 on)
각 HW 클래스 서브시스템 안에 방향·타입·범위 + **HW 구조체 접근 경로/필드 +
사용 스니펫**을 문서화합니다(`rc40_class_guide.m`). 예: `HwOutp/PropSig`
```
PropSig  (Proportional Signal Output)
DataType: u16   Range: 0~1000 (duty 0.1%)
Write: HwOutp_s.PropSig_as[DevOutp_<pin>_D].inp_s
  .flgSp_l             = TRUE;
  .dutyCycSp_perml_u16 = <duty 0.1%>;
  .stErrReactn_e       = <ErrReactn>;
```

### MASAR 인덱스 자동 활용 + 통합 코드 생성 (`cfg.genBswSnippets`, 기본 on)
- 각 포트의 **Description**에 MASAR 인덱스가 기록됩니다
  (`MASAR=PropSig_as[DevOutp_K81_D]`).
- 전 핀의 **바로 붙여쓰는 통합 코드**를 `<modelName>_bsw_integration.c`로
  자동 생성합니다. 출력은 `.inp_s`에 쓰고, 입력은 `.outp_s`에서 읽는 형태:
```c
HwOutp_s.PropSig_as[DevOutp_K81_D].inp_s.flgSp_l = TRUE;
HwOutp_s.PropSig_as[DevOutp_K81_D].inp_s.dutyCycSp_perml_u16 = /* <LS_K81 duty 0.1%> */;
HwOutp_s.PropSig_as[DevOutp_K81_D].inp_s.stErrReactn_e = /* <ErrReactn> */;
```
→ 통합 시 인덱스를 손으로 타이핑할 필요 없이 복사해서 값만 채우면 됩니다.

## 블록 파라미터 설정 (수작업 모델과 동일하게)

생성되는 각 Inport/Outport에 다음 블록 파라미터를 명시적으로 설정합니다:

- **데이터형** (`OutDataTypeStr`): Type/신호에서 도출한 타입 (uint8/uint16/...)
- **포트 차원** (`PortDimensions`): `cfg.portDimensions` (기본 `'1'`, 스칼라)

## CAN 값 모드 (`cfg.canValueMode`, 기본 `'raw'`)

- `'raw'`: 버스 원시 정수 타입 그대로(`u16`→`uint16` 등). factor/offset 무시.
  수작업 모델과 동일(예: `SteeringAngle_u16`). 스케일 변환은 모델 내부에서.
- `'phys'`: factor≠1 또는 offset≠0이면 물리값으로 보고 `single`(`_f32`)로 생성.

## MASAR 명명 규칙

- 입력: `<배열>_as[DevInp_<핀>_D]`
- 출력: `<배열>_as[DevOutp_<핀>[HS|LS]_D]`
- **HS/LS 접미사**는 MASAR **내부 편의상 명명**이며(하드웨어 사양 아님),
  `Description`의 `High Side`/`Low Side`에서 도출됩니다.
  **`PropPwr`(파워 출력)에만** 붙고 `PropSig`·`DigSig`·`AbsltU`에는 붙지 않습니다.
  `cfg.useHsLsSuffix = false`로 끄면 접미사 없이 생성됩니다.

## Per-pin override (`cfg.overrides`)

Excel이 표현하지 못하는 예외를 처리합니다.
- **K17**: Excel `Description`이 `-`(빈값)이라 HS/LS를 알 수 없음 → 데이터시트
  근거로 `side='HS'` override.
- **K13**: Excel은 `DigitalSignal`이지만 MASAR에서 파워로 재구성 가능
  (RC5-6 헤더 "K12/K13 can be reconfigured as high side outputs"). MASAR가
  실제로 재구성한다면 override를 활성화하세요(주석 처리된 예시 참고).

## CAN 포트 (MASAR 방식)

MASAR Tool은 CAN을 **`CANdbaseEditor.xlsm`의 `Dataset` 시트**로 관리합니다.
이 시트는 신호별로 **`Tx or Rx` 컬럼(AM)에 방향을 명시**하므로, CAN 방향은
DBC 노드에서 추론하지 않고 **이 컬럼 값을 그대로** 사용합니다.

- `cfg.canSource = 'excel'` (기본): `CANdbaseEditor.xlsm` / `Dataset` 시트 사용
- `cfg.canSource = 'dbc'`: 원본 `.dbc`(예: `KIA_ADUS_CAN_v0_6.dbc`) 사용 (fallback)
- `cfg.canDirectionMode = 'masar'` (기본): `Tx or Rx` 컬럼 → `Tx`=Outport, `Rx`=Inport
- `cfg.canDirectionMode = 'node'`: DBC 송신/수신 노드로 추론 (`cfg.ecuNode` 기준)

CAN 방향 로직은 `rc40_can_direction.m` 한 곳에 있으며, 데이터 소스 정규화는
`loadCanSignals`(build 스크립트 내부)가 담당합니다. MASAR 데이터타입
(`u8/u16/u32/i8/i16/i32/u64`)은 Simulink 타입으로 변환되고, 스케일(factor≠1
또는 offset≠0)이 있으면 물리값을 뜻하므로 `single`로 매핑됩니다.

현재 `Dataset` 기준 CAN 포트: **451개** (Tx 152 / Rx 299).
이 제어기의 CAN 노드명은 `Chassis_uC`입니다.

**CAN 포트 이름 (`cfg.canPortNaming`)**
- `'signal'` (기본): 신호명만 사용(`SteeringAngle_u16`). 이름이 겹치는 16개
  신호는 블록명 충돌을 피하려고 `_2` 등 숫자 접미사를 붙입니다.
- `'message'`: 메시지명을 접두(`VDC2_SteerWheelAngle_u16`) → 451개 전부 고유,
  수작업 모델처럼 메시지 컨텍스트 포함.

동일 이름 블록이 이미 있으면 `addPortBlock`이 `_N`을 붙여 자동으로 유일하게
만들므로, 어떤 명명 방식이든 생성이 중단되지 않습니다.

## HW 포트 그룹핑 (`cfg.hwGrouping`, 기본 `'nested'`)

HW 신호 포트는 방향별 최상위 서브시스템(`HwInp`/`HwOutp`) 아래에, 핀 설정값
(Type→HW 클래스)에 따라 하위 서브시스템으로 배치됩니다. 헤더 구조
(`HwInp_s.AnU_as[]`, `HwOutp_s.PropPwr_as[]` ...)와 동일한 계층입니다.

```
RC27_18_Model
├── HwInp
│   ├── AnU     (36 Inport)   ├── Dig  (8)   ├── FrqStd (14)
│   ├── R       (6)           └── Sent (2)
├── HwOutp
│   ├── PropPwr (35 Outport)  ├── DigSig (16)
│   ├── PropSig (4)           └── AbsltU (1)
├── CAN_Rx     (299 Inport)   ← CAN은 HW와 별개
└── CAN_Tx     (152 Outport)
```

옵션:
- `'nested'` (기본): 위처럼 `HwInp/<class>`, `HwOutp/<class>` 2단계
- `'hwinout'`: `HwInp`/`HwOutp` 하나에 전부 평면 배치
- `'byclass'`: `Input_AnU`, `Output_PropPwr` 등 클래스별 단일 서브시스템

Power/Ground/SensorSupply 핀은 포트로 만들지 않습니다(`cfg.includePowerPins=false`).

## 포트 선택 정책 (`cfg.portSelection`)

- `'all'` (기본): 모든 Signal 핀 포트화. `Pin Assignment (MASAR)`가 채워지면
  그 이름을, 없으면 물리 핀 이름(`A01`)을 포트명으로 사용.
- `'assigned'`: `Pin Assignment (MASAR)`가 채워진 핀만 포트화.

## 생성 요약 (현재 Excel 기준)

신호 포트 122개 — AnU 36, PropPwr 35, DigSig 16, FrqStd 14, Dig 8, R 6, PropSig 4, Sent 2, AbsltU 1.
(+ Communication 섹션 활성화 시 DBC 신호별 CAN 포트)
