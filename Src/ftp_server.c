#include "ftp_server.h"
#include "memory_access.h"
#include "client_thread.h" 
#include "lwip.h"
#include "lwip/timeouts.h"

#define CLIENT_TASK_STACK_SIZE 1000

Client *clients[MAX_CLIENTS];
uint8_t clients_number;

/*struct ip_addr_t get_ip_addr(char *ip_text) {
	uint8_t ip_array[4];
	uint8_t x = 0;
    char *start = ip_text;
	for(char *k = ip_text; *k != '\0'; k++) {
		if(*k == '.') {
            *k = '\0';
            ip_array[x++] = atoi(start);
            start = k+1;
        }
	}
	ip_array[3] = atoi(start);

	struct ip_addr_t ip;
	IP4_ADDR(&ip, ip_array[0], ip_array[1], ip_array[2], ip_array[3]);
}*/

struct netconn *open_conn(uint16_t port) {
	struct netconn *conn;
	while((conn = netconn_new(NETCONN_TCP)) == NULL) {
		printf("netconn_new error, trying again\r\n");
		vTaskDelay(1000);
	}

    while(netconn_bind(conn, &gnetif.ip_addr, port) != ERR_OK) {
        printf("netconn_bind error, trying again\r\n");
        vTaskDelay(1000);
    }

    while(netconn_listen(conn) != ERR_OK) {
        printf("netconn_listen error, trying again\r\n");
        vTaskDelay(1000);
    }

    return conn;
}

void accept_client() {
    Client *client = malloc(sizeof(Client));

    if(netconn_accept(command_conn, &client->conn) != ERR_OK) {
        printf("netconn_accept error\r\n");
        free(client);
        return;
    }
    printf("client accepted\n");

    serve_client(client->conn);

    /*BaseType_t res = xTaskCreate(
    	serve_client,
    	"CLIENT_TASK", 
    	CLIENT_TASK_STACK_SIZE, 
    	client->conn,
    	16,
    	client->taskHandle);

    if(res != pdPASS) {
    	printf("xTaskCreate error\r\n");
    	netconn_close(client->conn);
    	free(client);
    }  */

    clients_number++;
    printf("Client connected, current number of clients: %d\r\n", clients_number);
    for(uint8_t i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == NULL) {
            clients[i] = client;
            break;
        }
    }
}

void disconnect_client() {
    clients_number--;
    printf("Client disconnected, current number of clients: %d\r\n", clients_number);
}

void ftp_start() {
	for(uint8_t i = 0; i < MAX_CLIENTS; i++)
        clients[i] = NULL;    

	printf("Server started. Waiting for address...\r\n");
	while(gnetif.ip_addr.addr == 0) {
        /*printf("IP: %u.%u.%u.%u\r\nPORT: %d\r\n",
                ip4_addr1(&gnetif.ip_addr),
                ip4_addr2(&gnetif.ip_addr),
                ip4_addr3(&gnetif.ip_addr),
                ip4_addr4(&gnetif.ip_addr),
                COMMAND_PORT);*/
		vTaskDelay(1000);
	} 

	printf("IP: %u.%u.%u.%u\r\nPORT: %d\r\n",
				ip4_addr1(&gnetif.ip_addr),
				ip4_addr2(&gnetif.ip_addr),
				ip4_addr3(&gnetif.ip_addr),
				ip4_addr4(&gnetif.ip_addr),
				COMMAND_PORT);

	command_conn = open_conn(COMMAND_PORT);
    data_conn = open_conn(DATA_PORT);

    while(1) {
    	if(clients_number < MAX_CLIENTS)
            accept_client();
        else
			vTaskDelay(1000);        
    }
}