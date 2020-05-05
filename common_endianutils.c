#include "common_endianutils.h"


int _is_littlendian() {
	int value = 1;
	char *c = (char*) &value;
	if (*c)
		return 1;
	return 0;
}

uint32_t _change_endianness(uint32_t value){
	uint32_t endianness = 0;
	uint8_t* temp = (uint8_t*) &endianness;
	uint8_t* value_c = (uint8_t*) &value;
	for (int i = 0; i < sizeof(uint32_t); i++) {
		temp[i] = value_c[sizeof(uint32_t)-1-i];
	}
	return endianness;
}

uint32_t _to_littlendian(uint32_t number){
	uint32_t endianness;
	if (!_is_littlendian())
		endianness = _change_endianness(number);
	else 
		endianness =  number;
	return endianness;
}

int _uint8_t_to_uint32(uint8_t* value, int pos) {
	uint8_t temp[4];
	uint32_t* temp32;
	if (!_is_littlendian()){
		for (int j = 3; j >= 0; j--) {
			temp[3-j] = value[pos+j];
		}
	} else {
		for (int j = 0; j < 4; j++) {
			temp[j] = value[pos+j];
		}
	}
	temp32 = (uint32_t*) temp;		
	return *temp32;
}
