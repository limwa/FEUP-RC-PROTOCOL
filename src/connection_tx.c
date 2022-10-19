#include "connection_tx.h"
#include "constants.h"
#include "state_ua.h"

static int sequence_nr = 0;

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

int conn_tx_send_information(unsigned char *data, unsigned int size) {
    unsigned char frame[MAX_FRAME_SIZE] = { FLAG, A_TX_CMD, C_I(6), A_TX_CMD ^ C_I(6), data, check_bcc2(data, size), FLAG };
    return conn_send(frame, size + 6, TRUE);
}

int conn_tx_disconnect() {
    unsigned char frame[5] = { FLAG, A_TX_CMD, C_DISC, A_TX_CMD ^ C_DISC, FLAG };
    return conn_send(frame, sizeof(frame), TRUE);
}
