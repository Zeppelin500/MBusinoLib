# MBusinoLib change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [0.9.9] - 2025-01-27

### Added

- VIF group FDFDxx added (41 new VIFs)

- therfore some major changes in the lib, like the vif variable to find the definition is now uint64_t and somthing more.

CURRENT_SELECTED_APL,
SUB_DEVICES,
REMAIN_BAT_LIFE_MONTH,
CARBON_DIOXIDE_PPM,
CARBON_MONOXIDE_PPM,
VOLATILE_ORG_COMP_ppb,
VOLATILE_ORG_COMP_UG_M3,
PARTICLES_UNSPEC_UG_M3,
PARTICLES_PM1_UG_M3,
PARTICLES_PM2_5_UG_M3,
PARTICLES_PM10_UG_M3,
PARTICLES_UNSPEC_1M3,
PARTICLES_PM1_1M3,
PARTICLES_PM2_5_1M3,
PARTICLES_PM10_1M3,
ILLUMINANCE_LUX,
LUMINOUS_IDENSITY_CD,
RADIANT_FLUX_DENS,
WIND_SPEED_M_S,
RAINFALL_L_MM,
FORMAZIN_NEPHELOMETER_U,
POTENTIAL_HYDROGEN_PH,
DISMOUNTS_COUNTER,
TEST_BUTTON_COUNTER,
ALARM_COUNTER,
ALARM_MUTE_COUNTER,
OBSTACLE_DETECT_COUNTER,
SMOKE_ENTRIES_COUNTER,
SMOKE_CHAMBER_DEFECTS,
SELF_TEST_COUNTER,
SOUNDER_DEFECT_COUNTER,
DECIBEL_A,
BATTERY_PERCENTAGE,
CHAMBER_POLLUTION_LEVEL,
DISTANCE_MM,
MOISTURE_LEVEL_PERCENT,
PRESSURE_SENS_STATUS,
SMOKE_ALARM_STATUS,
CO_ALARM_STATUS,
HEAT_ALARM_STATUS,
DOOR_WINDOW_SENS_STATUS,

- VIFE name extensions like 0xFC01 for "L1", L1,L2,L3,N,L1-L2,L2-L3,L3-L1,abs.,delta


## [0.9.8] - 2025-01-13

### Added

- VIF FB2C frequency
- VIF FB34 apparent power
- VIF FB02 reactive energy

- additve / multiplicative corretion factor is now working for every VIF

### Changed

- unknown VIFs do not longer stop decoding

## [0.9.7] - 2024-11-05

### Changed

- HA autodiscovery state class energy is now "total"

## [0.9.6] - 2024-09-14

### Added

- VIF 7C customized ascii vif support
- VIF FC VIFE support

## [0.9.5] - 2024-04-16

### Changed

- roll back from float to double in example for a better precision

## [0.9.4] - 2024-04-15

### Added

- vif FB1A rel humidity 

### Changed

- better process for unknown VIFs


## [0.9.3] - 2024-04-11

### Changed

- better real support
- fix failure with more VIFEs

## [0.9.2] - 2024-04-09

### Changed

- fix a problem with floats since last version

## [0.9.1] - 2024-04-05

### Added

- MANUFACTURER_SPECIFIC VIF 0xFF

## [0.9.0] - 2024-03-11

### Added

- Added getStateClass() and getDeviceClass() for home assistant

## [0.8.0] - 2024-03-04

### Changed

- changed some units to home-assistant compatible units

## [0.7.1] - 2024-02-26

### Changed

- delete unused files in example

## [0.7.0] - 2024-02-26

### Added

- Added a much simpler example 

### Changed

- Typos in code: Year JJ --> YY

## [0.6.1] - 2024-01-28

### Changed

- Switch to ArduinoJSON 7

## [0.6.0] - 2024-01-19

### Added

- support of "Codes for Value Information Field Extension (VIFE)" other then 0xFB,0xFC and 0xFD. Only to prevent stumbling, not all Units are implemented 
 

## [0.5.3] - 2023-12-16

### Solved

- compilation problems at Windows machines

## [0.5.2] - 2023-12-13

### Changed

- revised documentation

## [0.5.1] - 2023-12-12

### Changed

- typing error correction

## [0.5.0] - 2023-12-12

### Added

- support sub unit, tariff and storage number

### Changed

- send only important and existing values to the json to save storage

## [0.4.0] - 2023-12-12

### Added

- support for variable length values included ASCII values

## [0.3.0] - 2023-12-04

### Changed

- delete all encode capabilities.

## [0.2.0] - 2023-11-28

### Added

- support for customized ASCII VIF 0hFC 

### Changed

- some tab format for a better clarity.



## [0.1.0] - 2023-11-21
- Initial version
