// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename) {
    LinkLayerRole llrole; 
    if (strcmp(role, "tx") == 0) {
        llrole = LlTx;
    } else if (strcmp(role, "rx") == 0) {
        llrole = LlRx;
    } else {
        printf("Invalid role: %s", role);
    }

    LinkLayer connectionParameters = {
        .role = llrole,
        .baudRate = baudRate,
        .nRetransmissions = nTries,
        .timeout = timeout
    };

    memcpy(connectionParameters.serialPort, serialPort, 50);

    int fd = llopen(connectionParameters);
    if (fd < 0) {
        exit(1);
    }

    if (llrole == LlTx) {
        application_layer_tx(filename);
    } else {
        application_layer_rx();
    }

    return;
}

void application_layer_tx(const char* filename) {
    unsigned char serial_number = 0;

    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat(filename, &st) < 0) {
        perror("stat");
        exit(1);
    }

    int fd = open(filename, "r");
    if (fd < 0) {
        perror("fopen");
        exit(1);
    }
    
    // TODO ensure file name fits in MAX_PAYLOAD_SIZE
    unsigned char ctrl_packet[MAX_PAYLOAD_SIZE];
    
    long file_size = st.st_size;
    ctrl_packet[0] = 2;
    ctrl_packet[1] = 0;
    ctrl_packet[2] = sizeof(file_size);
    memcpy(ctrl_packet + 3, &file_size, sizeof(file_size));
    unsigned int curr = 3 + sizeof(file_size);
    ctrl_packet[curr] = 1;
    ctrl_packet[++curr] = strlen(filename) + 1;
    memcpy(&ctrl_packet[++curr], filename, strlen(filename) + 1);
    
    if (llwrite(ctrl_packet, curr + strlen(filename) + 1) < 0) {
        exit(1);
    }

    unsigned char data_packet[MAX_PAYLOAD_SIZE];
    data_packet[0] = 0x01; // Data
    
    short bytes_read;
    while ((bytes_read = read(data_packet + 4, MAX_PAYLOAD_SIZE - 4, fd)) > 0) {
        data_packet[1] = serial_number;
        memcpy(data_packet + 2, &bytes_read, sizeof(bytes_read));

        
        serial_number = (++serial_number % 256);
        if (llwrite(data_packet, 4 + bytes_read) < 0) {
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("read");
    }

    ctrl_packet[0] = 3;
    if (llwrite(ctrl_packet, curr + strlen(filename) + 1) < 0) {
        exit(1);
    }

    close(fd);
}

void application_layer_rx() {

}