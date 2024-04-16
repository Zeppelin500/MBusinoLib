# MBusinoLib change log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

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
