#include "connection.h"
#include "link_layer.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "signal.h"

static int fd, timeout, tries;
static int current_try = 0;

static unsigned char frame_buffer[MAX_FRAME_SIZE];
static int frame_size;

int conn_setup(ConnectionInfo connectionParameters) {
    signal(SIGALRM, conn_timeout);

    fd = connectionParameters.fd;
    timeout = connectionParameters.timeout;
    tries = connectionParameters.tries;

    return 0;
}

void conn_timeout(int signal) {
    current_try++;
    if (current_try > tries) {
        return;
    }

    printf("sending: ");
    for (int i = 0; i < frame_size; i++) {
        printf("%02x ", frame_buffer[i]);
    }
    printf("\n");
    fflush(stdout);
    if (write(fd, frame_buffer, frame_size) < 0) {
        perror("conn_retry => write");
    }

    alarm(timeout);
}

int conn_is_stale() {
    return current_try > tries;
}

void conn_reset() {
    printf("resetting...");

    alarm(0);

    memset(frame_buffer, 0, MAX_FRAME_SIZE);
    current_try = 0;
    frame_size = 0;
}

int conn_send(unsigned char *buffer, int size, int is_command) {
    memcpy(frame_buffer, buffer, size);
    frame_size = size;

    printf("sending: ");
    for (int i = 0; i < size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
    fflush(stdout);

    if (write(fd, buffer, size) < 0) {
        conn_reset();

        perror("conn_send => write");
        return -1;
    }

    if (is_command) {
        alarm(timeout);
    }

    return 1;
}

int conn_read(unsigned char *buffer, int size) {
    
    int bytes = read(fd, buffer, size);
    printf("receiving: ");
    for (int i = 0; i < bytes; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
    fflush(stdout);
    sleep(1);
    return bytes;
}
