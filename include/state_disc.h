#pragma once

/**
 * @brief Whether or not the frame previously read was a DISC or not.
 * 
 * @return 1 if the frame previously read was a DISC
 * @return 0 if the frame previously read was not a DISC
 */
int state_is_disc();

/**
 * @brief Reset the state machine for DISC to the starting state.
 */
void state_clear_disc();

/**
 * @brief Update the state machine for DISC based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_disc(unsigned char byte);
