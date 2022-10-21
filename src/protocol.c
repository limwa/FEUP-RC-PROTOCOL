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

    for (int i = 0; i < size; i++) {
        printf("Wrote: %x (%c)\n", frame[i], frame[i]);
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

int protocol_read_frame(StateMachine *machines, unsigned int size) {
    printf("Read!\n");
    fflush(stdout);
    for (unsigned int machine_idx = 0; machine_idx < size; machine_idx++) {
        machines[machine_idx].clear();
    }

    unsigned char buf;
    unsigned int machine_idx = -1;
    while (machine_idx == -1) {
        int bytes_read = read(options.fd, &buf, 1);
        if (bytes_read <= 0) {
            break;
        }

        printf("Reading: %x (%c)\n", buf, buf);
        fflush(stdout);

        for (unsigned int i = 0; i < size; i++) {
            machines[i].read(buf);
            if (machines[i].is_frame()) {
                machine_idx = i;
                break;
            }
        }
    }

    printf("Accepted on machine: %d\n", machine_idx);

    protocol_reset_timeout();
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
    for (int i = 0; i < last_frame.size; i++) {
        printf("Wrote: %x (%c)\n", last_frame.bytes[i], last_frame.bytes[i]);
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
    if (protocol_read_frame(&state_machine_set, 1) < 0) {
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

    if (protocol_read_frame(&state_machine_ua, 1) < 0) {
        return -1;
    }

    return 1;
}

int protocol_connect() {
    return options.role == LlTx ? protocol_connect_tx() : protocol_connect_rx();
}

int protocol_information_read(unsigned char *data, unsigned int length) {
    static int has_read_information = FALSE;
    static int sequence_nr = 0;

    if (!has_read_information) {
        { // Answer pending SET
            StateMachine set_and_i_machine[2] = { state_machine_set, state_machine_i };

            unsigned char ua_frame[MAX_FRAME_SIZE];
            unsigned int ua_frame_size = frame_create_unnumbered(ua_frame, UA);
        
            while (TRUE) {
                // If we read a set, we want to send an UA
                unsigned int frame_idx = protocol_read_frame(set_and_i_machine, 2);
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
        if (protocol_read_frame(&state_machine_i, 1) < 0) {
            return -1;
        }
    }

    unsigned int bytes_read = 0;
    
    // The first I frame was read
    InformationFrame i_frame = state_get_i();
    if (sequence_nr == i_frame.sequence_nr) {
        sequence_nr = (sequence_nr + 1) % 2;
        memcpy(data, i_frame.payload.bytes, i_frame.payload.size);
        bytes_read = i_frame.payload.size;
    }

    // If we read an I, we want to send an RR
    unsigned char rr_frame[MAX_FRAME_SIZE];
    unsigned int rr_frame_size = frame_create_supervision(rr_frame, RR, sequence_nr);
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

    if (protocol_read_frame(&state_machine_rr, 1) < 0) {
        return -1;
    }

    ReceiverReadyFrame rr_frame = state_get_rr();
    if (rr_frame.sequence_nr == sequence_nr) {
        return -1;
    }

    sequence_nr = (sequence_nr + 1) % 2;
    return 1;
}