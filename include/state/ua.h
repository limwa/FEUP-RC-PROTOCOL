#pragma once

#include <stdbool.h>

/**
 * @brief Whether or not the packet previously read was a UA or not.
 * 
 * @return true if the packet previously read was a UA
 * @return false if the packet previously read was not a UA
 */
bool state_is_ua();

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