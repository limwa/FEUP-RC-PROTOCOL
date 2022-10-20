#pragma once

typedef struct {
    unsigned char sequence_nr;
} ReceiverReadyFrame;
/**
 * @brief Whether or not the frame previously read was a RR or not.
 * 
 * @return 1 if the frame previously read was a RR
 * @return 0 if the frame previously read was not a RR
 */
int state_is_rr();

/**
 * @brief Reset the state machine for RR to the starting state.
 */
void state_clear_rr();

/**
 * @brief Update the state machine for RR based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_rr(unsigned char byte);

ReceiverReadyFrame state_get_rr();
