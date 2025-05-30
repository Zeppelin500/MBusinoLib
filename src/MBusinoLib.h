/*

MBusinoLib, a Arduino M-Bus decoder

Based at the AllWize/mbus-payload library but with much more decode capabilies.

Credits to AllWize!


The MBusinoLib library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The MBusinoLib library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the MBusinoLib library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MBUSINOLIB_H
#define MBUSINOLIB_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define MBUS_DEFAULT_BUFFER_SIZE          254
#define ARDUINO_FLOAT_MIN                 1e-6  // Assume 0 if less than this
#define ARDUINO_FLOAT_DECIMALS            6     // 6 decimals is just below the limit for Arduino float maths

// Supported code types
enum MBUS_CODE {
  //unknown
  UNKNOWN_VIF,

  // no VIFE
  ENERGY_WH, 
  ENERGY_J,
  VOLUME_M3, 
  MASS_KG, 
  ON_TIME_S, 
  ON_TIME_MIN, 
  ON_TIME_H, 
  ON_TIME_DAYS, 
  OPERATING_TIME_S, 
  OPERATING_TIME_MIN, 
  OPERATING_TIME_H, 
  OPERATING_TIME_DAYS, 
  POWER_W,
  POWER_J_H, 
  VOLUME_FLOW_M3_H, 
  VOLUME_FLOW_M3_MIN,
  VOLUME_FLOW_M3_S, 
  MASS_FLOW_KG_H, 
  FLOW_TEMPERATURE_C, 
  RETURN_TEMPERATURE_C, 
  TEMPERATURE_DIFF_K, 
  EXTERNAL_TEMPERATURE_C, 
  PRESSURE_BAR, 
  TIME_POINT_DATE,
  TIME_POINT_DATETIME,
  //HCA,
  AVG_DURATION_S,
  AVG_DURATION_MIN,
  AVG_DURATION_H,
  AVG_DURATION_DAYS,
  ACTUAL_DURATION_S,
  ACTUAL_DURATION_MIN,
  ACTUAL_DURATION_H,
  ACTUAL_DURATION_DAYS,
  FABRICATION_NUMBER,
  BUS_ADDRESS,

  // VIFE 0xFD
  CREDIT,
  DEBIT,
  ACCESS_NUMBER,
  //MEDIUM,
  MANUFACTURER,
  PARAMETER_SET_ID,
  MODEL_VERSION,
  HARDWARE_VERSION,
  FIRMWARE_VERSION,
  SOFTWARE_VERSION,
  CUSTOMER_LOCATION,
  CUSTOMER,
  //ACCESS_CODE_USER,
  //ACCESS_CODE_OPERATOR,
  //ACCESS_CODE_SYSOP,
  //ACCESS_CODE_DEVELOPER,
  //PASSWORD,
  ERROR_FLAGS,
  ERROR_MASK,
  DIGITAL_OUTPUT,
  DIGITAL_INPUT,
  BAUDRATE_BPS,
  RESPONSE_DELAY_TIME,
  RETRY,
  SIZE_OF_STORAGE_BLOCK,
  STORAGE_INTERVAL_MONTH,
  GENERIC,
  VOLTS, 
  AMPERES, 
  RESET_COUNTER,
  CUMULATION_COUNTER,
  SPECIAL_SUPPLIER_INFO,

  // VIFE 0xFDFD
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

  // VIFE 0xFB
	REACTIVE_ENERGY,
  REACTIVE_POWER,
  RELATIVE_HUMIDITY,  
  VOLUME_FT3,
  VOLUME_GAL, 
  VOLUME_FLOW_GAL_M, 
  VOLUME_FLOW_GAL_H,
  APPARENT_POWER, 
  FLOW_TEMPERATURE_F,
  RETURN_TEMPERATURE_F,
  TEMPERATURE_DIFF_F,
  EXTERNAL_TEMPERATURE_F,
  TEMPERATURE_LIMIT_F,
  TEMPERATURE_LIMIT_C,
  MAX_POWER_W,
  PHASE_VOLT_DEG,
  PHASE_CURR_DEG,
  FREQUENCY,

  // VIFE 0xFC
  CUSTOMIZED_VIF,

  // VIFE 0xFF
  MANUFACTURER_SPECIFIC,
  
};

// Error codes
enum MBUS_ERROR {
  NO_ERROR,
  BUFFER_OVERFLOW,
  UNSUPPORTED_CODING,
  UNSUPPORTED_RANGE,
  UNSUPPORTED_VIF,
  NEGATIVE_VALUE,
};

// VIF codes

#define MBUS_VIF_DEF_NUM                  132

typedef struct {
  uint8_t code;
  uint32_t base;
  uint8_t size;
  int8_t scalar;
} vif_def_type;

static const vif_def_type vif_defs[MBUS_VIF_DEF_NUM] = {
  // unknown VIF as user hint. 
  { MBUS_CODE::UNKNOWN_VIF             , 0x7E     , 1,   0}, //0x7E is not used in direction Slave --> Master and therefore misused as library failure hint.

  // No VIFE
  { MBUS_CODE::ENERGY_WH               , 0x00     , 8,  -3},
  { MBUS_CODE::ENERGY_J                , 0x08     , 8,   0},
  { MBUS_CODE::VOLUME_M3               , 0x10     , 8,  -6},
  { MBUS_CODE::MASS_KG                 , 0x18     , 8,  -3},
  { MBUS_CODE::ON_TIME_S               , 0x20     , 1,   0},
  { MBUS_CODE::ON_TIME_MIN             , 0x21     , 1,   0},
  { MBUS_CODE::ON_TIME_H               , 0x22     , 1,   0},
  { MBUS_CODE::ON_TIME_DAYS            , 0x23     , 1,   0},
  { MBUS_CODE::OPERATING_TIME_S        , 0x24     , 1,   0},
  { MBUS_CODE::OPERATING_TIME_MIN      , 0x25     , 1,   0},
  { MBUS_CODE::OPERATING_TIME_H        , 0x26     , 1,   0},
  { MBUS_CODE::OPERATING_TIME_DAYS     , 0x27     , 1,   0},
  { MBUS_CODE::POWER_W                 , 0x28     , 8,  -3},
  { MBUS_CODE::POWER_J_H               , 0x30     , 8,   0},
  { MBUS_CODE::VOLUME_FLOW_M3_H        , 0x38     , 8,  -6},
  { MBUS_CODE::VOLUME_FLOW_M3_MIN      , 0x40     , 8,  -7},
  { MBUS_CODE::VOLUME_FLOW_M3_S        , 0x48     , 8,  -9},
  { MBUS_CODE::MASS_FLOW_KG_H          , 0x50     , 8,  -3},
  { MBUS_CODE::FLOW_TEMPERATURE_C      , 0x58     , 4,  -3},
  { MBUS_CODE::RETURN_TEMPERATURE_C    , 0x5C     , 4,  -3},
  { MBUS_CODE::TEMPERATURE_DIFF_K      , 0x60     , 4,  -3},
  { MBUS_CODE::EXTERNAL_TEMPERATURE_C  , 0x64     , 4,  -3},
  { MBUS_CODE::PRESSURE_BAR            , 0x68     , 4,  -3},
  { MBUS_CODE::TIME_POINT_DATE         , 0x6C     , 1,   0},
  { MBUS_CODE::TIME_POINT_DATETIME     , 0x6D     , 1,   0},
  //{ MBUS_CODE::HCA                     , 0x6E     , 1,   0},
  { MBUS_CODE::AVG_DURATION_S          , 0x70     , 1,   0},
  { MBUS_CODE::AVG_DURATION_MIN        , 0x71     , 1,   0},
  { MBUS_CODE::AVG_DURATION_H          , 0x72     , 1,   0},
  { MBUS_CODE::AVG_DURATION_DAYS       , 0x73     , 1,   0},
  { MBUS_CODE::ACTUAL_DURATION_S       , 0x74     , 1,   0},
  { MBUS_CODE::ACTUAL_DURATION_MIN     , 0x75     , 1,   0},
  { MBUS_CODE::ACTUAL_DURATION_H       , 0x76     , 1,   0},
  { MBUS_CODE::ACTUAL_DURATION_DAYS    , 0x77     , 1,   0},
  { MBUS_CODE::FABRICATION_NUMBER      , 0x78     , 1,   0},
  { MBUS_CODE::BUS_ADDRESS             , 0x7A     , 1,   0},

  { MBUS_CODE::VOLUME_M3               , 0x933A   , 1,   -3},
  { MBUS_CODE::VOLUME_M3               , 0x943A   , 1,   -2},

  // VIFE 0xFD
  { MBUS_CODE::CREDIT                  , 0xFD00   ,  4,  -3},
  { MBUS_CODE::DEBIT                   , 0xFD04   ,  4,  -3},
  { MBUS_CODE::ACCESS_NUMBER           , 0xFD08   ,  1,   0},
  //{ MBUS_CODE::MEDIUM                  , 0xFD09   ,  1,   0},
  { MBUS_CODE::MANUFACTURER            , 0xFD0A   ,  1,   0},
  { MBUS_CODE::PARAMETER_SET_ID        , 0xFD0B   ,  1,   0},
  { MBUS_CODE::MODEL_VERSION           , 0xFD0C   ,  1,   0},
  { MBUS_CODE::HARDWARE_VERSION        , 0xFD0D   ,  1,   0},
  { MBUS_CODE::FIRMWARE_VERSION        , 0xFD0E   ,  1,   0},
  { MBUS_CODE::SOFTWARE_VERSION        , 0xFD0F   ,  1,   0},  //neu
  { MBUS_CODE::CUSTOMER_LOCATION       , 0xFD10   ,  1,   0},
  { MBUS_CODE::CUSTOMER                , 0xFD11   ,  1,   0},
  //{ MBUS_CODE::ACCESS_CODE_USER        , 0xFD12   ,  1,   0},
  //{ MBUS_CODE::ACCESS_CODE_OPERATOR    , 0xFD13   ,  1,   0},
  //{ MBUS_CODE::ACCESS_CODE_SYSOP       , 0xFD14   ,  1,   0},
  //{ MBUS_CODE::ACCESS_CODE_DEVELOPER   , 0xFD15   ,  1,   0},
  //{ MBUS_CODE::PASSWORD                , 0xFD16   ,  1,   0},
  { MBUS_CODE::ERROR_FLAGS             , 0xFD17   ,  1,   0},
  { MBUS_CODE::ERROR_MASK              , 0xFD18   ,  1,   0},
  { MBUS_CODE::DIGITAL_OUTPUT          , 0xFD1A   ,  1,   0},
  { MBUS_CODE::DIGITAL_INPUT           , 0xFD1B   ,  1,   0},
  { MBUS_CODE::BAUDRATE_BPS            , 0xFD1C   ,  1,   0},
  { MBUS_CODE::RESPONSE_DELAY_TIME     , 0xFD1D   ,  1,   0},
  { MBUS_CODE::RETRY                   , 0xFD1E   ,  1,   0},
  { MBUS_CODE::SIZE_OF_STORAGE_BLOCK   , 0xFD22   ,  1,   0},
  { MBUS_CODE::STORAGE_INTERVAL_MONTH  , 0xFD28   ,  1,   0},
  { MBUS_CODE::GENERIC                 , 0xFD3A   ,  1,   0},
  { MBUS_CODE::VOLTS                   , 0xFD40   , 16,  -9},
  { MBUS_CODE::AMPERES                 , 0xFD50   , 16, -12},
  { MBUS_CODE::RESET_COUNTER           , 0xFD60   ,  1,   0},
  { MBUS_CODE::CUMULATION_COUNTER      , 0xFD61   ,  1,   0},
  { MBUS_CODE::SPECIAL_SUPPLIER_INFO   , 0xFD67   ,  1,   0},

  // VIFE 0xFDFD
  { MBUS_CODE::CURRENT_SELECTED_APL    , 0xFDFD00 , 1,   0},   
  { MBUS_CODE::SUB_DEVICES             , 0xFDFD01 , 1,   0},
  { MBUS_CODE::REMAIN_BAT_LIFE_MONTH   , 0xFDFD02 , 1,   0}, 
  { MBUS_CODE::CARBON_DIOXIDE_PPM      , 0xFDFD10 , 2,   0}, 
  { MBUS_CODE::CARBON_MONOXIDE_PPM     , 0xFDFD12 , 2,   0}, 
  { MBUS_CODE::VOLATILE_ORG_COMP_ppb   , 0xFDFD14 , 2,   0}, 
  { MBUS_CODE::VOLATILE_ORG_COMP_UG_M3 , 0xFDFD16 , 1,   0},   
  { MBUS_CODE::PARTICLES_UNSPEC_UG_M3  , 0xFDFD17 , 1,   0},  
  { MBUS_CODE::PARTICLES_PM1_UG_M3     , 0xFDFD18 , 1,   0},  
  { MBUS_CODE::PARTICLES_PM2_5_UG_M3   , 0xFDFD19 , 1,   0}, 
  { MBUS_CODE::PARTICLES_PM10_UG_M3    , 0xFDFD1A , 1,   0}, 
  { MBUS_CODE::PARTICLES_UNSPEC_1M3    , 0xFDFD1B , 1,   5},  
  { MBUS_CODE::PARTICLES_PM1_1M3       , 0xFDFD1C , 1,   5}, 
  { MBUS_CODE::PARTICLES_PM2_5_1M3     , 0xFDFD1D , 1,   5}, 
  { MBUS_CODE::PARTICLES_PM10_1M3      , 0xFDFD1E , 1,   5},   
  { MBUS_CODE::ILLUMINANCE_LUX         , 0xFDFD1F , 1,   0}, 
  { MBUS_CODE::LUMINOUS_IDENSITY_CD    , 0xFDFD20 , 1,   0}, 
  { MBUS_CODE::RADIANT_FLUX_DENS       , 0xFDFD21 , 1,   0}, 
  { MBUS_CODE::WIND_SPEED_M_S          , 0xFDFD22 , 1,   0}, 
  { MBUS_CODE::RAINFALL_L_MM           , 0xFDFD23 , 1,   0},   
  { MBUS_CODE::FORMAZIN_NEPHELOMETER_U , 0xFDFD25 , 1,   0},   
  { MBUS_CODE::POTENTIAL_HYDROGEN_PH   , 0xFDFD27 , 1,   0}, 
  { MBUS_CODE::DISMOUNTS_COUNTER       , 0xFDFD2C , 1,   0}, 
  { MBUS_CODE::TEST_BUTTON_COUNTER     , 0xFDFD2D , 1,   0}, 
  { MBUS_CODE::ALARM_COUNTER           , 0xFDFD2E , 1,   0}, 
  { MBUS_CODE::ALARM_MUTE_COUNTER      , 0xFDFD2F , 1,   0}, 
  { MBUS_CODE::OBSTACLE_DETECT_COUNTER , 0xFDFD30 , 1,   0}, 
  { MBUS_CODE::SMOKE_ENTRIES_COUNTER   , 0xFDFD31 , 1,   0}, 
  { MBUS_CODE::SMOKE_CHAMBER_DEFECTS   , 0xFDFD32 , 1,   0}, 
  { MBUS_CODE::SELF_TEST_COUNTER       , 0xFDFD33 , 1,   0},   
  { MBUS_CODE::SOUNDER_DEFECT_COUNTER  , 0xFDFD34 , 1,   0}, 
  { MBUS_CODE::DECIBEL_A               , 0xFDFD36 , 1,   0},  
  { MBUS_CODE::BATTERY_PERCENTAGE      , 0xFDFD38 , 1,   0}, 
  { MBUS_CODE::CHAMBER_POLLUTION_LEVEL , 0xFDFD39 , 1,   0}, 
  { MBUS_CODE::DISTANCE_MM             , 0xFDFD3A , 2,   0}, 
  { MBUS_CODE::MOISTURE_LEVEL_PERCENT  , 0xFDFD3E , 1,   0}, 
  { MBUS_CODE::PRESSURE_SENS_STATUS    , 0xFDFD40 , 1,   0}, 
  { MBUS_CODE::SMOKE_ALARM_STATUS      , 0xFDFD41 , 1,   0},
  { MBUS_CODE::CO_ALARM_STATUS         , 0xFDFD42 , 1,   0},   
  { MBUS_CODE::HEAT_ALARM_STATUS       , 0xFDFD43 , 1,   0}, 
  { MBUS_CODE::DOOR_WINDOW_SENS_STATUS , 0xFDFD44 , 1,   0}, 

  // VIFE 0xFB
  { MBUS_CODE::ENERGY_WH               , 0xFB00   , 2,   5},
  { MBUS_CODE::REACTIVE_ENERGY         , 0xFB02   , 2,   0},
  { MBUS_CODE::ENERGY_J                , 0xFB08   , 2,   8},
  { MBUS_CODE::VOLUME_M3               , 0xFB10   , 2,   2},
  { MBUS_CODE::REACTIVE_POWER          , 0xFB14   , 4,  -3},  
  { MBUS_CODE::MASS_KG                 , 0xFB18   , 2,   5},
  { MBUS_CODE::RELATIVE_HUMIDITY       , 0xFB1A   , 2,  -1},

  { MBUS_CODE::VOLUME_FT3              , 0xFB21   , 1,  -1},
  { MBUS_CODE::VOLUME_GAL              , 0xFB22   , 2,  -1},
  { MBUS_CODE::VOLUME_FLOW_GAL_M       , 0xFB24   , 1,  -3},
  { MBUS_CODE::VOLUME_FLOW_GAL_M       , 0xFB25   , 1,   0},
  { MBUS_CODE::VOLUME_FLOW_GAL_H       , 0xFB26   , 1,   0},
  { MBUS_CODE::POWER_W                 , 0xFB28   , 2,   5},
  { MBUS_CODE::PHASE_VOLT_DEG          , 0xFB2A   , 1,  -1},
  { MBUS_CODE::PHASE_CURR_DEG          , 0xFB2B   , 1,  -1},
  { MBUS_CODE::FREQUENCY               , 0xFB2C   , 4,  -3},
  { MBUS_CODE::POWER_J_H               , 0xFB30   , 2,   8},
  { MBUS_CODE::APPARENT_POWER          , 0xFB34   , 4,  -3},
  { MBUS_CODE::FLOW_TEMPERATURE_F      , 0xFB58   , 4,  -3},
  { MBUS_CODE::RETURN_TEMPERATURE_F    , 0xFB5C   , 4,  -3},
  { MBUS_CODE::TEMPERATURE_DIFF_F      , 0xFB60   , 4,  -3},
  { MBUS_CODE::EXTERNAL_TEMPERATURE_F  , 0xFB64   , 4,  -3},
  { MBUS_CODE::TEMPERATURE_LIMIT_F     , 0xFB70   , 4,  -3},
  { MBUS_CODE::TEMPERATURE_LIMIT_C     , 0xFB74   , 4,  -3},
  { MBUS_CODE::MAX_POWER_W             , 0xFB78   , 8,  -3},

  // VIFE 0xFC
  { MBUS_CODE::CUSTOMIZED_VIF          , 0xFC00   , 254,  0},

  // VIFE 0xFF
  { MBUS_CODE::MANUFACTURER_SPECIFIC   , 0xFF00   , 254,  0}

};

class MBusinoLib {

public:

  MBusinoLib(uint8_t size = MBUS_DEFAULT_BUFFER_SIZE);
  ~MBusinoLib();

  void reset(void);
  uint8_t getSize(void);
  uint8_t * getBuffer(void);
  uint8_t copy(uint8_t * buffer);
  uint8_t getError();
  
  uint8_t decode(uint8_t *buffer, uint8_t size, JsonArray& root);
  const char * getCodeName(uint8_t code);
  const char * getCodeUnits(uint8_t code);
  const char * getDeviceClass(uint8_t code);
  const char * getStateClass(uint8_t code);
  
protected:

  int16_t _findDefinition(uint32_t vif);
  uint32_t _getVIF(uint8_t code, int8_t scalar);

  uint8_t * _buffer;
  uint8_t _maxsize;
  uint8_t _cursor;
  uint8_t _error = NO_ERROR;

};
#endif
