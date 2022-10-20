// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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
        unsigned char message[12] = "Hello World";
        llwrite(message, 12);
        printf("Sent: Hello World");
    } else {
        unsigned char packet[MAX_PAYLOAD_SIZE];
        llread(packet);
        printf("packet: %s\n", packet);
    }


    return;
}