#include "connection_tx.h"
#include "constants.h"
#include "state_ua.h"

int sequencial = 0;
char sequencial_byte = BIT(sequencial);

int conn_tx_handshake() {
    if (conn_tx_send_set() < 0) {
        return -1;
    }

    int success = conn_tx_read_ua();
    conn_reset();

    return success;
}

int conn_tx_send_set() {
    unsigned char frame[5] = { FLAG, A_TX_CMD, C_SET, A_TX_CMD ^ C_SET, FLAG };
    return conn_send(frame, sizeof(frame), TRUE);
}

int conn_tx_read_ua() {
    state_clear_ua();

    unsigned char byte;
    int bytes_read;

    while (!conn_is_stale() && !state_is_ua() && (bytes_read = conn_read(&byte, 1)) >= 0) {
        if (bytes_read > 0) state_read_ua(byte);
    }

    return 1;
}

int check_bcc2(unsigned char* data, unsigned int size) {
    int bcc2 = data[0];
    for (int i = 1; i < size; i++) {
        bcc2 ^= data[i];
    }

    return bcc2;
}

int conn_tx_send_information(unsigned char *data, unsigned int size) {
    unsigned char frame[MAX_FRAME_SIZE] = { FLAG, A_TX_CMD, sequencial_byte, A_TX_CMD ^ sequencial_byte, data, check_bcc2(data, size), FLAG };
    return conn_send(frame, size + 6, TRUE);
}

int byte_stuffing(unsigned char* data, unsigned int size) {
    unsigned char stuffed_data[MAX_FRAME_SIZE];
    unsigned int new_size = 0;

    for (int i = 0, j = 0; i < size; i++, j++) {
        if (data[i] == 0x7E) {
            stuffed_data[j] = 0x7D; 
            j++;
            stuffed_data[j] = 0x5E;
        } else if (data[i] == 0x7D) {
            stuffed_data[j] = 0x7D;
            j++;
            stuffed_data[j] = 0x5D;
        } else {
            stuffed_data[j] = data[i];
        }

        new_size = j;
    }

    return new_size;
}

int byte_destuffing(unsigned char* data, unsigned int size) {
    unsigned int destuffed_data[size];
    unsigned int new_size = 0;
    
    for (int i = 0, j = 0; i < size; i++, j++) {
        if (data[i] == 0x7D) {
            i++;
            destuffed_data[j] = (data[i] || 0x20);
        } else {
            destuffed_data[j] = data[i];
        }

        new_size = j;
    }

    return new_size;
}
