#include "request.h"
#include <string.h>

Request *init_Request(uint8_t cmd_len, uint16_t args_len) {
	Request *req = malloc(sizeof(Request));
	req->command = malloc(sizeof(char)*(cmd_len + 1));
	req->args = malloc(sizeof(char)*(args_len + 1));
	return req;	
}

void free_Request(Request *req) {
	free(req->command);
	if(req->args != NULL)
		free(req->args);
	free(req);
}

Request *get_request(char *msg) {
	if(msg[0] == '\0')
        return NULL;
	
	uint8_t i = 0;
	for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
	uint8_t command_start = i;

	for(; !isspace(msg[i]) && msg[i] != '\0'; i++);
	uint8_t command_len = i - command_start + 1;
	if(msg[i] != '\0') {
		msg[i++] = '\0';
	}

	for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
	uint8_t args_start = i;

	for(; msg[i] != '\r' && msg[i] != '\n' && msg[i] != '\0'; i++);
	uint8_t args_len = i - args_start + 1;
	msg[i] = '\0';

	Request *req = init_Request(command_len, args_len);
	strcpy(req->command, &msg[command_start]);
	strcpy(req->args, &msg[args_start]);

	return req;
}