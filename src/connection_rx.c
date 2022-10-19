#include "connection_rx.h"
#include "state_set.h"
#include "constants.h"

int conn_rx_handshake() {
    if (conn_rx_read_set() < 0) {
        return -1;
    }

    int success = conn_rx_send_ua();
    conn_reset();

    return success;
}

int conn_rx_information(unsigned char *data, int length) {

}

int conn_rx_read_set() {
    state_clear_set();

    unsigned char byte;
    int bytes_read;

    while (!conn_is_stale() && !state_is_set() && (bytes_read = conn_read(&byte, 1)) >= 0) {
        if (bytes_read > 0) state_read_set(byte);
    }

    return 1;
}

int conn_rx_send_ua() {
    unsigned char frame[5] = { FLAG, A_RX_RES, C_UA, A_RX_RES ^ C_UA, FLAG };
    return conn_send(frame, sizeof(frame), FALSE);
}

int conn_rx_send_rr(int ready_nr) {
    unsigned char frame[5] = { FLAG, A_RX_RES, C_RR(ready_nr), A_RX_RES ^ C_RR(ready_nr), FLAG };
    return conn_send(frame, sizeof(frame), FALSE);
}

int conn_rx_disconnect() {
    unsigned char frame[5] = { FLAG, A_TX_CMD, C_DISC, A_TX_CMD ^ C_DISC, FLAG };
    return conn_send(frame, sizeof(frame), FALSE);
}
