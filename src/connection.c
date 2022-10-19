#include "connection.h"
#include "link_layer.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "signal.h"
#include "constants.h"
#include "state.h"

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

// unsigned int conn_disconnect() {
//     conn_send()
// }

unsigned int conn_read_frame(state_machine* machines, int size) {
    for (int i = 0; i < size; i++) {
        machines[i].clear();
    }

    unsigned char buf;
    unsigned int machine_idx = 0;

    int has_read = FALSE;
    while (!has_read) {
        int bytes_read = conn_read(&buf, 1);
        if (bytes_read < 0)
            return -1;

        if (bytes_read == 0) {
            continue;
        }

        for (machine_idx = 0; machine_idx < size; machine_idx++) {
            machines[machine_idx].read(buf);
            if (machines[machine_idx].is_frame()) {
                has_read = TRUE;
                break;
            }
        }
    }

    return machine_idx;
}

int byte_stuffing(unsigned char *src, unsigned int size, unsigned char *dest) {
    unsigned int new_size = 0;

    for (int i = 0, j = 0; i < size; i++, j++) {
        if (src[i] == FLAG) {
            dest[j] = ESC; 
            j++;
            dest[j] = ESC_FLAG;
        } else if (src[i] == ESC) {
            dest[j] = ESC;
            j++;
            dest[j] = ESC_ESC;
        } else {
            dest[j] = src[i];
        }

        new_size = j;
    }

    return new_size;
}

int byte_destuffing(unsigned char *src, unsigned int size, unsigned char *dest) {
    unsigned int new_size = 0;
    
    for (int i = 0, j = 0; i < size; i++, j++) {
        if (src[i] == ESC) {
            i++;
            if (src[i] == ESC_FLAG) {
                dest[j] = FLAG;
            } else if (src[i] == ESC_ESC) {
                dest[j] = ESC;
            }
        } else {
            dest[j] = src[i];
        }

        new_size = j;
    }

    return new_size;
}

int check_bcc2(unsigned char* data, unsigned int size) {
    int bcc2 = data[0];
    for (int i = 1; i < size; i++) {
        bcc2 ^= data[i];
    }

    return bcc2;
}