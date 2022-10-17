#pragma once

/**
 * @brief Whether or not the frame previously read was a UA or not.
 * 
 * @return 1 if the frame previously read was a UA
 * @return 0 if the frame previously read was not a UA
 */
int state_is_ua();

/**
 * @brief Reset the state machine for UA to the starting state.
 */
void state_clear_ua();

/**
 * @brief Update the state machine for UA based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_ua(unsigned char byte);