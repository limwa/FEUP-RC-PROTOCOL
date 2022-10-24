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
    unsigned char seq_nr;
    unsigned char payload[MAX_PAYLOAD_SIZE - 4]; // 4 bytes for C, N, L1 and L2
    unsigned short payload_size;
} DataPacket;

unsigned int packet_create_control(unsigned char *packet, ControlPacket ctrl);

unsigned int packet_create_data(unsigned char *packet, DataPacket data);

int packet_read_control(ControlPacket *result, unsigned char *packet);

int packet_read_data(DataPacket *result, unsigned char *packet);
