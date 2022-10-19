// Link layer protocol implementation

#include "link_layer.h"
#include "connection_rx.h"
#include "connection_tx.h"
#include "string.h"
#include "fcntl.h"
#include "termios.h"
#include "stdio.h"
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
    newterm.c_cc[VTIME] = 0; // Inter-character timer unused
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

    ConnectionInfo connInfo = {
        .fd = fd,
        .timeout = connectionParameters.timeout,
        .tries = connectionParameters.nRetransmissions
    };

    if (conn_setup(connInfo) < 0) {
        return -1;
    }

    if (connectionParameters.role == LlTx) {
        if (conn_tx_handshake() < 0) {
            return -1;
        }
    } else if (connectionParameters.role == LlRx) {
        if (conn_rx_handshake() < 0) {
            return -1;
        }
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    conn_send(buf, bufSize, TRUE);
    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    state_machine machines[2] = { state_machine_set, state_machine_i };
    conn_read_frame(machines, )
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {
    conn_disconnect();
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

