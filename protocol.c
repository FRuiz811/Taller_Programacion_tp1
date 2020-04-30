#include "protocol.h"
#include <string.h>
#include "buffer_dinamico.h"

int protocol_create(protocol_t* self) {
	self->buffer = buffer_create(0);
	self->id = -1;
	if (self->buffer == NULL)
		return -1;
	return 0;
}

static int protocol_restart(protocol_t* self) {
	buffer_destroy(self->buffer);
	self->buffer = buffer_create(0);
	if (self->buffer == NULL)
		return -1;
	return 0;
}

uint32_t protocol_id_message(protocol_t* self) {
	static uint32_t id = 0;
	id += 1	;
	self->id = id;
	return id;
}

//Esta función permite saber si se está trabajando en un sistema 
//con littlendian (Devolverá 1), o bigendian (devolvera 0)
static int _is_littlendian() {
	int value = 1;
	char *c = (char*) &value;
	if (*c)
		return 1;
	return 0;
}

//En caso de que se trabaje en big endian se cambia a littlendian
//devolviendo como un char* los bytes que representan a value en
//este nuevo endianness
static char* _change_endianness(uint32_t value){
	char *temp;
	char *endianness;
	temp = (char*) &value;
	int j = 0;
	for (int i = sizeof(uint32_t) -1 ; i >= 0; i--) {
		endianness[j] = temp[i];
		j++;
	}
	return endianness;
}

static char* _encode_message(char* message, uint32_t length, char data_type, uint8_t parameter_type) {
	char* endianness, *message_encoded;
	if (!_is_littlendian())  
		endianness = _change_endianness(length);
	else
		endianness = (char*) &length;
	uint32_t encode_length = length + sizeof(uint32_t) + 2 + 1 + 1;
	char encode[encode_length];
	encode[0] = parameter_type;
	encode[1] = 0x01;
	encode[2] = data_type;
	memset(&encode[3], 0, 1);
	memcpy(&encode[4], endianness, sizeof(uint32_t));
	message[length] = '\0';
	memcpy(&encode[8], message, length +1);
	message_encoded = encode;
	return message_encoded;
}	


static void _destiny_encode(protocol_t* self, char* message, uint32_t length) {
	//char *message_encoded = _encode_message(message, length, 's', 0x06);
	//size_t new_length = length + sizeof(uint32_t) + 2 + 1 + 1;
	char* endianness;
	if (!_is_littlendian())  
		endianness = _change_endianness(length);
	else
		endianness = (char*) &length;
	uint32_t encode_length = length + sizeof(uint32_t) + 2 + 1 + 1 + 1;
	char encode[encode_length];
	encode[0] = 0x06;
	encode[1] = 0x01;
	encode[2] = 's';
	memset(&encode[3], 0, 1);
	memcpy(&encode[4], endianness, sizeof(uint32_t));
	message[length] = '\0';
	memcpy(&encode[8], message, length+1);
	buffer_concatenate(self->buffer, encode, encode_length);
	return;
}

static void _path_encode(char* encode, char* message, uint32_t length) {
	uint8_t parameter_type = 0x01;
	char data_type = 'o';
	encode = _encode_message(message,length,'o', 0x01);
	return;
}

static void _interface_encode(char* encode, char* message, uint32_t length) {
	uint8_t parameter_type = 0x02;
	char data_type = 's';
	encode = _encode_message(message,length,'s',0x02);
	return;
}

static void _method_encode(char* encode, char* message, uint32_t length) {
	uint8_t parameter_type = 0x03;
	char data_type = 's';
	char* limit_method = strchr(message, '(');
	if (limit_method != NULL) {

	}
	encode = _encode_message(message,length,'s',0x03);
	return;
}

static void _build_header(protocol_t* self){
	char* endianness, header[16];
	header[0] = 'l';
	header[1] = 0x01;
	header[2] = 0x00;
	header[3] = 0x01;
	memset(&header[4], 0, 4);
	uint32_t id = protocol_id_message(self);	
	if (!_is_littlendian())  
		endianness = _change_endianness(self->id);
	else
		endianness = (char*) &self->id;
	memcpy(&header[8],endianness,4);
	memset(&header[12], 0, 4);
	buffer_concatenate(self->buffer, header, 16);
	return;	
}

void protocol_encode_message(protocol_t* self, char* message, size_t length) {
	protocol_restart(self);
	char *message_splited = strtok(message, " ");
	int argument = 0;
	char *destiny, *path, *interface, *method;
	_build_header(self);
	while (message_splited != NULL) {
		argument += 1;
		switch (argument) {
			case 1:
				_destiny_encode(self, message_splited, strlen(message_splited));
			/*case 2:
				destiny =_path_encode(path, message_splited, strlen(message_splited));
			case 3:
				_interface_encode(interface, message_splited, strlen(message_splited));
			case 4:
				_method_encode(method, message_splited, strlen(message_splited));*/
		}
		message_splited = strtok(NULL, " ");
	}
	return;
}

void protocol_destory(protocol_t* self) {
	self->id = -1;
	buffer_destroy(self->buffer);
	return;
}