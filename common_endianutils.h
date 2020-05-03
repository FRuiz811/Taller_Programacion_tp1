#ifndef COMMON_ENDIANUTILS_H
#define COMMON_ENDIANUTILS_H
#include <stdint.h>

//Esta función permite saber si se está trabajando en un sistema 
//con littlendian (Devolverá 1), o bigendian (devolvera 0)
int _is_littlendian();

//En caso de que se trabaje en big endian se cambia a littlendian
//devolviendo como un char* los bytes que representan a value en
//este nuevo endianness
uint8_t* _change_endianness(uint32_t value);

void _to_littlendian(uint8_t** endianness, uint32_t number);

int _uint8_t_to_uint32(uint8_t* value, int pos); 

#endif
