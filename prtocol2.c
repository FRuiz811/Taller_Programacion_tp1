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

static int _aligment(protocol_t* self) {
	buffer_t* aux = self->buffer;
	int mod = aux->length % 8;
	if (mod != 0) {
		int padding = 8 - mod; 
		uint8_t zeros[padding];
		memset(&zeros, 0, padding);
		return buffer_concatenate(self->buffer, zeros, padding);
	}
	return 0;
}

static int _set_array_lenght(protocol_t* self) {
	size_t array_length;
	char* endianness;
	char* data = buffer_get_data(self->buffer);
	array_length = buffer_get_length(self->buffer) - 16;
	if (!_is_littlendian())  
		endianness = _change_endianness(array_length);
	else
		endianness = (char*) &array_length;
	memcpy(&(data[12]),endianness,4);
	return 0;

}

static int _set_body_lenght(protocol_t* self, uint32_t body_length) {
	char* endianness;
	char* data = buffer_get_data(self->buffer);
	if (!_is_littlendian())  
		endianness = _change_endianness(body_length);
	else
		endianness = (char*) &body_length;
	memcpy(&(data[4]),endianness,4);
	return 0;

}



static int _encoding_header(protocol_t* self, char* message, uint32_t length, char data_type, uint8_t parameter_type) {
	char* endianness;
	if (!_is_littlendian())  
		endianness = _change_endianness(length);
	else
		endianness = (char*) &length;
	uint32_t encoding_length = length + 1 + sizeof(uint32_t) + 2 + 1 + 1;
	char encoding[encoding_length];
	memset(&encoding, 0, encoding_length);
	encoding[0] = parameter_type;
	encoding[1] = 0x01;
	encoding[2] = data_type;
	memset(&encoding[3], 0, 1);
	memcpy(&encoding[4], endianness, sizeof(uint32_t));
	message[length] = '\0';
	memcpy(&encoding[8], message, length+1);
	return buffer_concatenate(self->buffer, encoding, encoding_length);
}

static int _destiny_encode(protocol_t* self, char* destiny, uint32_t length) {
	uint8_t parameter_type = 0x06;
	char data_type = 's';
	int result =_encoding_header(self,destiny,length,data_type,parameter_type);
	return result;
}


static int _path_encode(protocol_t* self, char* path, uint32_t length) {
	uint8_t parameter_type = 0x01;
	char data_type = 'o';
	int result =_encoding_header(self,path,length,data_type,parameter_type);
	return result;
}

static int _interface_encode(protocol_t* self, char* interface, uint32_t length) {
	uint8_t parameter_type = 0x02;
	char data_type = 's';
	int result =_encoding_header(self,interface,length,data_type,parameter_type);
	return result;
}

static int _parameters_encode(protocol_t* self, char* parameters, uint32_t length) {
	uint8_t parameter_type = 0x09;
	char data_type = 'g';
	parameters[length-1] = '\0';
	length -= 1;
	char encoding[length];
	encoding[0] = 's';
	int j = 1;
	for (int i = 0; i < length; i++) {
		if (parameters[i] == ',') {
			encoding[j] = 's';
			j++;
		}
	}
	encoding[j] = '\n';
	int result =_encoding_header(self,encoding,j,data_type,parameter_type);
	return 0;
}

static int _method_encode(protocol_t* self, char* method, uint32_t length) {
	uint8_t parameter_type = 0x03;
	char data_type = 's';
	int method_length = length;
	char* parameters;	
	char* limit_method = strchr(method, '(');
	if (limit_method != NULL) {
		method_length = limit_method - method;
		parameters = &method[method_length+1];
	}
	int result = _encoding_header(self,method,method_length,data_type,parameter_type);
	if (result == -1)
		return result;
	if (limit_method != NULL){
		_aligment(self);
		result = _parameters_encode(self,parameters, strlen(parameters));
		_set_array_lenght(self);
		_aligment(self);
	//	_build_body(self, parameters, strlen(parameters));

	}
	return result;
}

static int _build_header(protocol_t* self){
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
	return buffer_concatenate(self->buffer, header, 16);	
}

int protocol_encode_message(protocol_t* self, char* message, size_t length) {
	protocol_restart(self);
	char *message_splited[4];
	message_splited[0] = strtok(message, " ");
	int i = 0;
	int error = _build_header(self);
	if(error != 0)
		return -1;
	while (message_splited[i] != NULL) {
		switch (i) {
			case 0 :
				error =_destiny_encode(self, message_splited[i], strlen(message_splited[i]));
				break;
			 case 1 :
				error = _path_encode(self, message_splited[i], strlen(message_splited[i]));
				break;
			case 2 :
				error = _interface_encode(self, message_splited[i], strlen(message_splited[i]));
				break;
			case 3 :
				error = _method_encode(self, message_splited[i], strlen(message_splited[i]));
				break;
		}
		_aligment(self);
		i++;
		if (error != 0)
			return -1;
		message_splited[i] = strtok(NULL," ");
	}
	return 0;
}

void protocol_destory(protocol_t* self) {
	self->id = -1;
	buffer_destroy(self->buffer);
	return;
}