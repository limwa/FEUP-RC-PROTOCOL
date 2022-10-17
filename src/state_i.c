#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h> 

#include "state_i.h"
#include "constants.h"

#define STATE_START 0
#define STATE_FLAG_RCV 1
#define STATE_A_RCV 2
#define STATE_C_RCV 3
#define STATE_BCC1_OK 4
#define STATE_DATA_RCV 5
#define STATE_DATA_RCV_ESC 6
#define STATE_BCC2_OK 7
#define STATE_STOP 8

static unsigned char state = STATE_START;

static int sequence_nr = 0;

static unsigned char buffer[MAX_FRAME_SIZE];
static int size = 0;

int state_is_i() {
    return state == STATE_STOP;
}

void state_clear_i() {
    state = STATE_START;
}

void state_read_i(unsigned char byte) {
    switch (byte) {
        case STATE_START:
            if (byte == FLAG) state = STATE_FLAG_RCV;
            break;

        case STATE_FLAG_RCV:
            if (byte == A_TX_CMD) state = STATE_A_RCV;
            else if (byte != FLAG) state = STATE_START;
            break;

        case STATE_A_RCV:
            if (byte == C_I(0)) {
                state = STATE_C_RCV;
                sequence_nr = 0;
            } else if (byte == C(1)) {
                state = STATE_C_RCV;
                sequence_nr = 1;
            } else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;
            break;

        case STATE_C_RCV:
            if (byte == (A_TX_CMD ^ C_I(sequence_nr))) state = STATE_BCC1_OK;
            else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;
            break;

        case STATE_BCC1_OK:

            break;

        case STATE_DATA_RCV:
            if (byte == FLAG) state = STATE_FLAG_RCV;
            else {
                buffer[size++] = byte;
            }


        case STATE_STOP:
            state = STATE_START;
            break;

        default:
            printf("i_read: unrecognized state\n");
            break;
    }
}
