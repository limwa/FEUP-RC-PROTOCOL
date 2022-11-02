// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "packet.h"

void application_layer_tx(const char* filename) {
    unsigned char packet[MAX_PAYLOAD_SIZE];

    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat(filename, &st) < 0) {
        perror("stat");
        exit(1);
    }
    
    printf("  - File name: %s\n", filename);


    ControlPacket ctrl = {
        .type = P_START,
        .filesize = st.st_size
    };

    #ifdef TESTING
        #if TESTING_USE_FILENAME == 0
        strcpy((char*) ctrl.filename, "penguin-received.gif");
        #else
        strcpy(ctrl.filename, filename);
        #endif
    #else
    strcpy(ctrl.filename, filename);
    #endif

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    
    // TODO ensure file name fits in MAX_PAYLOAD_SIZE
    unsigned int ctrl_size = packet_create_control(packet, ctrl);

    if (llwrite(packet, ctrl_size) < 0) {
        perror("couldn't write packet");
        exit(1);
    }

    DataPacket data;
    unsigned char serial_number = 0;

    short bytes_read;
    while ((bytes_read = read(fd, &data.payload, sizeof(data.payload) / sizeof(*data.payload))) > 0) {
        data.seq_nr = serial_number;
        data.payload_size = bytes_read;

        serial_number = (serial_number + 1) % (UCHAR_MAX + 1);
        
        unsigned int packet_size = packet_create_data(packet, data);
        if (llwrite(packet, packet_size) < 0) {
            perror("couldn't write packet");
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("read");
    }

    ctrl.type = P_END;
    ctrl_size = packet_create_control(packet, ctrl);

    if (llwrite(packet, ctrl_size) < 0) {
        perror("couldn't write packet");
        exit(1);
    }

    printf("File information:\n Name: %s\n Size: %d\n", ctrl.filename, ctrl.filesize); 
}

void application_layer_rx() {
    unsigned char packet[MAX_PAYLOAD_SIZE];
    int ctrl_packet_size = llread(packet);
    
    if (ctrl_packet_size < 0) {
        perror("couldn't read packet");
        exit(1);
    }
    
    ControlPacket ctrl;
    if (packet_read_control(&ctrl, packet) < 0) {
        printf("application_layer_rx: could not read control packet\n");
        exit(1);
    }
    
    int fd = open((char*) ctrl.filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    printf("  - File name: %s\n", ctrl.filename);

    DataPacket data;
    long bytes_read = 0;
    unsigned char curr_seq_nr = 0;

    while (bytes_read < ctrl.filesize) {
        if (llread(packet) < 0) {
            perror("couldn't read packet");
            exit(1);
        }

        if (packet_read_data(&data, packet) < 0) {
            printf("application_layer_rx: could not read data packet\n");
            exit(1);
        }

        if (data.seq_nr != curr_seq_nr) {
            perror("couldn't read next packet");
            exit(1);
        }

        if (write(fd, &data.payload, data.payload_size) < data.payload_size) {
            printf("application_layer_rx: could not write to file\n");
            exit(1);
        }
        
        curr_seq_nr = (curr_seq_nr + 1) % (UCHAR_MAX + 1);
        bytes_read += data.payload_size;
    }

    if (llread(packet) < 0) {
        perror("couldn't read packet");
        exit(1);
    }

    if (packet_read_control(&ctrl, packet) < 0) {
        printf("application_layer_rx: could not read end control packet\n");
        exit(1);
    }

    if (ctrl.type != P_END) {
        printf("application_layer_rx: expected end control packet\n");
        exit(1);
    }

    close(fd);
}

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

    if (llclose(TRUE) < 0) {
        exit(1);
    }

    close(fd);

    return;
}
