#pragma once

#include "constants.h"
#include "link_layer.h"

typedef enum {
    SET, DISC, UA
} UnnumberedFrameType;

typedef enum {
    RR, REJ
} SupervisionFrameType;

void frame_set_role(LinkLayerRole role);

unsigned char frame_get_command_addr();
unsigned char frame_get_response_addr();

unsigned int frame_create_unnumbered(unsigned char *frame, UnnumberedFrameType type);
unsigned int frame_create_supervision(unsigned char *frame, SupervisionFrameType type, unsigned char sequence_nr);
unsigned int frame_create_information(unsigned char *frame, unsigned char sequence_nr, const unsigned char *data, unsigned int size);
unsigned int frame_assemble(unsigned char *frame, unsigned char addr, unsigned char ctrl, const unsigned char *data, unsigned int size);

unsigned int frame_copy_stuffed(unsigned char *dest, const unsigned char *src, unsigned int size);
unsigned int frame_copy_destuffed(unsigned char *dest, const unsigned char *src, unsigned int size);
unsigned char frame_check_bcc2(const unsigned char* data, unsigned int size);