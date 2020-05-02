#include "protocol.h"
#include <string.h>
#include "buffer_dinamico.h"

int protocol_create(protocol_t* self) {
	self->header = buffer_create(0);
	if (self->header == NULL)
		return -1;
	self->body = buffer_create(0);
	if (self->body == NULL)
		return -1;
	self->id = -1;
	return 0;
}

static int protocol_restart(protocol_t* self) {
	buffer_destroy(self->header);
	self->header = buffer_create(0);
	if (self->header == NULL)
		return -1;
	buffer_destroy(self->body);
	self->body = buffer_create(0);
	if (self->body == NULL)
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
static uint8_t* _change_endianness(uint32_t value){
	uint8_t *temp;
	uint8_t *endianness;
	temp = (uint8_t*) &value;
	int j = 0;
	for (int i = sizeof(uint32_t) -1 ; i >= 0; i--) {
		endianness[j] = temp[i];
		j++;
	}
	return endianness;
}

static void _to_littlendian(uint8_t** endianness, uint32_t number){
	if (!_is_littlendian())
		*endianness = _change_endianness(number);
	else 
		*endianness = (uint8_t*) &number;
	return;
}

static int _aligment(protocol_t* self) {
	buffer_t* aux = self->header;
	int mod = aux->length % 8;
	if (mod != 0) {
		int padding = 8 - mod; 
		uint8_t zeros[padding];
		memset(&zeros, 0, padding);
		return buffer_concatenate(self->header, zeros, padding);
	}
	return 0;
}

static void _split_method_parameters(char* string_to_split, uint32_t length, char** method, char** parameters) {
	char* limit_method = strchr(string_to_split, '(');
	if (limit_method == NULL){
		*parameters = strchr(string_to_split, '\0');
		*method = string_to_split;
		return;
	}
	*limit_method = '\0';
	*limit_method++;
	*parameters = limit_method;
	string_to_split[length] = '\0';
	*method = string_to_split;
	return;
}

static int _set_array_lenght(protocol_t* self) {
	uint8_t* endianness;
	char* data = buffer_get_data(self->header);
	uint32_t array_length = buffer_get_length(self->header) - 16;
	_to_littlendian(&endianness,array_length);
	memcpy(&data[12],endianness,4);
	return 0;

}

static int _set_body_lenght(protocol_t* self, uint32_t body_length) {
	uint8_t* endianness;
	char* data = (buffer_get_data(self->header));
	_to_littlendian(&endianness, body_length);
	memcpy(&data[4],endianness,4);
	return 0;

}

static int _encoding_header(protocol_t* self, char* message, uint32_t length, char data_type, uint8_t parameter_type) {
	uint8_t* endianness;
	_to_littlendian(&endianness, length);
	uint32_t encoding_length = length + 1 + sizeof(uint32_t) + 2 + 1 + 1;
	char encoding[encoding_length];
	encoding[0] = parameter_type;
	encoding[1] = 0x01;
	encoding[2] = data_type;
	memset(&encoding[3], 0, 1);
	memcpy(&encoding[4], endianness, sizeof(uint32_t));
	message[length] = '\0';
	memcpy(&encoding[8], message, length+1);
	return buffer_concatenate(self->header, encoding, encoding_length);
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

static int _method_encode(protocol_t* self, char* method, uint32_t length) {
	uint8_t parameter_type = 0x03;
	char data_type = 's';
	int result = _encoding_header(self,method,length,data_type,parameter_type);
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
	encoding[j] = '\0';
	int result =_encoding_header(self,encoding,j,data_type,parameter_type);
	return 0;
}

static int _build_header(protocol_t* self){
	uint8_t* endianness;
	char header[16];
	header[0] = 'l';
	header[1] = 0x01;
	header[2] = 0x00;
	header[3] = 0x01;
	memset(&header[4], 0, 4);
	uint32_t id = protocol_id_message(self);	
	_to_littlendian(&endianness,id);
	memcpy(&header[8],endianness,4);
	memset(&header[12], 0, 4);
	return buffer_concatenate(self->header, header, 16);	
}


static int _encoding_body(protocol_t* self, char* message, uint32_t length) {
	uint8_t* endianness;
	_to_littlendian(&endianness,length);
	uint32_t encoding_length = length + 1 + sizeof(uint32_t);
	char encoding[encoding_length];
	memcpy(&encoding[0], endianness, sizeof(uint32_t));
	message[length]='\0';
	memcpy(&encoding[4], message, length+1);
	return buffer_concatenate(self->body, encoding, encoding_length);
}



static int _build_body(protocol_t* self, char* body, uint32_t length) {
	char* actual_string = body;
	char* end_char = strchr(actual_string,',');
	uint32_t actual_length;
	uint32_t total = 0;
	while (end_char != NULL) {
		actual_length = end_char - actual_string;
		actual_string[actual_length] ='\0';	
		total += actual_length + 1 + sizeof(uint32_t);
		_encoding_body(self, actual_string, actual_length);
		actual_string = (end_char + sizeof(char));
		end_char = strchr(actual_string, ',');
	}
	if (end_char == NULL) {
		end_char = strchr(actual_string, '\0');
		actual_length = end_char - actual_string;
		actual_string[actual_length] ='\0';	
		total += actual_length + 1 + sizeof(uint32_t);
		_encoding_body(self, actual_string, actual_length);
	}
	_set_body_lenght(self, total);
	return 0;
}


int protocol_encode_message(protocol_t* self, char* message, size_t length) {
	protocol_restart(self);
	char *message_splited[4];
	message_splited[0] = strtok(message, " ");
	int i = 0;
	char *method, *parameters = NULL;
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
				_split_method_parameters(message_splited[i],strlen(message_splited[i]), &method, &parameters);
				error = _method_encode(self, method, strlen(method));
				if (*parameters != '\0') {
					_aligment(self);
					error = _parameters_encode(self, parameters, strlen(parameters));
				}
				_set_array_lenght(self);
				break;
		}
		_aligment(self);
		if (parameters !=  NULL && *parameters != '\0') 
			_build_body(self, parameters, strlen(parameters));
		i++;
		if (error != 0)
			return -1;
		message_splited[i] = strtok(NULL," ");
	}
	return 0;
}

static int _uint8_t_to_uint32(uint8_t* message, int pos) {
	uint8_t temp[4];
	uint32_t* temp32;
	if (!_is_littlendian()){
		for (int j = 3; j >= 0; j--) {
			temp[3-j] = message[pos+j];
		}
	} else {
		for (int j = 0; j < 4; j++) {
			temp[j] = message[pos+j];
		}
	}
	temp32 = (uint32_t*) temp;		
	return *temp32;
}


void protocol_get_info_message(protocol_t* self,uint8_t* message, size_t length, uint32_t info_message[]) {
	int i = 0;
	while (i <= length) {
		if (i == 4)
		 	info_message[0] = _uint8_t_to_uint32(message, i);
		else if (i == 8)
			info_message[1] = _uint8_t_to_uint32(message, i);
		else if (i == 12)
		 	info_message[2] = _uint8_t_to_uint32(message, i);
		i++;
	}
	return;
}


void protocol_decode_header(protocol_t* self, uint8_t message[], size_t length, char* info_message[]) {
	int i = 0;
	int length_string;
	while(i < length){
		if (message[i] == 0) {
			i++;
			continue;
		}
		switch (message[i]) {
			case 1: //decode path
				info_message[1] = (char*)&message[i+8];
				break;
			case 2: //decode interface
				info_message[2] = (char*)&message[i+8];
				break;
			case 3: //decode method
				info_message[3] = (char*)&message[i+8];
				break; 
			case 6: //decode destiny
				info_message[0] = (char*)&message[i+8];
				break;
			case 9: //decode firm
				info_message[4] = (char*)&message[i+8];
				break;
			default:
				i++;
				continue;
		}
		length_string = _uint8_t_to_uint32(message, i+4);
		i += 8 + length_string;
	}
}

void protocol_decode_body(protocol_t* self, uint8_t message[], size_t length, char* info_message[]) {
	int i = 0;
	int j = 0;
	int length_string;
	while (i < length) {
		length_string = _uint8_t_to_uint32(message, i);
		info_message[j] = (char*)&message[i+4];
		i += 4 + length_string + 1;
		j++;
	}
	return;
}

buffer_t* protocol_header_message(protocol_t* self) {
	return self->header;
}

buffer_t* protocol_body_message(protocol_t* self){
	return self->body;
}


void protocol_destroy(protocol_t* self) {
	self->id = -1;
	buffer_destroy(self->header);
	buffer_destroy(self->body);
	return;
}