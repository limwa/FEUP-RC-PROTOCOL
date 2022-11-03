#include "state_i.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "frame.h"
#include "statistics.h"

#define STATE_START 0
#define STATE_FLAG_RCV 1
#define STATE_A_RCV 2
#define STATE_C_RCV 3
#define STATE_BCC1_OK 4
#define STATE_DATA_RCV 5
#define STATE_BCC2_OK 6
#define STATE_BCC2_NOT_OK 7

static unsigned char state = STATE_START;
static InformationFrame current_frame;

int state_is_i() {
    return state == STATE_BCC2_OK || state == STATE_BCC2_NOT_OK;
}

void state_clear_i() {
    state = STATE_START;
    memset(&current_frame, 0, sizeof(InformationFrame));
}

void state_read_i(unsigned char byte) {
    statistics_set_last_received_bytes(0);
    switch (state) {
        case STATE_START:
            if (byte == FLAG) state = STATE_FLAG_RCV;
            break;

        case STATE_FLAG_RCV:
            if (byte == A_TX_CMD) state = STATE_A_RCV;
            else if (byte != FLAG) state = STATE_START;
            break;

        case STATE_A_RCV:
            memset(&current_frame, 0, sizeof(current_frame));

            if (byte == C_I(0)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 0;
            } else if (byte == C_I(1)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 1;
            } else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;
            break;

        case STATE_C_RCV:
            if (byte == (A_TX_CMD ^ C_I(current_frame.sequence_nr))) state = STATE_DATA_RCV;
            else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;
            break;

        case STATE_DATA_RCV:
            if (byte == FLAG) {
                statistics_set_last_received_bytes(current_frame.payload.size + 5);
                unsigned int new_size = frame_copy_destuffed(current_frame.payload.bytes, current_frame.payload.bytes, current_frame.payload.size);
                current_frame.payload.size = new_size - 1; // Subtract one because of BCC2

                unsigned char expected_bcc2 = frame_check_bcc2(current_frame.payload.bytes, current_frame.payload.size);
                unsigned char actual_bcc2 = current_frame.payload.bytes[current_frame.payload.size];

                current_frame.payload.bytes[current_frame.payload.size] = 0; // Remove BCC2 from payload

                #ifdef SIM_FER
                if (expected_bcc2 == actual_bcc2 && rand() % 100 < SIM_FER) {
                    printf("simulated bad\n");
                    actual_bcc2 ^= 0xFF; // make it so they aren't equal
                }
                #endif

                if (expected_bcc2 == actual_bcc2) {
                    statistics_count_frame_good();
                    state = STATE_BCC2_OK;
                    current_frame.payload.is_valid = TRUE;
                } else {
                    statistics_set_last_received_bytes(0);
                    statistics_count_frame_bad();
                    state = STATE_BCC2_NOT_OK;
                    current_frame.payload.is_valid = FALSE;
                }
            } else {
                current_frame.payload.bytes[current_frame.payload.size] = byte;
                current_frame.payload.size++;

                if (current_frame.payload.size >= MAX_FRAME_SIZE) {
                    state = STATE_START;
                }
            }

            break;

        case STATE_BCC2_OK:
        case STATE_BCC2_NOT_OK:
            break;

        default:
            printf("i_read: unrecognized state\n");
            break;
    }
}

InformationFrame state_get_i() {
    return current_frame;
}
