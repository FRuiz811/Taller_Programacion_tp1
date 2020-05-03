#include "common_endianutils.h"

int _is_littlendian() {
	int value = 1;
	char *c = (char*) &value;
	if (*c)
		return 1;
	return 0;
}

uint8_t* _change_endianness(uint32_t value){
	uint8_t *temp;
	uint8_t *endianness = {0};
	temp = (uint8_t*) &value;
	int j = 0;
	for (int i = sizeof(uint32_t) -1 ; i >= 0; i--) {
		endianness[j] = temp[i];
		j++;
	}
	return endianness;
}

void _to_littlendian(uint8_t** endianness, uint32_t number){
	if (!_is_littlendian())
		*endianness = _change_endianness(number);
	else 
		*endianness = (uint8_t*) &number;
	return;
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
