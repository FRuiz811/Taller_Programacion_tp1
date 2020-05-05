#ifndef COMMON_ENDIANUTILS_H
#define COMMON_ENDIANUTILS_H
#include <stdint.h>

//Esta función permite saber si se está trabajando en un sistema 
//con littlendian (Devolverá 1), o bigendian (devolvera 0)
int _is_littlendian();

//En caso de que se trabaje en big endian se cambia a littlendian
//devolviendo como un uint32_t con los bytes de value invertidos.
uint32_t _change_endianness(uint32_t value);

//Transforma el parámetro number a littleendian y lo almacena
//en el puntero endianness.
uint32_t _to_littlendian(uint32_t number);

//Transforma el valor que se encuentra en la posición pos más 
//las 3 posiciones siguiente del array de uint8_t y lo transforma
//en un int que es devuelto
int _uint8_t_to_uint32(uint8_t* value, int pos); 

#endif
