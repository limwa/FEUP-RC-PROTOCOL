#pragma once

typedef struct {
    int fd;
    int timeout;
    int tries;
} ConnectionInfo;

int conn_setup(ConnectionInfo connectionParameters);

void conn_timeout(int signal);
int conn_is_stale();
void conn_reset();

int conn_send(unsigned char *buffer, int size, int is_command);
int conn_read(unsigned char *buffer, int size);
