# MBusinoLib - an Arduino M-Bus Decoder Library

[![version](https://img.shields.io/badge/version-0.3.0-brightgreen.svg)](CHANGELOG.md)
[![license](https://img.shields.io/badge/license-GPL--3.0-orange.svg)](LICENSE)


## Documentation

The **MBusinoLib** library enables Arduino devices to decode M-Bus (Meterbus) telegrams.

Current, not the whole M-Bus protocol is implemented. But the decoding capabilities still increase. Most M-Bus divices should work.

Tested at ESPs and Arduino MKR boards.

### Credits

**MBusinoLib** based at the AllWize/mbus-payload library but with much more decode capabilities. mbus-payload's encode capabilities are not supported.

Thanks to **AllWize!** for the origin library https://github.com/allwize/mbus-payload 

Thanks to **HWHardsoft** for the M-Bus communication at the example https://github.com/HWHardsoft/emonMbus

### Class: `MBusinoLib`

Include and instantiate the MBusinoLib class. The constructor takes the size of the allocated buffer.

```c
#include <MBusinoLib.h>

MBusinoLib payload(uint8_t size);
```

- `uint8_t size`: The maximum payload size to send, e.g. `512`.

## Decoding

### Method: `decode`

Decodes a byte array into a JsonArray (requires ArduinoJson library). The result is an array of objects, each one containing channel, type, type name and value. The value can be a scalar or an object (for accelerometer, gyroscope and GPS data). The method call returns the number of decoded fields or 0 if error.

```c
uint8_t decode(uint8_t *buffer, uint8_t size, JsonArray& root);
```

same line from the example
```c
uint8_t fields = payload.decode(&mbus_data[Startadd], packet_size - Startadd - 2, root); 
```

Example JSON output:

```
[
{
    "vif": 101,
    "code": 21,
    "scalar": -2,
    "value_raw": 2206,
    "value_scaled": 22.06,
    "units": "C",
    "name": "external_temperature_min"
  },
]
```

Example extract the JSON

```c
      for (uint8_t i=0; i<fields; i++) {
        double value = root[i]["value_scaled"].as<double>();
        const char* name = root[i]["name"];
        const char* units = root[i]["units"];

        //...send or process the Values
      }
```

### Method: `getCodeUnits`

Returns a pointer to a C-string with the unit abbreviation for the given code.


```c
const char * getCodeUnits(uint8_t code);
```

### Method: `getError`

Returns the last error ID, once returned the error is reset to OK. Possible error values are:

* `MBUS_ERROR::NO_ERROR`: No error
* `MBUS_ERROR::BUFFER_OVERFLOW`: Buffer cannot hold the requested data, try increasing the buffer size. When decoding: incomming buffer size is wrong.
* `MBUS_ERROR::UNSUPPORTED_CODING`: ~~The library only supports 1,2,3 and 4 bytes integers and 2,4,6 or 8 BCD.~~
* ~~`MBUS_ERROR::UNSUPPORTED_RANGE`: Couldn't encode the provided combination of code and scale, try changing the scale of your value.~~
* `MBUS_ERROR::UNSUPPORTED_VIF`: When decoding: the VIF is not supported and thus it cannot be decoded.
* `MBUS_ERROR::NEGATIVE_VALUE`: ~~Library only supports non-negative values at the moment.~~

```c
uint8_t getError(void);
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
