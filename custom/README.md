# VibeAILab QGC

Custom QGroundControl build for our own flight controller firmware (quad).
Base: QGC v5.1.5, branch `vibeailab`. All VibeAILab code lives in this folder;
core QGC files are never edited.

## Build

Needs Qt 6.11.1 with all Additional Libraries.

```bash
just configure
JOBS=4 just build
```

This folder must always contain both `CMakeLists.txt` and
`cmake/CustomOverrides.cmake`, or configure fails.

## Layout

| Path | Holds |
| ---- | ----- |
| `src/Integration/` | Classes QGC calls directly: core plugin, firmware plugin factory, firmware plugin, autopilot (setup) plugin, param metadata |
| `src/Features/<Name>/` | One folder per feature with its own code; its QML in `qml/` |
| `resources/` | Data and images, bundled by `vibeailab.qrc` |

## Features

| Feature | Setup page | Follows |
| ------- | ---------- | ------- |
| `Sensors` | Sensors: Gyro Calibration, Accel Calibration | The FC's replies to the calibration command and its calibration status texts |
| `Tuning` | PID Tuning: Rate Controller and Attitude Controller tabs, Thrust Curve | The FC params `RATE_P/I/D_X/Y/Z`, `ATT_P_X/Y/Z`, `MOT_THR_CURVE`; the FC's `ATTITUDE_QUATERNION` and `ATTITUDE_TARGET` |

## Adding a feature

| Need | QGC hook | Put code in |
| ---- | -------- | ----------- |
| New Setup page | `AutoPilotPlugin::vehicleComponents()` + a `VehicleComponent` + QML | `src/Features/<Name>/`, register in `VibeAILabAutoPilotPlugin` |
| New Analyze tool | `QGCCorePlugin::analyzePages()` | `src/Features/<Name>/`, register in `VibeAILabCorePlugin` |
| Toolbar indicator | `QGCCorePlugin::toolBarIndicators()` or `FirmwarePlugin::toolIndicators()` | `src/Features/<Name>/` |
| Handle our own MAVLink messages | `QGCCorePlugin::mavlinkMessage()` | `src/Features/<Name>/` |
| Brand colours, logo | `QGCCorePlugin::paletteOverride()` + images | `src/Integration/` + `resources/` |
| Hide or adjust settings | `QGCOptions`, `overrideSettingsGroupVisibility()`, `adjustSettingMetaData()` | `src/Integration/` |
| Param description, limits, decimal places | `FirmwarePlugin::_createParameterMetaData()` (`VibeAILabParameterMetaData`) | `resources/ParameterMetaData.json` |
| QML control used by 2+ features | - | `qml/Controls/` |

A feature's QML goes in its own QML module in `CMakeLists.txt`.

## Rules

- Class names start with `VibeAILab`; files are `ClassName.h` / `ClassName.cc`.
  With `QGC_UNITY_BUILD` on, several `.cc` files compile together, so names must be unique.
- Params always go through the Fact System; no separate param storage.
- Never copy a core QGC file into `custom/`; discuss first if core must change.
