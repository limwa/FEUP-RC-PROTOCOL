#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static ProtocolOptions options;
static LastFrame last_frame;

void protocol_setup(ProtocolOptions new_options) {
    signal(SIGALRM, protocol_handle_timeout);
    options = new_options;
}

int protocol_send_frame(const unsigned char *frame, unsigned int size, int retry_on_timeout) {
    printf("Write!\n");
    fflush(stdout);
    if (write(options.fd, frame, size) < size) {
        printf("protocol_send_frame: could not send frame\n");
        return -1;
    }

    printf("Wrote: ");
    for (int i = 0; i < frame; i++) {
        printf("%x (%c) / ", frame[i], frame[i]);
    }

    printf("\n");
    fflush(stdout);

    if (retry_on_timeout) {
        memcpy(last_frame.bytes, frame, size);
        last_frame.size = size;
        last_frame.tries_left = options.tries;

        alarm(options.timeout);
    }

    return 1;
}

int protocol_read_frame(StateMachine *machines, unsigned int size, int reset_timeout_on_success) {
    for (unsigned int machine_idx = 0; machine_idx < size; machine_idx++) {
        machines[machine_idx].clear();
    }

    printf("Read!\n");
    printf("Reading: ");
    unsigned char buf;
    unsigned int machine_idx = -1;
    while (machine_idx == -1) {
        int bytes_read = read(options.fd, &buf, 1);
        if (bytes_read <= 0) {
            break;
        }

        printf("%x (%c)  / ", buf, buf);
        fflush(stdout);

        for (unsigned int i = 0; i < size; i++) {
            machines[i].read(buf);
            if (machines[i].is_frame()) {
                machine_idx = i;
                break;
            }
        }
    }

    printf("\n");
    printf("Accepted on machine: %d\n", machine_idx);
    fflush(stdout);

    if (reset_timeout_on_success) {
        protocol_reset_timeout();
    }

    return machine_idx;
}

void protocol_handle_timeout(int signal) {
    printf("Timeout!\n");
    fflush(stdout);
    if (write(options.fd, last_frame.bytes, last_frame.size) < last_frame.size) {
        printf("protocol_handle_timeout: could not resend frame\n");
        protocol_reset_timeout();
        return;
    }

    printf("Wrote: ");
    for (int i = 0; i < last_frame.size; i++) {
        printf("%x (%c) / ", last_frame.bytes[i], last_frame.bytes[i]);
    }
    printf("\n");
    fflush(stdout);

    last_frame.tries_left--;
    if (last_frame.tries_left <= 0) {
        protocol_reset_timeout();
        return;
    }

    alarm(options.timeout);
}

void protocol_reset_timeout() {
    alarm(0);
    memset(&last_frame, 0, sizeof(LastFrame));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int protocol_connect_rx() {
    if (protocol_read_frame(&state_machine_set, 1, TRUE) < 0) {
        return -1;
    }

    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, UA);

    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    return 1;
}

int protocol_connect_tx() {
    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, SET);

    if (protocol_send_frame(frame, frame_size, TRUE) < 0) {
        return -1;
    }

    if (protocol_read_frame(&state_machine_ua, 1, TRUE) < 0) {
        return -1;
    }

    return 1;
}

int protocol_connect() {
    return options.role == LlTx ? protocol_connect_tx() : protocol_connect_rx();
}

int protocol_information_read(unsigned char *data, unsigned int length) {
    static int has_read_information = FALSE;
    static int expected_sequence_nr = 0;

    if (!has_read_information) {
        { // Answer pending SET
            StateMachine set_and_i_machine[2] = { state_machine_set, state_machine_i };

            unsigned char ua_frame[MAX_FRAME_SIZE];
            unsigned int ua_frame_size = frame_create_unnumbered(ua_frame, UA);
        
            while (TRUE) {
                // If we read a set, we want to send an UA
                unsigned int frame_idx = protocol_read_frame(set_and_i_machine, 2, TRUE);
                if (frame_idx == 0) { // SET
                    if (protocol_send_frame(ua_frame, ua_frame_size, FALSE) < 0) {
                        return -1;
                    }
                } else if (frame_idx == 1) { // I
                    has_read_information = TRUE;
                    break;
                } else { // ERROR
                    return -1;
                }
            }
        }
    } else {
        if (protocol_read_frame(&state_machine_i, 1, TRUE) < 0) {
            return -1;
        }
    }

    // The first I frame was read 

    unsigned int bytes_read = 0;
    InformationFrame i_frame = state_get_i();

    if (i_frame.sequence_nr == expected_sequence_nr) {
        // This is a new frame, we need to return it's payload to the layer above
        memcpy(data, i_frame.payload.bytes, i_frame.payload.size);
        bytes_read = i_frame.payload.size;

        expected_sequence_nr = (expected_sequence_nr + 1) % 2;
    }

    // TODO REJ
    // If we read an I, we want to send an RR
    unsigned char rr_frame[MAX_FRAME_SIZE];
    unsigned int rr_frame_size = frame_create_supervision(rr_frame, RR, expected_sequence_nr);

    if (protocol_send_frame(rr_frame, rr_frame_size, FALSE) < 0) {
        return -1;
    }

    return bytes_read;
}

int protocol_information_send(const unsigned char *data, unsigned int length) {
    static int sequence_nr = 0;

    unsigned char i_frame[MAX_FRAME_SIZE];
    unsigned int i_frame_size = frame_create_information(i_frame, sequence_nr, data, length);

    if (protocol_send_frame(i_frame, i_frame_size, TRUE) < 0) {
        return -1;
    }

    if (protocol_read_frame(&state_machine_rr, 1, TRUE) < 0) {
        return 0;
    }

    ReceiverReadyFrame rr_frame = state_get_rr();
    if (rr_frame.sequence_nr == sequence_nr) {
        return 0; // TODO se forem iguais, é preciso ler outra RR
    }

    sequence_nr = (sequence_nr + 1) % 2;
    return length;
}

int protocol_disconnect_tx() {
    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, DISC);
    if (protocol_send_frame(frame, frame_size, TRUE) < 0) {
        return -1;
    }

    if (protocol_read_frame(&state_machine_disc, 1, TRUE) < 0) {
        return -1;
    }

    frame_size = frame_create_unnumbered(frame, UA);
    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    return 1;
}

int protocol_disconnect_rx() {
    if (protocol_read_frame(&state_machine_disc, 1, TRUE) < 0) {
        return -1;
    }

    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, DISC);
    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    if (protocol_read_frame(&state_machine_ua, 1, FALSE) < 0) {
        return -1;
    }
    
    return 1;
}

int protocol_disconnect() {
    return options.role == LlTx ? protocol_disconnect_tx() : protocol_disconnect_rx();
}
