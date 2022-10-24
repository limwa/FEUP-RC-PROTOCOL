#pragma once

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "constants.h"

typedef enum {
    P_START = PACKET_C_START,
    P_END = PACKET_C_END
} ControlPacketType;

typedef struct {
    ControlPacketType type;
    unsigned char filename[UCHAR_MAX + 1];
    long filesize;
} ControlPacket;

typedef struct {

} DataPacket;

unsigned int packet_create_control(unsigned char *packet, ControlPacketType type, unsigned char *filename, long filesize);

unsigned int packet_create_data(unsigned char *packet, unsigned char seq_nr, unsigned char *data, unsigned short size);