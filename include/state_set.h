#pragma once

/**
 * @brief Whether or not the frame previously read was a SET or not.
 * 
 * @return 1 if the frame previously read was a SET
 * @return 0 if the frame previously read was not a SET
 */
int state_is_set();

/**
 * @brief Reset the state machine for SET to the starting state.
 */
void state_clear_set();

/**
 * @brief Update the state machine for SET based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_set(unsigned char byte);
