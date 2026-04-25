# MBusinoLib - an Arduino M-Bus Decoder Library

[![version](https://img.shields.io/badge/version-0.9.19-brightgreen.svg)](CHANGELOG.md)
[![license](https://img.shields.io/badge/license-GPL--3.0-orange.svg)](LICENSE)


## Documentation

The **MBusinoLib** library enables Arduino devices to decode M-Bus (Meterbus) RSP_UD telegrams. (Answer from Slave to Master with data records)

Most M-Bus devices should be supported.

Tested at ESPs, Arduino MKR, pico pi and Uno R4. 8 Bit Arduinos do not work because of missing float64 support.

Live test of the [**MBusinoLib Example**](https://wokwi.com/projects/402235052803622913) at wokwi.com

A working M-Bus --> MQTT gateway with this library [**MBusino**](https://github.com/Zeppelin500/MBusino)

### Credits

**MBusinoLib** based at the AllWize/mbus-payload library but with much more decode capabilities. mbus-payload's encode capabilities are not supported.

Thanks to **AllWize!** for the origin library https://github.com/allwize/mbus-payload 

Thanks to **HWHardsoft** and **TrystanLea** for the M-Bus communication for MBusino: https://github.com/HWHardsoft/emonMbus and https://github.com/openenergymonitor/HeatpumpMonitor

### Class: `MBusinoLib`

Include and instantiate the MBusinoLib class:

```c
#include <MBusinoLib.h>

MBusinoLib decode;
```

## Decoding

### Method: `decodeRecords`

Decodes payload of a whole M-Bus telegram as byte array into a JsonArray (requires ArduinoJson library). The result is an array of objects. The method call returns the number of decoded fields or 0 if error.

```c
uint8_t decodeRecords(uint8_t *buffer, uint8_t size, JsonArray& root);
```

same line from the example
```c
uint8_t fields = decode.decodeRecords(&mbus_data[Startadd], packet_size - Startadd - 2, root); 
```

Example JSON output:

```
[
{
    "value_scaled": 22.06,
    "units": "C",
    "name": "external_temperature_min"
  },
]
```

Example extract the JSON

```c
      for (uint8_t i=0; i<fields; i++) {
        const char* name = root[i]["name"];
        const char* units = root[i]["units"];           
        double value = root[i]["value_scaled"].as<double>(); 
        const char* valueString = root[i]["value_string"];   

        //...send or process the Values
      }
```
### possible contained records
only contained records will be sended

* **["vif"]** contains the VIF(E) as HEX in a string.
* **["value_scaled"]** contains the value of the record as 64 bit real
* **["value_string"]** contains the value of the record as ASCII string (only for Time/Dates and special variable lengs values)
* **["units"]** contains the unit of the value as ASCII string
* **["name"]** contains the name of the value as ASCII string incl. the information of the function field (min, max, err or nothing for instantaneous)
* **["subUnit"]** countains the transmitted sub unit
* **["storage"]** countains the transmitted storage number
* **["tariff"]** countains the transmitted tariff

There are more records available but you have to delete the out comment in the library.

* **["code"]** contains the library internal code of the VIF
* **["scalar"]** contains the scaler of the value
* **["value_raw"]** contains the raw value


### Method: `decodeHeader`

Decodes the header of an M-Bus Long Frame telegram into a JsonObject (requires ArduinoJson library). Returns `true` on success, `false` on error.

```c
bool decodeHeader(const uint8_t* buffer, size_t length, JsonObject& json);
```

- `const uint8_t* buffer`: Pointer to the raw M-Bus telegram (complete frame including start/stop bytes)
- `size_t length`: Total length of the telegram
- `JsonObject& json`: ArduinoJson object where the decoded header fields are written

Usage example:
```c
MBusinoLib decode;

StaticJsonDocument<512> headerDoc;
JsonObject headerObj = headerDoc.to<JsonObject>();

int packet_size = mbus_data[1] + 6;

if (decode.decodeHeader(mbus_data, packet_size, headerObj)) {
    const char* meterId = headerObj["id"].as<const char*>();
    const char* manufacturer = headerObj["manufacturer"].as<const char*>();
    const char* medium = headerObj["medium"].as<const char*>();
    int statusCode = headerObj["status"].as<int>();
    int version = headerObj["version"].as<int>();
    int accessCounter = headerObj["access_counter"].as<int>();

    // Status details
    bool battLow = headerObj["status_details"]["battery_low"].as<bool>();
    bool tempErr = headerObj["status_details"]["temporary_error"].as<bool>();
    bool permErr = headerObj["status_details"]["permanent_error"].as<bool>();
}
```

Example JSON output (from a Cold water meter):

```
{
  "len": 196,
  "c_field": 8,
  "a_field": 20,
  "ci_field": 114,
  "id": "29079172",
  "manufacturer": "WZG",
  "version": 3,
  "medium_code": 22,
  "medium": "Cold water",
  "access_counter": 194,
  "status": 0,
  "status_details": {
    "battery_low": false,
    "permanent_error": false,
    "temporary_error": false,
    "app_status": "no_error"
  },
  "signature": 0
}
```

### possible header fields

* **["len"]** telegram length field
* **["c_field"]** control field
* **["a_field"]** address field
* **["ci_field"]** control information field (0x72 or 0x76 = variable data with header)
* **["id"]** meter identification number as HEX string
* **["manufacturer"]** 3-letter manufacturer code
* **["version"]** meter version/revision number
* **["medium"]** medium name as text (e.g. "Electricity", "Gas", "Water")
* **["medium_code"]** medium code as integer
* **["access_counter"]** transmission counter
* **["status"]** status byte as integer
* **["status_details"]** nested object with decoded status bits: `battery_low`, `permanent_error`, `temporary_error`, `app_status`
* **["signature"]** signature field

### Method: `getError`

Returns the last error ID, once returned the error is reset to OK. Possible error values are:

* `MBUS_ERROR::NO_ERROR`: No error
* `MBUS_ERROR::BUFFER_OVERFLOW`: Buffer cannot hold the requested data, try increasing the buffer size. When decoding: incomming buffer size is wrong.
* `MBUS_ERROR::UNSUPPORTED_CODING`: ~~The library only supports 1,2,3 and 4 bytes integers and 2,4,6 or 8 BCD.~~
* ~~`MBUS_ERROR::UNSUPPORTED_RANGE`: Couldn't encode the provided combination of code and scale, try changing the scale of your value.~~
* `MBUS_ERROR::UNSUPPORTED_VIF`: When decoding: the VIF is not supported and thus it cannot be decoded.

```c
uint8_t getError(void);
```
if you receive error code 4 or an "UNSUPPORTED_VIF", please open an issue

### Home Assistant

The returned units are Home Assistant compatible. 
There are two methods to get the right device- and state-classes for use as Home Assistant sensor.

```c
const char* getDeviceClass(uint8_t code);
```

```c
const char* getStateClass(uint8_t code);
```


## References

* [The M-Bus: A Documentation Rev. 4.8 - Appendix](https://m-bus.com/assets/downloads/MBDOC48.PDF)
* [Dedicated Application Layer (M-Bus)](https://datasheet.datasheetarchive.com/originals/crawler/m-bus.com/ba82a2f0a320ffda901a2d9814f48c24.pdf) by H. Ziegler

## License


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
