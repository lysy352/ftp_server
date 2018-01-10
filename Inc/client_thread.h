#ifndef FTPSERVER_CLIENT_THREAD_H
#define FTPSERVER_CLIENT_THREAD_H

typedef struct {
	struct netconn *conn;
    char *current_dir;
} ClientData;

void serve_client(void *client_conn);
//void send_msg(char *status, char *msg, int sock_fd);
//Request *read_msg(int sock_fd);

#endif //FTPSERVER_CLIENT_THREAD_H
