/*

MBusinoLib, an Arduino M-Bus decoder

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

#include "MBusinoLib.h"

// ----------------------------------------------------------------------------

MBusinoLib::MBusinoLib(uint8_t size) : _maxsize(size) {
  _buffer = (uint8_t *) malloc(size);
  _cursor = 0;
}

MBusinoLib::~MBusinoLib(void) {
  free(_buffer);
}

void MBusinoLib::reset(void) {
  _cursor = 0;
}

uint8_t MBusinoLib::getSize(void) {
  return _cursor;
}

uint8_t *MBusinoLib::getBuffer(void) {
  return _buffer;
}

uint8_t MBusinoLib::copy(uint8_t *dst) {
  memcpy(dst, _buffer, _cursor);
  return _cursor;
}

uint8_t MBusinoLib::getError() {
  uint8_t error = _error;
  _error = MBUS_ERROR::NO_ERROR;
  return error;
}

// ----------------------------------------------------------------------------

uint8_t MBusinoLib::decode(uint8_t *buffer, uint8_t size, JsonArray& root) {

	uint8_t count = 0;
	uint8_t index = 0;
	
	while (index < size) {

    	count++;

    	// Decode DIF
    	uint8_t dif = buffer[index++];
    	uint8_t difLeast4bit = (dif & 0x0F);
		  uint8_t difFunctionField = ((dif & 0x30) >> 4);
    	uint8_t len = 0;
    	uint8_t dataCodingType = 0;
    	/*
    	0 -->   no Data
    	1 -->   integer
    	2 -->   bcd
    	3 -->   real
    	4 -->   variable lengs
    	5 -->   special functions
    	6 -->   TimePoint Date&Time Typ F
    	7 -->   TimePoint Date Typ G

    	Length in Bit	Code	    Meaning	        Code	Meaning
    	0	            0000	    No data	        1000	Selection for Readout
    	8	            0001	    8 Bit Integer	1001	2 digit BCD
    	16	            0010	    16 Bit Integer	1010	4 digit BCD
    	24	            0011	    24 Bit Integer	1011	6 digit BCD
    	32	            0100	    32 Bit Integer	1100	8 digit BCD
    	32 / N	        0101	    32 Bit Real	    1101	variable length
    	48	            0110	    48 Bit Integer	1110	12 digit BCD
    	64	            0111	    64 Bit Integer	1111	Special Functions 
    	*/
    
    	switch(difLeast4bit){
        	case 0x00:  //No data
            	len = 0;
            	dataCodingType = 0;
            	break;        
        	case 0x01:  //0001	    8 Bit Integer
            	len = 1;
            	dataCodingType = 1;
            	break;
        	case 0x02:  //0010	    16 Bit Integer
            	len = 2;
            	dataCodingType = 1;
            	break;
        	case 0x03:  //0011	    24 Bit Integer
            	len = 3;
            	dataCodingType = 1;
            	break;
        	case 0x04:  //0100	    32 Bit Integer
            	len = 4;
            	dataCodingType = 1;
            	break;
        	case 0x05:  //0101	    32 Bit Real
            	len = 4;
            	dataCodingType = 3;
            	break;        
        	case 0x06:  //0110	    48 Bit Integer
            	len = 6;
            	dataCodingType = 1;
            	break; 
        	case 0x07:  //0111	    64 Bit Integer
            	len = 8;
            	dataCodingType = 1;
            	break;
        	case 0x08:  //not supported
            	len = 0;
            	dataCodingType = 0;
            	break;
        	case 0x09:  //1001 2 digit BCD
            	len = 1;
            	dataCodingType = 2;
            	break;
        	case 0x0A:  //1010 4 digit BCD
            	len = 2;
            	dataCodingType = 2;
            	break;
        	case 0x0B:  //1011 6 digit BCD
            	len = 3;
            	dataCodingType = 2;
            	break;
        	case 0x0C:  //1100 8 digit BCD
            	len = 4;
            	dataCodingType = 2;
            	break;
        	case 0x0D:  //1101	variable length
            	len = 0;
            	dataCodingType = 4;
            	break;
        	case 0x0E:  //1110	12 digit BCD
            	len = 6;
            	dataCodingType = 2;
            	break;
        	case 0x0F:  //1111	Special Functions
            	len = 0;
            	dataCodingType = 5;
            	break;
    	}
		char stringFunctionField[5];
		switch(difFunctionField){
			case 0:
				strcpy(stringFunctionField, ""); //current
				break;
			case 1:
				strcpy(stringFunctionField, "_max");
				break;
			case 2:
				strcpy(stringFunctionField, "_min");
				break;
			case 3:
				strcpy(stringFunctionField, "_err");
				break;
		}
			
			
    // handle DIFE
    uint16_t storageNumber = 0;
    if((dif & 0x40) == 0x40){
      storageNumber = 1;
    }
    uint8_t difeNumber = 0;
    uint8_t dife[10] = {0};
    bool ifDife = ((dif & 0x80) == 0x80); //check if the first bit of DIF marked as "DIFE is following" 
    while(ifDife) {        
        difeNumber ++;
        dife[difeNumber] = buffer[index];
      	ifDife = false;
      	ifDife = ((buffer[index] & 0x80) == 0x80); //check if after the DIFE another DIFE is following 
        index++;
    } 
    uint8_t subUnit = 0;
    uint8_t tariff = 0;
    
    for(uint8_t i = 0; difeNumber > 0 && i<=difeNumber; i++){    
      if(i==0){
        storageNumber = storageNumber | ((dife[i+1] & 0x0F) << 1);
      }
      else{
        storageNumber = storageNumber | ((dife[i+1] & 0x0F) << (4*i));
      }
      subUnit = subUnit | (((dife[i+1] & 0x40) >> 6) << i);
      tariff = tariff | (((dife[i+1] & 0x30) >> 4) << (2*i));
    }
    
    //  End of DIFE handling
 

    // Get VIF(E)
    uint32_t vif = 0;
    uint8_t vifarray[10] = {0};
    uint8_t vifcounter = 0;

    uint8_t vifes = 0;
    uint8_t customVIFlen = 0;
    char customVIF[20] = {0}; 
		bool ifcustomVIF = false;	
    uint8_t firstVifeExtension = 1; //8.4.5 Codes for Value Information Field Extension (VIFE)

    do { // copy the VIF(E)s to an array for later analysys and the variable vif for the devinition 
      if (index == size) {
        _error = MBUS_ERROR::BUFFER_OVERFLOW;
        return 0;
      }

      vifarray[vifcounter] = buffer[index++];
      
      if((vifarray[0] & 0x7F) == 0x7C && vifcounter == 0){ // Customized ASCII VIF
        customVIFlen = buffer[index];// length of ASCII VIF
        if(vifarray[0] == 0xFC){
          index = index + customVIFlen + 1; // SKIPskip the ASCII string to find the vife
        }
      }

      if(vifcounter < 2){   // only vif and first vife will be stored in vif normaly....
        vif = (vif << 8) + vifarray[vifcounter];
      }
      else if(vifcounter == 2 && vif == 0xFDFD){   // ... but FIV 0xFDFD uses the third Byte to assign the deffinition. 
        vif = (vif << 8) + vifarray[vifcounter];
      }

      vifcounter++;
    } while ((vifarray[vifcounter-1] & 0x80) == 0x80);

    if(((vifarray[0] & 0x80) == 0x80) && (vifarray[0] != 0xFD) && (vifarray[0] != 0xFC)&& (vifarray[0] != 0xFB) && (vifarray[0] != 0xFF)){ //if the true vif is in the first byte
      vif = (vifarray[0] & 0x7F);
    }
    
    if(vifarray[0] == 0x7C){ // Customized ASCII VIF
      vif = 0x7C00;
    }

    if((vif & 0x7F) == 0x6D){  // TimePoint Date&Time TypF
      dataCodingType = 6;   
    }
    else if((vif & 0x7F) == 0x6C){  // TimePoint Date TypG
      dataCodingType = 7;  
    }
    else if((vif & 0x7F00) == 0x7C00){   // VIF 0xFC / 0x7c --> Customized VIF as ASCII
      vif = 0xFC00;
      if(vifarray[0] == 0xFC){
        index = index - customVIFlen -1; // befor, we skipped the ASCII bytes to find the VIFEs, so we set the index back to reed the ASCII string
      }
      
      char vifBuffer[customVIFlen];// = {0};    
      for (uint8_t i = 0; i<=customVIFlen; i++) { // reeding the ASCII string ...
        vifBuffer[customVIFlen-i] = buffer[index-vifcounter + 1]; 
				index++;
      }  
      strncpy(customVIF, vifBuffer,customVIFlen ); // ... and copy it to our variable
			ifcustomVIF = true;	
    }

    vif = (vif & 0xFFFFFF7F); // delete the leeding zero of the last vif byte, if additional vifes follow, to find the right definition

    // Find definition
    int16_t def = _findDefinition(vif);
    if (def < 0) {
      _error = MBUS_ERROR::UNSUPPORTED_VIF;
      def = 0; 
      //return 0; 
    }	  

    //find VIF extensions like the table 8.4.5 Codes for Value Information Field Extension (VIFE) ---------------------------------------------------------------

    int8_t extensionScaler = 0; // additional scaler (x * 10 ^extensionScaler)
    double extensionAdditiveConstant = 0;
    char stringNameExtension[10] = {0}; //needed for VIF name extensions like L1,L2,L3 ans so on e.g: Voltage L1 

    if(vifcounter-1 > 0){

      if(vifarray[0] == 0xFB || (vifarray[0] == 0xFD && vifarray[1] != 0xFD) || vifarray[0] == 0xFF){  // to find the first vife after the true vif
        firstVifeExtension = 2;    
      }
      else if (vifarray[0] == 0xFD && vifarray[1] == 0xFD){
        firstVifeExtension = 3;
      }
      else{
        firstVifeExtension = 1;
      }

      uint8_t extensionsCounter = firstVifeExtension;

      do{
        if((vifarray[extensionsCounter] & 0x7F) == 0x7D){ // from table "Codes for Value Information Field Extension" E111 1101	Multiplicative correction factor: 1000
          extensionScaler = 3; // x1000
        }
        else if((vifarray[extensionsCounter] & 0x78) == 0x70){ // from table "Codes for Value Information Field Extension" E111 0nnn	Multiplicative correction factor: 10nnn-6
          extensionScaler = (vifarray[extensionsCounter] & 7) - 6;
        }
        else if((vifarray[extensionsCounter] & 0x7C) == 0x78){ // from table "Codes for Value Information Field Extension" E111 10nn	Additive correction constant: 10nn-3 • unit of VIF (offset)
          int8_t extensionAdditiveConstantScaler = 0;
          extensionAdditiveConstantScaler = (vifarray[extensionsCounter] & 3) - 3;
          extensionAdditiveConstant = 1;
          for (int8_t i=0; i<extensionAdditiveConstantScaler; i++) extensionAdditiveConstant *= 10;
          for (int8_t i=extensionAdditiveConstantScaler; i<0; i++) extensionAdditiveConstant /= 10;
        }
        else if(vifarray[extensionsCounter] == 0xFC || vifarray[extensionsCounter] == 0xFF){
          uint8_t vifExtensionBuffer = vifarray[extensionsCounter+1] & 0x7F;
          switch(vifExtensionBuffer){  
            case 1:
              strcpy(stringNameExtension, "_L1");
              extensionsCounter++;  
              break;
            case 2:
              strcpy(stringNameExtension, "_L2");
              extensionsCounter++;  
              break;
            case 3:
              strcpy(stringNameExtension, "_L3");
              extensionsCounter++;  
              break;
            case 4:
              strcpy(stringNameExtension, "_N");
              extensionsCounter++;  
              break;
            case 5:
              strcpy(stringNameExtension, "_L1-L2");
              extensionsCounter++;  
              break;
            case 6:
              strcpy(stringNameExtension, "_L2-L3");
              extensionsCounter++;  
              break;
            case 7:
              strcpy(stringNameExtension, "_L3-L1");
              extensionsCounter++;  
              break;  
            case 10:
              strcpy(stringNameExtension, "_abs.");
              extensionsCounter++;  
              break;    
            case 12:
              strcpy(stringNameExtension, "_delta");
              extensionsCounter++;  
              break;                                                                                                                                      
            break;
          }
        }
        extensionsCounter++;  
      }while(extensionsCounter <= vifcounter);

    }
    
    // Check buffer overflow
    if (index + len > size) {
      _error = MBUS_ERROR::BUFFER_OVERFLOW;
      return 0;
    }
  
    // read value
    int16_t value16 = 0;  	// int16_t to notice negative values at 2 byte data
    int32_t value32 = 0;	// int32_t to notice negative values at 4 byte data	  
    int64_t value = 0;

    float valueFloat = 0; //real value
	  
	  
	  uint8_t date[len]; // ={0};
	  char datestring[16] = {0}; //needed for simple formatted dates 
	  char datestring2[30] = {0};//needed for extensive formatted dates
    char valueString [30] = {0}; // contain the ASCII value at variable length ascii values and formatted dates
    bool switchAgain = false; // repeat the switch(dataCodingType) at variable length coding
    bool negative = false;  // set a negative flag for negate the value
    uint8_t asciiValue = 0; // 0 = double, 1 = ASCII, 2 = both;

    do{
      switchAgain = false;
      switch(dataCodingType){
        case 0:    //no Data
              
          break;
        case 1:    //integer
          if(len==2){
            for (uint8_t i = 0; i<len; i++) {
              value16 = (value16 << 8) + buffer[index + len - i - 1];
            }
            value = (int64_t)value16;
          }
          else if(len==4){
            for (uint8_t i = 0; i<len; i++) {
              value32 = (value32 << 8) + buffer[index + len - i - 1];
            }	
            value = (int64_t)value32;
          }			
          else{
            for (uint8_t i = 0; i<len; i++) {
              value = (value << 8) + buffer[index + len - i - 1];
            }            
          }
          break;
        case 2:    //bcd
          if(len==2){
            for (uint8_t i = 0; i<len; i++) {
              uint8_t byte = buffer[index + len - i - 1];
              if((i == 0) && ((byte & 0xF0) == 0xF0)){ //Although only unsigned BCDs are intended, some manufacturers use an F in the first halfbyte to mark negative numbers
                byte = byte & 0x0F;
                negative = true;
              }
              value16 = (value16 * 100) + ((byte >> 4) * 10) + (byte & 0x0F);
            }
            value = (int64_t)value16;
          }
          else if(len==4){
            for (uint8_t i = 0; i<len; i++) {
              uint8_t byte = buffer[index + len - i - 1];
              if((i == 0) && ((byte & 0xF0) == 0xF0)){ //Although only unsigned BCDs are intended, some manufacturers use an F in the first halfbyte to mark negative numbers
                byte = byte & 0x0F;
                negative = true;
              }
              value32 = (value32 * 100) + ((byte >> 4) * 10) + (byte & 0x0F);
            }
            value = (int64_t)value32;				 
          }
          else{
            for (uint8_t i = 0; i<len; i++) {
              uint8_t byte = buffer[index + len - i - 1];
              if((i == 0) && ((byte & 0xF0) == 0xF0)){ //Although only unsigned BCDs are intended, some manufacturers use an F in the first halfbyte to mark negative numbers
                byte = byte & 0x0F;
                negative = true;
              }
              value = (value * 100) + ((byte >> 4) * 10) + (byte & 0x0F);
            }           
          } 
          if(negative == true){
            value = value * -1;
          }    
          break;
        case 3:    //real
          for (uint8_t i = 0; i<len; i++) {
            value = (value << 8) + buffer[index + len - i - 1];
          } 
          memcpy(&valueFloat, &value, sizeof(valueFloat));
          break;
        case 4:    //variable lengs
          /*
          Variable Length:
          With data field = `1101b` several data types with variable length can be used. The length of
          the data is given with the first byte of data, which is here called LVAR.
          LVAR = 00h .. BFh :ASCII string with LVAR characters
          LVAR = C0h .. CFh :positive BCD number with (LVAR - C0h) • 2 digits
          LVAR = D0h .. DFH :negative BCD number with (LVAR - D0h) • 2 digits
          LVAR = E0h .. EFh :binary number with (LVAR - E0h) bytes
          LVAR = F0h .. FAh :floating point number with (LVAR - F0h) bytes [to bedefined]
          LVAR = FBh .. FFh :Reserved
          */
          // only ASCII string supported
          if(buffer[index] >= 0x00 && buffer[index] <= 0xBF){ //ASCII string with LVAR characters
            len = buffer[index];
            index ++;
            char charBuffer[len]; // = {0};        
            for (uint8_t i = 0; i<=len; i++) { // evtl das "=" löschen
              charBuffer[i] = buffer[index + (len-i-1)]; 
            }  
            strncpy(valueString, charBuffer,len );  
            asciiValue = 1;	
          }
          else if(buffer[index] >= 0xC0 && buffer[index] <= 0xCF){ //positive BCD number with (LVAR - C0h) • 2 digits
            len = buffer[index] - 0xC0;
            index ++;
            dataCodingType = 2;
            switchAgain = true;
            break;
          }
          else if(buffer[index] >= 0xD0 && buffer[index] <= 0xDF){ //negative BCD number with (LVAR - D0h) • 2 digits
            len = buffer[index] - 0xD0;
            index ++;
            dataCodingType = 2;
            switchAgain = true;
            negative = true;
            break;
          }    
          else if(buffer[index] >= 0xE0 && buffer[index] <= 0xEF){ //binary number with (LVAR - E0h) bytes
            len = buffer[index] - 0xE0;
            index ++;            
            dataCodingType = 1;
            switchAgain = true;
            break;
          }    
          else if(buffer[index] >= 0xF0 && buffer[index] <= 0xFA){ //floating point number with (LVAR - F0h) bytes [to bedefined]
            len = buffer[index] - 0xF0;
            index ++;            
            dataCodingType = 3;
            switchAgain = true;
            break;
          } 

          break;
        case 5:    //special functions
          
          break;
        case 6:    //TimePoint Date&Time Typ F
          for (uint8_t i = 0; i<len; i++) {
            date[i] =  buffer[index + i];
          }            			
          if ((date[0] & 0x80) != 0) {    // Time valid ?
            //out_len = snprintf(output, output_size, "invalid");
            break;
          }
          snprintf(datestring, 24, "20%02d%02d%02d%02d%02d",
            ((date[2] & 0xE0) >> 5) | ((date[3] & 0xF0) >> 1), // year
            date[3] & 0x0F, // mon
            date[2] & 0x1F, // mday
            date[1] & 0x1F, // hour
            date[0] & 0x3F // min
          );  
        
          snprintf(datestring2, 24, "20%02d-%02d-%02d %02d:%02d:00",
            ((date[2] & 0xE0) >> 5) | ((date[3] & 0xF0) >> 1), // year
            date[3] & 0x0F, // mon
            date[2] & 0x1F, // mday
            date[1] & 0x1F, // hour
            date[0] & 0x3F // min
          );	
          value = atof( datestring);
          strcpy(valueString, datestring2);
          asciiValue = 2;	
          break;
        case 7:    //TimePoint Date Typ G
          for (uint8_t i = 0; i<len; i++) {
            date[i] =  buffer[index + i];
          }            
        
          if ((date[1] & 0x0F) > 12) {    // Time valid ?
            //out_len = snprintf(output, output_size, "invalid");
            break;
          }
          snprintf(datestring, 12, "20%02d%02d%02d",
            ((date[0] & 0xE0) >> 5) | ((date[1] & 0xF0) >> 1), // year
            date[1] & 0x0F, // mon
            date[0] & 0x1F  // mday
          );
          snprintf(datestring2, 12, "20%02d-%02d-%02d",
            ((date[0] & 0xE0) >> 5) | ((date[1] & 0xF0) >> 1), // year
            date[1] & 0x0F, // mon
            date[0] & 0x1F  // mday
          );			
          value = atof( datestring);
          strcpy(valueString, datestring2);
          asciiValue = 2;	
          break;
        default:
          break;
      }
    }while (switchAgain == true);

    index += len;

    // scaled value
    double scaled = 0;
    int8_t scalar = 0;	
    if(def != 0){ // with unknown vif (def 0) we cant set the scalar
      scalar = vif_defs[def].scalar + vif - vif_defs[def].base + extensionScaler;
    } 
    if(dataCodingType == 3){
      scaled = valueFloat;
      if(vifarray[0] != 0xFF){  
        for (int8_t i=0; i<scalar; i++) scaled *= 10;
        for (int8_t i=scalar; i<0; i++) scaled /= 10;
        scaled = scaled + extensionAdditiveConstant;
      }
    }
    else if(vifarray[0]==0xFF){
      scaled = value;  
    }
    else{
      scaled = value;
      for (int8_t i=0; i<scalar; i++) scaled *= 10;
      for (int8_t i=scalar; i<0; i++) scaled /= 10;
      scaled = scaled + extensionAdditiveConstant;
    }

    // Init object
    JsonObject data = root.add<JsonObject>();
      data["vif"] = String("0x" + String(vif,HEX));
      data["code"] = vif_defs[def].code;
    //data["vifarray1"] = String("0x" + String(vifarray[1],HEX));
    //data["vifarray2"] = String("0x" + String(vifarray[2],HEX));


    if(asciiValue != 1){ //0 = double, 1 = ASCII, 2 = both;
      //data["scalar"] = scalar;
      //data["value_raw"] = value;
      data["value_scaled"] = scaled; 
    }
    if(asciiValue > 0){ //0 = double, 1 = ASCII, 2 = both;
      data["value_string"] = String(valueString); 
    }
    if(ifcustomVIF == true){
		  data["units"] = String(customVIF);
	  }
    else if(getCodeUnits(vif_defs[def].code)!=0){
      data["units"] = String(getCodeUnits(vif_defs[def].code));
    }
    data["name"] = String(getCodeName(vif_defs[def].code)+String(stringNameExtension)+String(stringFunctionField));
    if(subUnit > 0){
      data["subUnit"] = subUnit;
    }
    if(storageNumber > 0){
      data["storage"] = storageNumber;
    }
    if(tariff > 0){
      data["tariff"] = tariff;
    }    
    /* // only for debug
    data["difes"] = difeNumber;
    if(difeNumber > 0){
      data["dife1"] = dife[1];
      data["dife2"] = dife[2];
    }
    */

    if(buffer[index] == 0x0F && index != size){ // If last byte 0x0F --> Start of manufacturer specific data structures to end of user data --> nothing to decode
      break;
	}	

	if(buffer[index] == 0x1F && index != size){ // If last byte 0x1F --> More records follow in next telegram
      data["telegramFollow"] = 1;
      break;
	}	         
  }
  return count;
}

const char * MBusinoLib::getCodeUnits(uint8_t code) {
  switch (code) {

    case MBUS_CODE::ENERGY_WH:
      return "Wh";
    
    case MBUS_CODE::ENERGY_J:
      return "J";

    case MBUS_CODE::VOLUME_M3: 
      return "m³";

    case MBUS_CODE::MASS_KG: 
      return "kg";

    case MBUS_CODE::ON_TIME_S: 
    case MBUS_CODE::OPERATING_TIME_S: 
    case MBUS_CODE::AVG_DURATION_S:
    case MBUS_CODE::ACTUAL_DURATION_S:
      return "s";

    case MBUS_CODE::ON_TIME_MIN: 
    case MBUS_CODE::OPERATING_TIME_MIN: 
    case MBUS_CODE::AVG_DURATION_MIN:
    case MBUS_CODE::ACTUAL_DURATION_MIN:
      return "min";
      
    case MBUS_CODE::ON_TIME_H: 
    case MBUS_CODE::OPERATING_TIME_H: 
    case MBUS_CODE::AVG_DURATION_H:
    case MBUS_CODE::ACTUAL_DURATION_H:
      return "h";
      
    case MBUS_CODE::ON_TIME_DAYS: 
    case MBUS_CODE::OPERATING_TIME_DAYS: 
    case MBUS_CODE::AVG_DURATION_DAYS:
    case MBUS_CODE::ACTUAL_DURATION_DAYS:
      return "d";


      
    case MBUS_CODE::POWER_W:
    case MBUS_CODE::MAX_POWER_W: 
      return "W";
      
    case MBUS_CODE::POWER_J_H: 
      return "J/h";
      
    case MBUS_CODE::VOLUME_FLOW_M3_H: 
      return "m³/h";
      
    case MBUS_CODE::VOLUME_FLOW_M3_MIN:
      return "m³/min";
      
    case MBUS_CODE::VOLUME_FLOW_M3_S: 
      return "m³/s";
      
    case MBUS_CODE::MASS_FLOW_KG_H: 
      return "kg/h";
      
    case MBUS_CODE::FLOW_TEMPERATURE_C: 
    case MBUS_CODE::RETURN_TEMPERATURE_C: 
    case MBUS_CODE::EXTERNAL_TEMPERATURE_C: 
    case MBUS_CODE::TEMPERATURE_LIMIT_C:
      return "°C";

    case MBUS_CODE::TEMPERATURE_DIFF_K: 
      return "K";

    case MBUS_CODE::PRESSURE_BAR: 
      return "bar";

    case MBUS_CODE::TIME_POINT_DATE:
      return "YYYYMMDD";  

    case MBUS_CODE::TIME_POINT_DATETIME:
      return "YYYYMMDDhhmm";  

    case MBUS_CODE::BAUDRATE_BPS:
      return "bit/s";

    case MBUS_CODE::VOLTS: 
      return "V";

    case MBUS_CODE::AMPERES: 
      return "A";
      
    case MBUS_CODE::VOLUME_FT3:
      return "ft³";

    case MBUS_CODE::VOLUME_GAL: 
      return "gal";
      
    case MBUS_CODE::VOLUME_FLOW_GAL_M: 
      return "gal/min";
      
    case MBUS_CODE::VOLUME_FLOW_GAL_H: 
      return "gal/h";
      
    case MBUS_CODE::FLOW_TEMPERATURE_F:
    case MBUS_CODE::RETURN_TEMPERATURE_F:
    case MBUS_CODE::TEMPERATURE_DIFF_F:
    case MBUS_CODE::EXTERNAL_TEMPERATURE_F:
    case MBUS_CODE::TEMPERATURE_LIMIT_F:
      return "°F";

    case MBUS_CODE::STORAGE_INTERVAL_MONTH:  
    case MBUS_CODE::REMAIN_BAT_LIFE_MONTH:        
      return "month"; 

    case MBUS_CODE::RELATIVE_HUMIDITY:
    case MBUS_CODE::BATTERY_PERCENTAGE:   
    case MBUS_CODE::CHAMBER_POLLUTION_LEVEL:
    case MBUS_CODE::MOISTURE_LEVEL_PERCENT:         
      return "%";       

    case MBUS_CODE::REACTIVE_ENERGY:
      return "kvarh";

    case MBUS_CODE::REACTIVE_POWER:
      return "kvar";  

    case MBUS_CODE::APPARENT_POWER:
      return "kVA";     

    case MBUS_CODE::PHASE_VOLT_DEG:
      return "°"; 

    case MBUS_CODE::PHASE_CURR_DEG:
      return "°"; 

    case MBUS_CODE::FREQUENCY:
      return "Hz";  

    case MBUS_CODE::CARBON_DIOXIDE_PPM:
    case MBUS_CODE::CARBON_MONOXIDE_PPM:   
      return "ppm";    

    case MBUS_CODE::VOLATILE_ORG_COMP_ppb:   
      return "ppb";   

    case MBUS_CODE::VOLATILE_ORG_COMP_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_UG_M3:
    case MBUS_CODE::PARTICLES_PM1_UG_M3:
    case MBUS_CODE::PARTICLES_PM2_5_UG_M3:
    case MBUS_CODE::PARTICLES_PM10_UG_M3:
      return "µg/m³";    

    case MBUS_CODE::PARTICLES_UNSPEC_1M3:
    case MBUS_CODE::PARTICLES_PM1_1M3: 
    case MBUS_CODE::PARTICLES_PM2_5_1M3:    
    case MBUS_CODE::PARTICLES_PM10_1M3:    
      return "1/m³";     

    case MBUS_CODE::ILLUMINANCE_LUX:
      return "lx"; 

    case MBUS_CODE::LUMINOUS_IDENSITY_CD:
      return "cd"; 

    case MBUS_CODE::RADIANT_FLUX_DENS:
      return "W/m²"; 

    case MBUS_CODE::WIND_SPEED_M_S:
      return "m/s"; 

    case MBUS_CODE::RAINFALL_L_MM:
      return "l/mm²"; 

    case MBUS_CODE::FORMAZIN_NEPHELOMETER_U:
      return "FNU"; 

    case MBUS_CODE::DECIBEL_A:
      return "dB"; 

    case MBUS_CODE::DISTANCE_MM:
      return "mm";                

    default:
      break; 

  }

  return 0;

}

const char * MBusinoLib::getCodeName(uint8_t code) {
  switch (code) {

    case MBUS_CODE::UNKNOWN_VIF:
      return "unknown_vif";

    case MBUS_CODE::ENERGY_WH:
    case MBUS_CODE::ENERGY_J:
      return "energy";
    
    case MBUS_CODE::VOLUME_M3: 
    case MBUS_CODE::VOLUME_FT3:
    case MBUS_CODE::VOLUME_GAL: 
      return "volume";

    case MBUS_CODE::MASS_KG: 
      return "mass";

    case MBUS_CODE::ON_TIME_S: 
    case MBUS_CODE::ON_TIME_MIN: 
    case MBUS_CODE::ON_TIME_H: 
    case MBUS_CODE::ON_TIME_DAYS: 
      return "on_time";
    
    case MBUS_CODE::OPERATING_TIME_S: 
    case MBUS_CODE::OPERATING_TIME_MIN: 
    case MBUS_CODE::OPERATING_TIME_H: 
    case MBUS_CODE::OPERATING_TIME_DAYS: 
      return "operating_time";
    
    case MBUS_CODE::AVG_DURATION_S:
    case MBUS_CODE::AVG_DURATION_MIN:
    case MBUS_CODE::AVG_DURATION_H:
    case MBUS_CODE::AVG_DURATION_DAYS:
      return "avg_duration";
    
    case MBUS_CODE::ACTUAL_DURATION_S:
    case MBUS_CODE::ACTUAL_DURATION_MIN:
    case MBUS_CODE::ACTUAL_DURATION_H:
    case MBUS_CODE::ACTUAL_DURATION_DAYS:
      return "actual_duration";

    case MBUS_CODE::POWER_W:
    case MBUS_CODE::MAX_POWER_W: 
    case MBUS_CODE::POWER_J_H: 
      return "power";
      
    case MBUS_CODE::VOLUME_FLOW_M3_H: 
    case MBUS_CODE::VOLUME_FLOW_M3_MIN:
    case MBUS_CODE::VOLUME_FLOW_M3_S: 
    case MBUS_CODE::VOLUME_FLOW_GAL_M: 
    case MBUS_CODE::VOLUME_FLOW_GAL_H: 
      return "volume_flow";

    case MBUS_CODE::MASS_FLOW_KG_H: 
      return "mass_flow";

    case MBUS_CODE::FLOW_TEMPERATURE_C: 
    case MBUS_CODE::FLOW_TEMPERATURE_F:
      return "flow_temperature";

    case MBUS_CODE::RETURN_TEMPERATURE_C: 
    case MBUS_CODE::RETURN_TEMPERATURE_F:
      return "return_temperature";

    case MBUS_CODE::EXTERNAL_TEMPERATURE_C: 
    case MBUS_CODE::EXTERNAL_TEMPERATURE_F:
      return "external_temperature";

    case MBUS_CODE::TEMPERATURE_LIMIT_C:
    case MBUS_CODE::TEMPERATURE_LIMIT_F:
      return "temperature_limit";

    case MBUS_CODE::TEMPERATURE_DIFF_K: 
    case MBUS_CODE::TEMPERATURE_DIFF_F:
      return "temperature_diff";

    case MBUS_CODE::PRESSURE_BAR: 
      return "pressure";

    case MBUS_CODE::TIME_POINT_DATE:
    case MBUS_CODE::TIME_POINT_DATETIME:
      return "time_point";

    case MBUS_CODE::BAUDRATE_BPS:
      return "baudrate";

    case MBUS_CODE::VOLTS: 
      return "voltage";

    case MBUS_CODE::AMPERES: 
      return "current";
      
    case MBUS_CODE::FABRICATION_NUMBER: 
      return "fab_number";

    case MBUS_CODE::BUS_ADDRESS: 
      return "bus_address";

    case MBUS_CODE::CREDIT: 
      return "credit";

    case MBUS_CODE::DEBIT: 
      return "debit";

    case MBUS_CODE::ACCESS_NUMBER: 
      return "access_number";

    case MBUS_CODE::MANUFACTURER: 
      return "manufacturer";

    case MBUS_CODE::PARAMETER_SET_ID: 
      return "set_id";

    case MBUS_CODE::MODEL_VERSION: 
      return "model_version";

    case MBUS_CODE::HARDWARE_VERSION: 
      return "hardware_version";

    case MBUS_CODE::FIRMWARE_VERSION: 
      return "firmware_version";

    case MBUS_CODE::SOFTWARE_VERSION: 
      return "software_version"; //neu

    case MBUS_CODE::CUSTOMER_LOCATION: 
      return "customer_location"; //neu      

    case MBUS_CODE::CUSTOMER: 
      return "customer";
  
    case MBUS_CODE::ERROR_FLAGS: 
      return "error_flags";
  
    case MBUS_CODE::ERROR_MASK: 
      return "error_mask";
  
    case MBUS_CODE::DIGITAL_OUTPUT: 
      return "digital_output";
  
    case MBUS_CODE::DIGITAL_INPUT: 
      return "digital_input";
  
    case MBUS_CODE::RESPONSE_DELAY_TIME: 
      return "response_delay";
  
    case MBUS_CODE::RETRY: 
      return "retry";

     case MBUS_CODE::SIZE_OF_STORAGE_BLOCK: 
      return "sizeof_storageblock";

     case MBUS_CODE::STORAGE_INTERVAL_MONTH: 
      return "storage_interval";
 
    case MBUS_CODE::GENERIC: 
      return "generic";
  
    case MBUS_CODE::RESET_COUNTER: 
    case MBUS_CODE::CUMULATION_COUNTER: 
      return "counter";

    case MBUS_CODE::CUSTOMIZED_VIF: 
      return "customized_vif"; 

    case MBUS_CODE::MANUFACTURER_SPECIFIC: 
        return "manufactur_specific";

    case MBUS_CODE::RELATIVE_HUMIDITY:
      return "humidity";

    case MBUS_CODE::REACTIVE_ENERGY:
      return "reactive_energy";   

    case MBUS_CODE::REACTIVE_POWER:
      return "reactive_power";    

    case MBUS_CODE::APPARENT_POWER:
      return "apparent_power"; 

    case MBUS_CODE::PHASE_VOLT_DEG:
      return "phase_deg_voltage"; 

    case MBUS_CODE::PHASE_CURR_DEG:
      return "phase_deg_current";               

    case MBUS_CODE::FREQUENCY:
      return "frequency";  

    case MBUS_CODE::SPECIAL_SUPPLIER_INFO:
      return "special_suppl_info";    

    case MBUS_CODE::CURRENT_SELECTED_APL:
      return "current_selected_appl."; 

    case MBUS_CODE::SUB_DEVICES:
      return "sub_devices"; 

    case MBUS_CODE::REMAIN_BAT_LIFE_MONTH:
      return "remain_bat_life"; 

    case MBUS_CODE::CARBON_DIOXIDE_PPM:
      return "CO²"; 
    
    case MBUS_CODE::CARBON_MONOXIDE_PPM:
      return "CO"; 

    case MBUS_CODE::VOLATILE_ORG_COMP_ppb:
    case MBUS_CODE::VOLATILE_ORG_COMP_UG_M3:
      return "VOC"; 

    case MBUS_CODE::PARTICLES_UNSPEC_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_1M3:
      return "particels_unspecific"; 

    case MBUS_CODE::PARTICLES_PM1_UG_M3:
    case MBUS_CODE::PARTICLES_PM1_1M3:    
      return "particles_PM1"; 

    case MBUS_CODE::PARTICLES_PM2_5_UG_M3:
    case MBUS_CODE::PARTICLES_PM2_5_1M3:    
      return "particles_PM2,5"; 

    case MBUS_CODE::PARTICLES_PM10_UG_M3:
    case MBUS_CODE::PARTICLES_PM10_1M3:    
      return "particles_PM10"; 

    case MBUS_CODE::ILLUMINANCE_LUX:
      return "illuminance"; 

    case MBUS_CODE::LUMINOUS_IDENSITY_CD:
      return "luminus_idensity"; 

    case MBUS_CODE::RADIANT_FLUX_DENS:
      return "radiant_flux_density"; 

    case MBUS_CODE::WIND_SPEED_M_S:
      return "wind_speed"; 

    case MBUS_CODE::RAINFALL_L_MM:
      return "rainfall"; 

    case MBUS_CODE::FORMAZIN_NEPHELOMETER_U:
      return "formazin_nephelometric"; 

    case MBUS_CODE::POTENTIAL_HYDROGEN_PH:
      return "PH"; 

    case MBUS_CODE::DISMOUNTS_COUNTER:
      return "dismounts_counter"; 

    case MBUS_CODE::TEST_BUTTON_COUNTER:
      return "test_button_counter"; 

    case MBUS_CODE::ALARM_COUNTER:
      return "alarm_counter"; 

    case MBUS_CODE::ALARM_MUTE_COUNTER:
      return "alarm_mute_counter"; 

    case MBUS_CODE::OBSTACLE_DETECT_COUNTER:
      return "obstacle_detect_counter"; 

    case MBUS_CODE::SMOKE_ENTRIES_COUNTER:
      return "smoke_entries_counter"; 

    case MBUS_CODE::SMOKE_CHAMBER_DEFECTS:
      return "smoke_chamber_defects"; 

    case MBUS_CODE::SELF_TEST_COUNTER:
      return "self_test_counter"; 

    case MBUS_CODE::SOUNDER_DEFECT_COUNTER:
      return "sounder_defect_counter"; 

    case MBUS_CODE::DECIBEL_A:
      return "decibel_A"; 

    case MBUS_CODE::BATTERY_PERCENTAGE:
      return "battery"; 

    case MBUS_CODE::CHAMBER_POLLUTION_LEVEL:
      return "chamber_pollution_level"; 

    case MBUS_CODE::DISTANCE_MM:
      return "distance"; 

    case MBUS_CODE::MOISTURE_LEVEL_PERCENT:
      return "moisture_level"; 

    case MBUS_CODE::PRESSURE_SENS_STATUS:
      return "pressure_sens_status"; 

    case MBUS_CODE::SMOKE_ALARM_STATUS:
      return "smoke_alarm_status"; 

    case MBUS_CODE::CO_ALARM_STATUS:
      return "CO_alarm_status"; 

    case MBUS_CODE::HEAT_ALARM_STATUS:
      return "heat_alarm_status"; 

    case MBUS_CODE::DOOR_WINDOW_SENS_STATUS:
      return "door_window_sens_status"; 

    default:
        break; 

  }

  return "";

}

const char * MBusinoLib::getDeviceClass(uint8_t code) {
  switch (code) {

    case MBUS_CODE::ENERGY_WH:
    case MBUS_CODE::ENERGY_J: 
      return "energy";
    
    case MBUS_CODE::VOLUME_M3: 
    case MBUS_CODE::VOLUME_FT3:
    case MBUS_CODE::VOLUME_GAL: 
      return "volume";

    case MBUS_CODE::MASS_KG: 
      return "weight";

    case MBUS_CODE::OPERATING_TIME_S: 
    case MBUS_CODE::OPERATING_TIME_MIN: 
    case MBUS_CODE::OPERATING_TIME_H: 
    case MBUS_CODE::OPERATING_TIME_DAYS: 
    case MBUS_CODE::ON_TIME_S: 
    case MBUS_CODE::ON_TIME_MIN: 
    case MBUS_CODE::ON_TIME_H: 
    case MBUS_CODE::ON_TIME_DAYS: 
    case MBUS_CODE::AVG_DURATION_S:
    case MBUS_CODE::AVG_DURATION_MIN:
    case MBUS_CODE::AVG_DURATION_H:
    case MBUS_CODE::AVG_DURATION_DAYS:
    case MBUS_CODE::ACTUAL_DURATION_S:
    case MBUS_CODE::ACTUAL_DURATION_MIN:
    case MBUS_CODE::ACTUAL_DURATION_H:
    case MBUS_CODE::ACTUAL_DURATION_DAYS:
      return "duration";

    case MBUS_CODE::POWER_W:
    case MBUS_CODE::MAX_POWER_W: 
    case MBUS_CODE::POWER_J_H: 
      return "power";
      
    case MBUS_CODE::VOLUME_FLOW_M3_H: 
    case MBUS_CODE::VOLUME_FLOW_M3_MIN:
    case MBUS_CODE::VOLUME_FLOW_M3_S: 
    case MBUS_CODE::VOLUME_FLOW_GAL_M: 
    case MBUS_CODE::VOLUME_FLOW_GAL_H: 
      return "volume_flow_rate";

    case MBUS_CODE::MASS_FLOW_KG_H: // no Unit or device Class in HA defind
      return "";

    case MBUS_CODE::FLOW_TEMPERATURE_C: 
    case MBUS_CODE::FLOW_TEMPERATURE_F:
    case MBUS_CODE::RETURN_TEMPERATURE_C: 
    case MBUS_CODE::RETURN_TEMPERATURE_F:
    case MBUS_CODE::EXTERNAL_TEMPERATURE_C: 
    case MBUS_CODE::EXTERNAL_TEMPERATURE_F:
    case MBUS_CODE::TEMPERATURE_LIMIT_C:
    case MBUS_CODE::TEMPERATURE_LIMIT_F:
    case MBUS_CODE::TEMPERATURE_DIFF_K: 
    case MBUS_CODE::TEMPERATURE_DIFF_F:
      return "temperature";

    case MBUS_CODE::PRESSURE_BAR: 
      return "pressure";

    case MBUS_CODE::TIME_POINT_DATE:
    case MBUS_CODE::TIME_POINT_DATETIME:
      return "";

    case MBUS_CODE::BAUDRATE_BPS:
      return "data_rate";

    case MBUS_CODE::VOLTS: 
      return "voltage";

    case MBUS_CODE::AMPERES: 
      return "current";

    case MBUS_CODE::RELATIVE_HUMIDITY:
      return "humidity";      

    case MBUS_CODE::FREQUENCY:
      return "frequency";    

    case MBUS_CODE::UNKNOWN_VIF:
    case MBUS_CODE::FABRICATION_NUMBER: 
    case MBUS_CODE::BUS_ADDRESS: 
    case MBUS_CODE::CREDIT: 
    case MBUS_CODE::DEBIT: 
    case MBUS_CODE::ACCESS_NUMBER: 
    case MBUS_CODE::MANUFACTURER: 
    case MBUS_CODE::PARAMETER_SET_ID:
    case MBUS_CODE::MODEL_VERSION: 
    case MBUS_CODE::HARDWARE_VERSION: 
    case MBUS_CODE::FIRMWARE_VERSION: 
    case MBUS_CODE::SOFTWARE_VERSION: 
    case MBUS_CODE::CUSTOMER_LOCATION: 
    case MBUS_CODE::CUSTOMER: 
    case MBUS_CODE::ERROR_FLAGS: 
    case MBUS_CODE::ERROR_MASK: 
    case MBUS_CODE::DIGITAL_OUTPUT: 
    case MBUS_CODE::DIGITAL_INPUT: 
    case MBUS_CODE::RESPONSE_DELAY_TIME: 
    case MBUS_CODE::RETRY:   
    case MBUS_CODE::SIZE_OF_STORAGE_BLOCK:  
    case MBUS_CODE::STORAGE_INTERVAL_MONTH: 
    case MBUS_CODE::GENERIC: 
    case MBUS_CODE::RESET_COUNTER: 
    case MBUS_CODE::CUMULATION_COUNTER: 
    case MBUS_CODE::CUSTOMIZED_VIF: 
    case MBUS_CODE::MANUFACTURER_SPECIFIC: 
    case MBUS_CODE::REACTIVE_ENERGY: 
    case MBUS_CODE::REACTIVE_POWER:  
    case MBUS_CODE::APPARENT_POWER: 
    case MBUS_CODE::PHASE_VOLT_DEG:
    case MBUS_CODE::PHASE_CURR_DEG: 
    case MBUS_CODE::SPECIAL_SUPPLIER_INFO:  

// has to be sort
    case MBUS_CODE::CURRENT_SELECTED_APL:
    case MBUS_CODE::SUB_DEVICES:
    case MBUS_CODE::REMAIN_BAT_LIFE_MONTH:
    case MBUS_CODE::CARBON_DIOXIDE_PPM:
    case MBUS_CODE::CARBON_MONOXIDE_PPM:
    case MBUS_CODE::VOLATILE_ORG_COMP_ppb:
    case MBUS_CODE::VOLATILE_ORG_COMP_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_1M3:
    case MBUS_CODE::PARTICLES_PM1_UG_M3:
    case MBUS_CODE::PARTICLES_PM1_1M3:    
    case MBUS_CODE::PARTICLES_PM2_5_UG_M3:
    case MBUS_CODE::PARTICLES_PM2_5_1M3:    
    case MBUS_CODE::PARTICLES_PM10_UG_M3:
    case MBUS_CODE::PARTICLES_PM10_1M3:    
    case MBUS_CODE::ILLUMINANCE_LUX:
    case MBUS_CODE::LUMINOUS_IDENSITY_CD:
    case MBUS_CODE::RADIANT_FLUX_DENS:
    case MBUS_CODE::WIND_SPEED_M_S:
    case MBUS_CODE::RAINFALL_L_MM:
    case MBUS_CODE::FORMAZIN_NEPHELOMETER_U:
    case MBUS_CODE::POTENTIAL_HYDROGEN_PH:
    case MBUS_CODE::DISMOUNTS_COUNTER:
    case MBUS_CODE::TEST_BUTTON_COUNTER:
    case MBUS_CODE::ALARM_COUNTER:
    case MBUS_CODE::ALARM_MUTE_COUNTER:
    case MBUS_CODE::OBSTACLE_DETECT_COUNTER:
    case MBUS_CODE::SMOKE_ENTRIES_COUNTER:
    case MBUS_CODE::SMOKE_CHAMBER_DEFECTS:
    case MBUS_CODE::SELF_TEST_COUNTER:
    case MBUS_CODE::SOUNDER_DEFECT_COUNTER:
    case MBUS_CODE::DECIBEL_A:
    case MBUS_CODE::BATTERY_PERCENTAGE:
    case MBUS_CODE::CHAMBER_POLLUTION_LEVEL:
    case MBUS_CODE::DISTANCE_MM:
    case MBUS_CODE::MOISTURE_LEVEL_PERCENT:
    case MBUS_CODE::PRESSURE_SENS_STATUS:
    case MBUS_CODE::SMOKE_ALARM_STATUS:
    case MBUS_CODE::CO_ALARM_STATUS:
    case MBUS_CODE::HEAT_ALARM_STATUS:
    case MBUS_CODE::DOOR_WINDOW_SENS_STATUS:

      return "";  
    default:
        break;

  }
  return "";
}

const char * MBusinoLib::getStateClass(uint8_t code) {
  switch (code) {

    case MBUS_CODE::AMPERES: 
    case MBUS_CODE::MASS_KG: 
    case MBUS_CODE::POWER_W:
    case MBUS_CODE::MAX_POWER_W: 
    case MBUS_CODE::POWER_J_H: 
    case MBUS_CODE::VOLUME_FLOW_M3_H: 
    case MBUS_CODE::VOLUME_FLOW_M3_MIN:
    case MBUS_CODE::VOLUME_FLOW_M3_S: 
    case MBUS_CODE::VOLUME_FLOW_GAL_M: 
    case MBUS_CODE::VOLUME_FLOW_GAL_H: 
    case MBUS_CODE::MASS_FLOW_KG_H: 
    case MBUS_CODE::FLOW_TEMPERATURE_C: 
    case MBUS_CODE::FLOW_TEMPERATURE_F:
    case MBUS_CODE::RETURN_TEMPERATURE_C: 
    case MBUS_CODE::RETURN_TEMPERATURE_F:
    case MBUS_CODE::EXTERNAL_TEMPERATURE_C: 
    case MBUS_CODE::EXTERNAL_TEMPERATURE_F:
    case MBUS_CODE::TEMPERATURE_LIMIT_C:
    case MBUS_CODE::TEMPERATURE_LIMIT_F:
    case MBUS_CODE::TEMPERATURE_DIFF_K: 
    case MBUS_CODE::TEMPERATURE_DIFF_F:
    case MBUS_CODE::PRESSURE_BAR: 
    case MBUS_CODE::BAUDRATE_BPS:
    case MBUS_CODE::VOLTS: 
    case MBUS_CODE::RELATIVE_HUMIDITY:
    case MBUS_CODE::PHASE_VOLT_DEG:
    case MBUS_CODE::PHASE_CURR_DEG:    
      return "measurement";

    case MBUS_CODE::ENERGY_WH:
    case MBUS_CODE::ENERGY_J:   
    case MBUS_CODE::UNKNOWN_VIF:    
    case MBUS_CODE::VOLUME_M3: 
    case MBUS_CODE::VOLUME_FT3:
    case MBUS_CODE::VOLUME_GAL: 
    case MBUS_CODE::OPERATING_TIME_S: 
    case MBUS_CODE::OPERATING_TIME_MIN: 
    case MBUS_CODE::OPERATING_TIME_H: 
    case MBUS_CODE::OPERATING_TIME_DAYS: 
    case MBUS_CODE::ON_TIME_S: 
    case MBUS_CODE::ON_TIME_MIN: 
    case MBUS_CODE::ON_TIME_H: 
    case MBUS_CODE::ON_TIME_DAYS: 
    case MBUS_CODE::AVG_DURATION_S:
    case MBUS_CODE::AVG_DURATION_MIN:
    case MBUS_CODE::AVG_DURATION_H:
    case MBUS_CODE::AVG_DURATION_DAYS:
    case MBUS_CODE::ACTUAL_DURATION_S:
    case MBUS_CODE::ACTUAL_DURATION_MIN:
    case MBUS_CODE::ACTUAL_DURATION_H:
    case MBUS_CODE::ACTUAL_DURATION_DAYS:
    case MBUS_CODE::TIME_POINT_DATE:
    case MBUS_CODE::TIME_POINT_DATETIME:
    case MBUS_CODE::FABRICATION_NUMBER: 
    case MBUS_CODE::BUS_ADDRESS: 
    case MBUS_CODE::CREDIT: 
    case MBUS_CODE::DEBIT: 
    case MBUS_CODE::ACCESS_NUMBER: 
    case MBUS_CODE::MANUFACTURER:
    case MBUS_CODE::PARAMETER_SET_ID: 
    case MBUS_CODE::MODEL_VERSION: 
    case MBUS_CODE::HARDWARE_VERSION: 
    case MBUS_CODE::FIRMWARE_VERSION: 
    case MBUS_CODE::SOFTWARE_VERSION: 
    case MBUS_CODE::CUSTOMER_LOCATION: 
    case MBUS_CODE::CUSTOMER: 
    case MBUS_CODE::ERROR_FLAGS: 
    case MBUS_CODE::ERROR_MASK: 
    case MBUS_CODE::DIGITAL_OUTPUT: 
    case MBUS_CODE::DIGITAL_INPUT: 
    case MBUS_CODE::RESPONSE_DELAY_TIME: 
    case MBUS_CODE::RETRY: 
    case MBUS_CODE::SIZE_OF_STORAGE_BLOCK: 
    case MBUS_CODE::STORAGE_INTERVAL_MONTH:  
    case MBUS_CODE::GENERIC: 
    case MBUS_CODE::RESET_COUNTER: 
    case MBUS_CODE::CUMULATION_COUNTER: 
    case MBUS_CODE::CUSTOMIZED_VIF:
    case MBUS_CODE::MANUFACTURER_SPECIFIC: 
    case MBUS_CODE::REACTIVE_ENERGY: 
    case MBUS_CODE::REACTIVE_POWER:     
    case MBUS_CODE::FREQUENCY: 
    case MBUS_CODE::APPARENT_POWER:

// has to be sort
    case MBUS_CODE::CURRENT_SELECTED_APL:
    case MBUS_CODE::SUB_DEVICES:
    case MBUS_CODE::REMAIN_BAT_LIFE_MONTH:
    case MBUS_CODE::CARBON_DIOXIDE_PPM:
    case MBUS_CODE::CARBON_MONOXIDE_PPM:
    case MBUS_CODE::VOLATILE_ORG_COMP_ppb:
    case MBUS_CODE::VOLATILE_ORG_COMP_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_UG_M3:
    case MBUS_CODE::PARTICLES_UNSPEC_1M3:
    case MBUS_CODE::PARTICLES_PM1_UG_M3:
    case MBUS_CODE::PARTICLES_PM1_1M3:    
    case MBUS_CODE::PARTICLES_PM2_5_UG_M3:
    case MBUS_CODE::PARTICLES_PM2_5_1M3:    
    case MBUS_CODE::PARTICLES_PM10_UG_M3:
    case MBUS_CODE::PARTICLES_PM10_1M3:    
    case MBUS_CODE::ILLUMINANCE_LUX:
    case MBUS_CODE::LUMINOUS_IDENSITY_CD:
    case MBUS_CODE::RADIANT_FLUX_DENS:
    case MBUS_CODE::WIND_SPEED_M_S:
    case MBUS_CODE::RAINFALL_L_MM:
    case MBUS_CODE::FORMAZIN_NEPHELOMETER_U:
    case MBUS_CODE::POTENTIAL_HYDROGEN_PH:
    case MBUS_CODE::DISMOUNTS_COUNTER:
    case MBUS_CODE::TEST_BUTTON_COUNTER:
    case MBUS_CODE::ALARM_COUNTER:
    case MBUS_CODE::ALARM_MUTE_COUNTER:
    case MBUS_CODE::OBSTACLE_DETECT_COUNTER:
    case MBUS_CODE::SMOKE_ENTRIES_COUNTER:
    case MBUS_CODE::SMOKE_CHAMBER_DEFECTS:
    case MBUS_CODE::SELF_TEST_COUNTER:
    case MBUS_CODE::SOUNDER_DEFECT_COUNTER:
    case MBUS_CODE::DECIBEL_A:
    case MBUS_CODE::BATTERY_PERCENTAGE:
    case MBUS_CODE::CHAMBER_POLLUTION_LEVEL:
    case MBUS_CODE::DISTANCE_MM:
    case MBUS_CODE::MOISTURE_LEVEL_PERCENT:
    case MBUS_CODE::PRESSURE_SENS_STATUS:
    case MBUS_CODE::SMOKE_ALARM_STATUS:
    case MBUS_CODE::CO_ALARM_STATUS:
    case MBUS_CODE::HEAT_ALARM_STATUS:
    case MBUS_CODE::DOOR_WINDOW_SENS_STATUS:  
      return "total";
    default:
        break;
  }
  return "";
}

// ----------------------------------------------------------------------------

int16_t MBusinoLib::_findDefinition(uint32_t vif) {
  
  for (uint8_t i=0; i<MBUS_VIF_DEF_NUM; i++) {
    vif_def_type vif_def = vif_defs[i];
    if ((vif_def.base <= vif) && (vif < (vif_def.base + vif_def.size))) {
      return i;
    }
  }
  
  return -1;

}

uint32_t MBusinoLib::_getVIF(uint8_t code, int8_t scalar) {

  for (uint8_t i=0; i<MBUS_VIF_DEF_NUM; i++) {
    vif_def_type vif_def = vif_defs[i];
    if (code == vif_def.code) {
      if ((vif_def.scalar <= scalar) && (scalar < (vif_def.scalar + vif_def.size))) {
        return vif_def.base + (scalar - vif_def.scalar);
      }
    }
  }
  
  return 0xFF; // this is not a valid VIF

}
