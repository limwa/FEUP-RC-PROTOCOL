#pragma once

typedef struct {
    unsigned char sequence_nr;
} RejectFrame;

/**
 * @brief Whether or not the frame previously read was a REJ or not.
 * 
 * @return 1 if the frame previously read was a REJ
 * @return 0 if the frame previously read was not a REJ
 */
int state_is_rej();

/**
 * @brief Reset the state machine for REJ to the starting state.
 */
void state_clear_rej();

/**
 * @brief Update the state machine for REJ based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_rej(unsigned char byte);

RejectFrame state_get_rej();