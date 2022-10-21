#include "state_i.h"

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "frame.h"

#define STATE_START 0
#define STATE_FLAG_RCV 1
#define STATE_A_RCV 2
#define STATE_C_RCV 3
#define STATE_BCC1_OK 4
#define STATE_DATA_RCV 5
#define STATE_STOP 6

static unsigned char state = STATE_START;
static InformationFrame current_frame;

int state_is_i() {
    return state == STATE_STOP;
}

void state_clear_i() {
    state = STATE_START;
    memset(&current_frame, 0, sizeof(InformationFrame));
}

void state_read_i(unsigned char byte) {
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
                unsigned int new_size = frame_copy_destuffed(current_frame.payload.bytes, current_frame.payload.bytes, current_frame.payload.size);
                current_frame.payload.size = new_size - 1; // Subtract one because of BCC2

                unsigned char expected_bcc2 = frame_check_bcc2(current_frame.payload.bytes, current_frame.payload.size);
                unsigned char actual_bcc2 = current_frame.payload.bytes[current_frame.payload.size];

                current_frame.payload.bytes[current_frame.payload.size] = 0; // Remove BCC2 from payload

                if (expected_bcc2 == actual_bcc2) state = STATE_STOP;
                else state = STATE_FLAG_RCV;
            } else {
                current_frame.payload.bytes[current_frame.payload.size] = byte;
                current_frame.payload.size++;

                if (current_frame.payload.size >= MAX_FRAME_SIZE) {
                    state = STATE_START;
                }
            }

            break;

        case STATE_STOP:
            state = STATE_START;
            break;

        default:
            printf("i_read: unrecognized state\n");
            break;
    }
}

InformationFrame state_get_i() {
    return current_frame;
}
