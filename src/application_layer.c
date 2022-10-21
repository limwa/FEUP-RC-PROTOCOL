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
        const char *message[] = { "olá, this is a message by baby to baby\n", "i hab someting bery important to tell you\n", "nestum is so mfing gud\n", "but u are better :pleading_face: :drooling_face:\n", "i wub u\n" };
        for (unsigned int i = 0; i < sizeof(message) / sizeof(*message); i++) {
            int bytes_written = llwrite(message[i], strlen(message[i]) + 1);
            printf("bytes written: %d\n", bytes_written);
            if (bytes_written == 0) {
                i--;
                continue;
            }
            printf("sent: %s\n", message[i]);
        }
    } else {
        unsigned char packet[MAX_PAYLOAD_SIZE];
        for (unsigned int i = 0; i < 5; i++) {
            int bytes_read = llread(packet);
            printf("bytes read: %d\n", bytes_read);
            if (bytes_read == 0) {
                i--;
                continue;
            }
            printf("packet: %s\n", packet);
        }
    }


    return;
}