#pragma once

#include <stdbool.h>

/**
 * @brief Whether or not the packet previously read was a SET or not.
 * 
 * @return true if the packet previously read was a SET
 * @return false if the packet previously read was not a SET
 */
bool state_is_set();

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
