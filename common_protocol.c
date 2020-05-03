#include "common_protocol.h"
#include <string.h>
#include "common_dynamic_buffer.h"
#include "common_endianutils.h"

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
	id += 1;
	self->id = id;
	return id;
}

static int _aligment(protocol_t* self) {
	buffer_t* aux = self->header;
	int mod = aux->length % 8;
	if (mod != 0) {
		int padding = 8 - mod; 
		uint8_t zeros[8];
		memset(&zeros, 0, padding);
		return buffer_concatenate(self->header, zeros, padding);
	}
	return 0;
}

static void _split_method_parameters(char* string_to_split, uint32_t length,
																		 char** method, char** parameters) {
	char* limit_method = strchr(string_to_split, '(');
	if (limit_method == NULL){
		*parameters = strchr(string_to_split, '\0');
		*method = string_to_split;
		return;
	}
	*limit_method++ = '\0';
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

static int _encoding_header(protocol_t* self, char* message, uint32_t length,
														char data_type, uint8_t parameter_type) {
	uint32_t encoding_length = length + sizeof(uint32_t) + 2 + 1 + 1;
	char* encoding = malloc(sizeof(char) * (encoding_length+1));
	if (encoding == NULL)
		return -1;
	memset(encoding, 0, sizeof(char) * (encoding_length+1));
	uint8_t* endianness;
	_to_littlendian(&endianness, length);
	encoding[0] = parameter_type;
	encoding[1] = 0x01;
	encoding[2] = data_type;
	memset(&encoding[3], 0, 1);
	memcpy(&encoding[4], endianness, sizeof(uint32_t));
	memcpy(&encoding[8], message, length);
	int result = buffer_concatenate(self->header, encoding, encoding_length+1);
	free(encoding);
	return result;
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

static int _interface_encode(protocol_t* self, char* interface, 
														 uint32_t length) {
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

static int _parameters_encode(protocol_t* self, char* parameters,
															uint32_t length) {
	if (length == 0)
		return 0;
	uint8_t parameter_type = 0x09;
	char data_type = 'g';
	length -= 1;
	char* encoding = malloc(sizeof(char) * length + 1);
	if(encoding == NULL)
		return -1;
	memset(encoding,0,sizeof(char) * length + 1);
	int j = 1;
	for (int i = 0; i < length; i++) {
		if (parameters[i] == ',') {
			j++;
		}
	}
	memset(encoding,'s',j);
	int result =_encoding_header(self,encoding,j,data_type,parameter_type);
	free(encoding);
	return result;
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
	uint32_t encoding_length = length + sizeof(uint32_t);
	char* encoding =  malloc(sizeof(char) * (encoding_length+1));
	if (encoding == NULL)
		return -1;
	memset(encoding, 0, sizeof(char) * (encoding_length+1));
	uint8_t* endianness;
	_to_littlendian(&endianness,length);
	memcpy(&encoding[0], endianness, sizeof(uint32_t));
	memcpy(&encoding[4], message, length);
	int result = buffer_concatenate(self->body, encoding, encoding_length+1);
	free(encoding);
	return result;
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
				error =_destiny_encode(self, message_splited[i],
															 strlen(message_splited[i]));
				break;
			 case 1 :
				error = _path_encode(self, message_splited[i],
														 strlen(message_splited[i]));
				break;
			case 2 :
				error = _interface_encode(self, message_splited[i],
																  strlen(message_splited[i]));	
				break;
			case 3 :
				_split_method_parameters(message_splited[i],
													strlen(message_splited[i]), &method, &parameters);
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
		if (i == 3)
			message_splited[i] = strtok(NULL,")");
		else
			message_splited[i] = strtok(NULL," ");
	}
	return 0;
}

void protocol_get_info_message(protocol_t* self,uint8_t* message,
															 size_t length, uint32_t info_message[]) {
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


void protocol_decode_header(protocol_t* self, uint8_t message[],
														size_t length, char* info_message[]) {
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

void protocol_decode_body(protocol_t* self, uint8_t message[],
													size_t length, char* info_message[]) {
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
