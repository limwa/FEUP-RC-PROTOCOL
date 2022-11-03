#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>

#include "statistics.h"

static ProtocolOptions options;
static LastFrame last_frame;

void protocol_setup(ProtocolOptions new_options) {
    signal(SIGALRM, protocol_handle_timeout);
    options = new_options;
}

int protocol_send_frame(const unsigned char *frame, unsigned int size, int retry_on_timeout) {
    if (write(options.fd, frame, size) < size) {
        printf("protocol_send_frame: could not send frame\n");
        return -1;
    }

    if (retry_on_timeout) {
        memcpy(last_frame.bytes, frame, size);
        last_frame.size = size;
        last_frame.tries_left = options.tries;

        #ifdef TESTING
        printf("Alarm set! %d tries left.\n", options.tries - 1);
        #endif

        alarm(options.timeout);
    }

    return 1;
}

int protocol_read_frame(StateMachine *machines, unsigned int size, int reset_timeout_on_success) {
    
    #ifdef SIM_T_PROP
    poll(NULL, 0, SIM_T_PROP);
    #endif
    
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

        for (unsigned int i = 0; i < size; i++) {
            machines[i].read(buf);
            if (machines[i].is_frame()) {
                machine_idx = i;
                break;
            }
        }
    }


    if (reset_timeout_on_success) {
        protocol_reset_timeout();
    }

    return machine_idx;
}

void protocol_handle_timeout(int signal) {

    if (write(options.fd, last_frame.bytes, last_frame.size) < last_frame.size) {
        printf("protocol_handle_timeout: could not resend frame\n");
        protocol_reset_timeout();
        return;
    }

    last_frame.tries_left--;
    if (last_frame.tries_left <= 0) {
        protocol_reset_timeout();
        return;
    }

    #ifdef TESTING
    printf("Alarm! %d tries left.\n", last_frame.tries_left - 1);
    #endif

    alarm(options.timeout);
}

void protocol_reset_timeout() {
    #ifdef TESTING
    printf("Alarm reset!\n");
    #endif

    alarm(0);
    memset(&last_frame, 0, sizeof(LastFrame));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int protocol_connect_rx() {
    #ifdef TESTING
    printf("<<< SET <<<\n");
    #endif

    if (protocol_read_frame(&state_machine_set, 1, TRUE) < 0) {
        return -1;
    }

    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, UA);

    #ifdef TESTING
    printf(">>> UA >>>\n");
    #endif

    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    return 1;
}

int protocol_connect_tx() {
    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, SET);

    #ifdef TESTING
    printf(">>> SET >>>\n");
    #endif

    if (protocol_send_frame(frame, frame_size, TRUE) < 0) {
        return -1;
    }

    #ifdef TESTING
    printf("<<< UA <<<\n");
    #endif
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
                #ifdef TESTING
                printf("<<< SET | I <<<\n");
                #endif

                // If we read a set, we want to send an UA
                unsigned int frame_idx = protocol_read_frame(set_and_i_machine, 2, TRUE);
                if (frame_idx == 0) { // SET
                    #ifdef TESTING
                    printf("! SET\n");
                    printf(">>> UA >>>\n");
                    #endif
                    if (protocol_send_frame(ua_frame, ua_frame_size, FALSE) < 0) {
                        return -1;
                    }
                } else if (frame_idx == 1) { // I
                    #ifdef TESTING
                    printf("! I\n");
                    #endif
                    has_read_information = TRUE;
                    break;
                } else { // ERROR
                    return -1;
                }
            }
        }
    } else {
        #ifdef TESTING
        printf("<<< I <<<\n");
        #endif
        if (protocol_read_frame(&state_machine_i, 1, TRUE) < 0) {
            return -1;
        }
    }

    // The first I frame was read 

    unsigned int bytes_read = 0;
    InformationFrame i_frame = state_get_i();

    #ifdef TESTING
    printf("I : Ns = %d, Valid = %d\n", i_frame.sequence_nr, i_frame.payload.is_valid);
    #endif

    unsigned char res_frame[MAX_FRAME_SIZE];
    if (i_frame.sequence_nr == expected_sequence_nr) {
        if (!i_frame.payload.is_valid) {
            unsigned int rej_frame_size = frame_create_supervision(res_frame, REJ, expected_sequence_nr);
            #ifdef TESTING
            printf(">>> REJ >>>\n");
            printf("REJ : Nr = %d\n", expected_sequence_nr);
            #endif
            if (protocol_send_frame(res_frame, rej_frame_size, FALSE) < 0) {
                return -1;
            }

            return bytes_read;
        }

        // This is a new frame, we need to return it's payload to the layer above
        memcpy(data, i_frame.payload.bytes, i_frame.payload.size);
        bytes_read = i_frame.payload.size;
        statistics_add_received_bytes();

        expected_sequence_nr = (expected_sequence_nr + 1) % 2;
    } 

    // If we read an I, we want to send an RR
    unsigned int rr_frame_size = frame_create_supervision(res_frame, RR, expected_sequence_nr);
    #ifdef TESTING
    printf(">>> RR >>>\n");
    printf("RR : Nr = %d\n", expected_sequence_nr);
    #endif
    if (protocol_send_frame(res_frame, rr_frame_size, FALSE) < 0) {
        return -1;
    }

    return bytes_read;
}

int protocol_information_send(const unsigned char *data, unsigned int length) {
    static int sequence_nr = 0;

    unsigned char i_frame[MAX_FRAME_SIZE];
    unsigned int i_frame_size = frame_create_information(i_frame, sequence_nr, data, length);

    #ifdef TESTING
    printf(">>> I >>>\n");
    printf("I : Ns = %d\n", sequence_nr);
    #endif

    if (protocol_send_frame(i_frame, i_frame_size, TRUE) < 0) {
        return -1;
    }

    int ready_sequence_nr;
    StateMachine machines[2] = { state_machine_rr, state_machine_rej };

    do {
        #ifdef TESTING
        printf("<<< RR | REJ <<<\n");
        #endif
        int machine_idx = protocol_read_frame(machines, 2, FALSE);
        if (machine_idx < 0) {
            return -1;
        }
        
        if (machine_idx == 0) {
            ready_sequence_nr = state_get_rr().sequence_nr;

            #ifdef TESTING
            printf("! RR\n");
            printf("RR : Nr = %d\n", ready_sequence_nr);
            #endif

        } else  if (machine_idx == 1) {
            ready_sequence_nr = state_get_rej().sequence_nr;
            
            #ifdef TESTING
            printf("! REJ\n");
            printf("REJ : Nr = %d\n", ready_sequence_nr);
            #endif
            if (ready_sequence_nr == sequence_nr) {
                protocol_reset_timeout();

                #ifdef TESTING
                printf(">>> I >>>\n");
                printf("I : Ns = %d\n", sequence_nr);
                #endif

                if (protocol_send_frame(i_frame, i_frame_size, TRUE) < 0) {
                    return -1;
                }
            }
        }
    } while (ready_sequence_nr == sequence_nr);

    protocol_reset_timeout();
    sequence_nr = (sequence_nr + 1) % 2;

    return length;
}

int protocol_disconnect_tx() {
    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, DISC);

    #ifdef TESTING
    printf(">>> DISC >>>\n");
    #endif
    
    if (protocol_send_frame(frame, frame_size, TRUE) < 0) {
        return -1;
    }

    #ifdef TESTING
    printf ("<<< DISC <<<\n");
    #endif
    if (protocol_read_frame(&state_machine_disc, 1, TRUE) < 0) {
        return -1;
    }

    frame_size = frame_create_unnumbered(frame, UA);

    #ifdef TESTING
    printf(">>> UA >>>\n");
    #endif

    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    return 1;
}

int protocol_disconnect_rx() {

    #ifdef TESTING
    printf("<<< DISC <<<\n");
    #endif

    if (protocol_read_frame(&state_machine_disc, 1, TRUE) < 0) {
        return -1;
    }

    unsigned char frame[MAX_FRAME_SIZE];
    unsigned int frame_size = frame_create_unnumbered(frame, DISC);

    #ifdef TESTING
    printf(">>> DISC >>>\n");
    #endif

    if (protocol_send_frame(frame, frame_size, FALSE) < 0) {
        return -1;
    }

    #ifdef TESTING
    printf("<<< UA <<<\n");
    #endif

    if (protocol_read_frame(&state_machine_ua, 1, FALSE) < 0) {
        return -1;
    }
    
    return 1;
}

int protocol_disconnect() {
    return options.role == LlTx ? protocol_disconnect_tx() : protocol_disconnect_rx();
}
