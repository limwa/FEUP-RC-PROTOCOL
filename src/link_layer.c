// Link layer protocol implementation

#include "link_layer.h"

#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "constants.h"
#include "frame.h"
#include "protocol.h"
#include "state.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

static int fd; // File descriptor for the serial port
static struct termios oldterm; // Old serial port configuration

int restore_serial() {
    if (tcflush(fd, TCIOFLUSH) < 0) {
        perror("tcflush");
        return -1;
    }

    if (tcsetattr(fd, TCSANOW, &oldterm) < 0) {
        perror("tcsetattr");
        return -1;
    }

    memset(&oldterm, 0, sizeof(oldterm));
    return 1;
}

int config_serial(int baudRate) {
    // Save current port settings
    if (tcgetattr(fd, &oldterm) < 0) {
        perror("tcgetattr");
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) < 0) {
        perror("tcflush");
        return -1;
    }

    struct termios newterm;
    memset(&newterm, 0, sizeof(newterm));

    newterm.c_cflag = baudRate | CS8 | CLOCAL | CREAD;
    newterm.c_iflag = IGNPAR;
    newterm.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newterm.c_lflag = 0;
    newterm.c_cc[VTIME] = 10 * SERIAL_TIMEOUT; // Inter-character timer unused
    newterm.c_cc[VMIN] = 0;  // Blocking read until 5 chars received
    
    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newterm) < 0) {
        perror("tcsetattr");
        return -1;
    }

    return 1;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (config_serial(connectionParameters.baudRate) < 0) {
        return -1;
    }

    ProtocolOptions protocol_options = {
        .fd = fd,
        .timeout = connectionParameters.timeout,
        .tries = connectionParameters.nRetransmissions,
        .role = connectionParameters.role
    };

    protocol_setup(protocol_options);
    frame_set_role(connectionParameters.role);

    printf("connect: %d\n", protocol_connect());
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    int bytes = protocol_information_send(buf, bufSize);
    printf("write: %d\n", bytes);
    return bytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    int bytes = protocol_information_read(packet, MAX_PAYLOAD_SIZE);
    printf("read: %d\n", bytes);
    return bytes;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {
    // conn_disconnect();
    return 1;
}



// int llwrite(const unsigned char *buf, int bufSize) {
//     if (state != FD_OPEN) {
//         perror("Serial port connection is not open");
//         return -1;
//     }

//     int bytes = write(fd, buf, sizeof(buf) / sizeof(char));
    
//     if (bytes <= 0) {
//         return -1;
//     }
//     return bytes;
// }


// int llopen(LinkLayer connectionParameters) {
//     if (state != CLOSED) return -1;

//     fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
//     if (fd < 0) {
//         LLOPEN_ERROR("open");
//         return -1;
//     }

//     state = FD_OPEN;

//     // Save current port settings
//     if (tcgetattr(fd, &oldterm) < 0) {
//         LLOPEN_ERROR("tcgetattr");
//         return -1;
//     }

//     state = OLDTERM_SET;

//     if (tcflush(fd, TCIOFLUSH) < 0) {
//         LLOPEN_ERROR("tcflush");
//         return -1;
//     }

//     state = IO_FLUSHED;

//     struct termios newterm;
//     memset(&newterm, 0, sizeof(newterm));

//     newterm.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
//     newterm.c_iflag = IGNPAR;
//     newterm.c_oflag = 0;

//     // Set input mode (non-canonical, no echo,...)
//     newterm.c_lflag = 0;
//     newterm.c_cc[VTIME] = 0; // Inter-character timer unused
//     newterm.c_cc[VMIN] = 5;  // Blocking read until 5 chars received
    
//     // Set new port settings
//     if (tcsetattr(fd, TCSANOW, &newterm) < 0) {
//         LLOPEN_ERROR("tcsetattr");
//         return -1;
//     }
    
//     state = OPEN;
//     return TRUE;
// }


// int llread(unsigned char *packet) {
//     read(fd, &packet, 1);
    
//     return 0;
// }

// int llclose(int showStatistics) {
//     switch (state) {
//         case OPEN:            
//             tcflush(fd, TCIOFLUSH);

//         case IO_FLUSHED:
//         case OLDTERM_SET:
//             tcsetattr(fd, TCSANOW, &oldterm);
//             memset(&oldterm, 0, sizeof(oldterm));

//         case FD_OPEN:
//             close(fd);
//             break;

//         default:
//             return FALSE;
//     }

//     state = CLOSED;
//     return TRUE;
// }

