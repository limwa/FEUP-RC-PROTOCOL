#include "state_rr.h"

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "statistics.h"

#define STATE_START 0
#define STATE_FLAG_RCV 1
#define STATE_A_RCV 2
#define STATE_C_RCV 3
#define STATE_BCC_OK 4
#define STATE_STOP 5

static unsigned char state = STATE_START;
static ReceiverReadyFrame current_frame;

int state_is_rr() {
    return state == STATE_STOP;
}

void state_clear_rr() {
    state = STATE_START;
}

void state_read_rr(unsigned char byte) {
    switch (state) {
        case STATE_STOP:
            state = STATE_START;

        case STATE_START:
            if (byte == FLAG) state = STATE_FLAG_RCV;

            break;

        case STATE_FLAG_RCV:
            if (byte == A_RX_RES) state = STATE_A_RCV;
            else if (byte != FLAG) state = STATE_START;

            break;

        case STATE_A_RCV:
            memset(&current_frame, 0, sizeof(current_frame));

            if (byte == C_RR(0)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 0;
            } else if (byte == C_RR(1)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 1;
            } 
            else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;

            break;  

        case STATE_C_RCV:   
            if (byte == (A_TX_CMD ^ C_RR(current_frame.sequence_nr))) state = STATE_BCC_OK;
            else {
                statistics_count_frame_bad();
                if (byte == FLAG) state = STATE_FLAG_RCV;
                else state = STATE_START;
            }
            break;

        case STATE_BCC_OK:
            if (byte == FLAG) {
                statistics_count_frame_good();
                state = STATE_STOP;
            } else state = STATE_START;
            
            break;  
        
        default:
            printf("rr_read: unrecognized state\n");
            break;
    }
}

ReceiverReadyFrame state_get_rr() {
    return current_frame;
}
