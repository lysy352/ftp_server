#include "request.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*Request *init_Request(uint8_t cmd_len, uint16_t args_len) {
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
}*/

Request get_request(char *msg) {
    Request req;
    req.command[0] = '\0';
    req.args[0] = '\0';

    if(msg[0] == '\0')
        return req;

    uint8_t i = 0;
    for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
    uint8_t command_start = i;

    for(; !isspace(msg[i]) && msg[i] != '\0'; i++);
    if(msg[i] != '\0')
        msg[i++] = '\0';

    for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
    uint8_t args_start = i;

    for(; msg[i] != '\r' && msg[i] != '\n' && msg[i] != '\0'; i++);
    msg[i] = '\0';

    strcpy(req.command, &msg[command_start]);

    strcpy(req.args, &msg[args_start]);

    return req;
}

uint8_t update_request(Request *req, char *msg) {
	if(strlen(msg) > COMMAND_SIZE + ARGS_SIZE)
        return 0;

	uint8_t i = 0;
    for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
    uint8_t command_start = i;

    for(; !isspace(msg[i]) && msg[i] != '\0'; i++);
    if(msg[i] != '\0')
        msg[i++] = '\0';

    for(; isspace(msg[i]) && msg[i] != '\0' ; i++);
    uint8_t args_start = i;

    for(; msg[i] != '\r' && msg[i] != '\n' && msg[i] != '\0'; i++);
    msg[i] = '\0';

    strcpy(req->command, &msg[command_start]);

    strcpy(req->args, &msg[args_start]);

    return 1;
}