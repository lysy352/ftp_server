#include "client_thread.h"
#include "request.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "memory_access.h"
#include "ftp_server.h"

#define MAX_MSG_LEN 256

int8_t send_data(void *data, size_t data_size, struct netconn *conn) {
    if(netconn_write(conn, data, data_size, NETCONN_NOCOPY) != ERR_OK) {
    	printf("netconn_write error\r\n");
    	return 0;
    }
    return 1;
}

int8_t recv_data(void *data, size_t *data_size, struct netconn *conn) {
	struct netbuf *buf;

	if(netconn_recv(conn, &buf) != ERR_OK) {
    	printf("netconn_recv error\r\n");
    	return 0;
    }    

    char *data_recvd;

    if(netbuf_data(buf, &data_recvd, data_size) != ERR_OK) {
    	printf("netbuf_data error\r\n");
    	return 0;
    }

    data = malloc(sizeof(char) * (*data_size + 1));
    for(uint16_t i = 0; i < *data_size; i++)
    	((char *)data)[i] = data_recvd[i];
    ((char *)data)[*data_size] = '\0';
    netbuf_delete(buf);
    return 1;
}

int8_t send_msg(char *status, char *msg, struct netconn *conn) {	
    size_t respond_size = sizeof(char)*(strlen(status) + 1 + strlen(msg) + 3);
    char *respond = malloc(respond_size);
    sprintf(respond, "%s %s\r\n", status, msg);
    printf("RESPOND: %s", respond);

    send_data(respond, respond_size, conn);
   
    free(respond);
    return 1;    
}

Request *read_msg(struct netconn *conn) {
	char *msg;
	size_t msg_size;

	recv_data(msg, &msg_size, conn);

    printf("REQUEST: %s\n", msg);
    Request *req = get_request(msg);
    free(msg);
    return req;
}

void type_response(char *args, ClientData *client_data) {
    if(strcmp(args, "I") == 0)
        send_msg("200", "Binary mode accepted", client_data->conn);
    else if(strcmp(args, "A") == 0)
        send_msg("200", "ASCII mode accepted", client_data->conn);
    else
        send_msg("502", "Command not implemented", client_data->conn);
}

void pasv_response(ClientData *client_data) {
    char *msg = malloc(sizeof(char)*50);
    sprintf(msg, "Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
            ip4_addr1(&gnetif.ip_addr),
			ip4_addr2(&gnetif.ip_addr),
			ip4_addr3(&gnetif.ip_addr),
			ip4_addr4(&gnetif.ip_addr),
            DATA_PORT/256,
            DATA_PORT % 256);  

    send_msg("227", msg, client_data->conn);
    free(msg);
}

void list_response(ClientData *client_data) {
    send_msg("150", "Directory listing", client_data->conn);

    struct netconn *client_data_conn;
	if(netconn_accept(data_conn, &client_data_conn) != ERR_OK) {
        printf("netconn_accept error\r\n");
        return;
    } 

    char *list_data = list_directory(client_data->current_dir);
    send_data(list_data, sizeof(char)*strlen(list_data), client_data_conn);
    printf("LIST DATA: %s\r\n", list_data);
    free(list_data);
    send_msg("226", "List OK", client_data->conn);

    netconn_close(client_data_conn);
    netconn_delete(client_data_conn);
}

void cwd_response(char *args, ClientData *client_data) {
    char *new_path = change_directory(client_data->current_dir, args);
    if(new_path == NULL) {
        if(!create_dir(client_data->current_dir, args)) {
        	printf("create_dir error\r\n");
            return;
        }

        new_path = change_directory(client_data->current_dir, args);
    }
    free(client_data->current_dir);

    client_data->current_dir = new_path;
    send_msg("250", "OK.", client_data->conn);
}

int8_t send_file(char *current_path, char *filename, struct netconn *conn) {
    FIL *file = open_file(current_path, filename);
    if(file == NULL) {
        printf("open_file error\r\n");
        return 0;
    }

    uint16_t buf_size = sizeof(char)*BUFFER_SIZE; 
    char *buf = malloc(buf_size);
    
    uint16_t br;
    while(1) {
		if(f_read(file, buf, buf_size, &br) != FR_OK) {
			printf("f_read error\r\n");
			close_file(file);
			free(buf);
		}		
        send_data(buf, br, conn);

        if(br < buf_size) 
        	break;
    }

	close_file(file);
	free(buf);
    return 1;
}


int8_t recv_file(char *current_path, char *filename, struct netconn *conn) {
    FIL *file = create_file(current_path, filename);
    if(file == NULL) {
        printf("create_file error\r\n");
        return 0;
    }

    struct netbuf *buf;
    void *data;
    uint16_t data_size;
    uint16_t bw;

    while(netconn_recv(conn, &buf) == ERR_OK) {
        do {
            netbuf_data(buf, &data, &data_size);
            if(f_write(file, data, data_size, &bw) != FR_OK) {
            	printf("f_write error\r\n");
            }
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
    }
    close_file(file);
    return 1;
}

void retr_response(char *args, ClientData *client_data) {
    send_msg("150", "File transfer", client_data->conn);

    struct netconn *client_data_conn;
	if(netconn_accept(data_conn, &client_data_conn) != ERR_OK) {
        printf("netconn_accept error\r\n");
        return;
    } 

    send_file(client_data->current_dir, args, client_data_conn);

    send_msg("226", "Transfer completed", client_data->conn);
 
    netconn_close(client_data_conn);
    netconn_delete(client_data_conn);
}

void stor_response(char *args, ClientData *client_data) {
    send_msg("150", "File transfer", client_data->conn);

    struct netconn *client_data_conn;
	if(netconn_accept(data_conn, &client_data_conn) != ERR_OK) {
        printf("netconn_accept error\r\n");
        return;
    } 

    recv_file(client_data->current_dir, args, client_data_conn);

    send_msg("226", "Transfer completed", client_data->conn);

 	netconn_close(client_data_conn);
    netconn_delete(client_data_conn);
}

void mkd_response(char *args, ClientData *client_data) {
    if(!create_dir(client_data->current_dir, args)) {
        printf("create_dir error\r\n");
        return;
    }
    send_msg("257", "Directory created", client_data->conn);
}

void free_resources(ClientData *client_data) {
    netconn_close(client_data->conn);
    netconn_delete(client_data->conn);
    free(client_data->current_dir);
    free(client_data);    
}

void quit_response(ClientData *client_data) {
    send_msg("221", "Quit", client_data->conn);
}

void serve_request(Request *req, ClientData *client_data) {
    struct netconn *client_conn = client_data->conn;
    char *command = req->command;

    if (strcmp(command, "USER") == 0) {
        send_msg("230", "Success", client_conn);
    } else if (strcmp(command, "SYST") == 0) {
        send_msg("215", "EMBOS", client_conn);
    } else if (strcmp(command, "PWD") == 0) {
        send_msg("257", client_data->current_dir, client_conn);
    } else if ((strcmp(command, "TYPE") == 0)) {
        type_response(req->args, client_data);
    } else if (strcmp(command, "PASV") == 0) {
        pasv_response(client_data);
    } else if (strcmp(command, "LIST") == 0) {
        list_response(client_data);
    } else if (strcmp(command, "CWD") == 0) {
        cwd_response(req->args, client_data);
    } else if (strcmp(command, "RETR") == 0) {
        retr_response(req->args, client_data);
    } else if (strcmp(command, "STOR") == 0) {
        stor_response(req->args, client_data);
    } else if (strcmp(command, "MKD") == 0) {
        mkd_response(req->args, client_data);
    } else if(strcmp(command, "QUIT") == 0) {
        quit_response(client_data);
    } else {
        send_msg("502", "Command not implemented", client_conn);
    }
}

void serve_client(void *client_conn) {
    ClientData *client_data = malloc(sizeof(ClientData));
    client_data->conn = (struct netconn *)client_conn;
    client_data->current_dir = malloc(sizeof(char)*2);
    strcpy(client_data->current_dir, "/");

    send_msg("220",  "Welcome", client_data->conn);

    Request *req;
    while(1) {
        req = read_msg(client_data->conn);
        if(req == NULL) {
            free_resources(client_data); 
            printf("NULL message\r\n");     
            return;      
        }
        serve_request(req, client_data);
        free(req);
    }    
}

void serve_client_task(void *arg) {
    while(1) {
        ClientData *client_data = malloc(sizeof(ClientData));
        
        if(!xQueueReceive(clients_queue, &client_data->conn, portMAX_DELAY)) {
            printf("xQueueReceive error, serve_client_task\r\n");
        }
        client_data->current_dir = malloc(sizeof(char)*2);
        strcpy(client_data->current_dir, "/");

        send_msg("220",  "Welcome", client_data->conn);

        Request *req;
        while(1) {
            req = read_msg(client_data->conn);
            if(req == NULL) {
                free_resources(client_data); 
                printf("NULL message\r\n");     
                return;      
            }
            serve_request(req, client_data);
            free(req);
        }    
    }

}
