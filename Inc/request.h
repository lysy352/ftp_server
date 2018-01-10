#ifndef REQUEST_H_
#define REQUEST_H_

#include "stdint.h"

typedef struct {
	char *command;
	char *args;
} Request;


Request *init_Request(uint8_t cmd_len, uint16_t args_len);

void free_Request(Request *req);

Request *get_request(char *msg);

#endif /* REQUEST_H_ */
