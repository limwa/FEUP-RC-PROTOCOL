// Link layer protocol implementation

#include "link_layer.h"
#include "connection_rx.h"
#include "connection_tx.h"
#include "string.h"
#include "fcntl.h"
#include "termios.h"
#include "stdio.h"

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
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {
    // TODO

    return 1;
}
