#ifndef FTP_SERVER_H_
#define FTP_SERVER_H_

#include "api.h"
#include "lwip/netif.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#define COMMAND_PORT 8000
#define DATA_PORT 7999
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

extern struct netif gnetif;

typedef struct {
    struct netconn *conn;
    TaskHandle_t *taskHandle;
} Client;

struct netconn *command_conn;
struct netconn *data_conn;

void ftp_start();
void disconnect_client();

#endif /* FTP_SERVER_H_ */
