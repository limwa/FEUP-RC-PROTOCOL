#pragma once

#include "constants.h"

typedef struct {
    unsigned char sequence_nr;
    
    struct {
        int is_valid;
        
        unsigned char bytes[MAX_FRAME_SIZE];
        unsigned int size;
    } payload;
} InformationFrame;

/**
 * @brief Whether or not the frame previously read was an I frame or not.
 * 
 * @return 1 if the frame previously read was an I frame
 * @return 0 if the frame previously read was not an I frame
 */
int state_is_i();

/**
 * @brief Reset the state machine for I to the starting state.
 */
void state_clear_i();

/**
 * @brief Update the state machine for I based on a single byte that was received.
 * 
 * @param byte the byte that was received
 */
void state_read_i(unsigned char byte);

InformationFrame state_get_i();
