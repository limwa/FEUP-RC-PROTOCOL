#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h> 

#include "state_rej.h"
#include "constants.h"
#include "statistics.h"

#define STATE_START 0
#define STATE_FLAG_RCV 1
#define STATE_A_RCV 2
#define STATE_C_RCV 3
#define STATE_BCC_OK 4
#define STATE_STOP 5


static unsigned char state = STATE_START;
static RejectFrame current_frame;

int state_is_rej() {
    return state == STATE_STOP;
}

void state_clear_rej() {
    state = STATE_START;
}

void state_read_rej(unsigned char byte) {
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
            if (byte == C_REJ(0)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 0;
            } else if (byte == C_REJ(1)) {
                state = STATE_C_RCV;
                current_frame.sequence_nr = 1;
            } 
            else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;

            break;  

        case STATE_C_RCV:   
            if (byte == (A_TX_CMD ^ C_SET)) state = STATE_BCC_OK;
            else if (byte == FLAG) state = STATE_FLAG_RCV;
            else state = STATE_START;
            
            break;

        case STATE_BCC_OK:
            if (byte == FLAG) state = STATE_STOP;
            else state = STATE_START;
            
            break;  
        
        default:
            printf("rej_read: unrecognized state\n");
            break;
    }
}

RejectFrame state_get_rej() {
    return current_frame;
}
