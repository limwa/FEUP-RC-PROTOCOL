#include "packet.h"

unsigned int packet_create_control(unsigned char *packet, ControlPacketType type, unsigned char *filename, long filesize) {
    packet[0] = type;

    packet[1] = PACKET_T_FILESIZE;

    unsigned int curr_pos = 2;
    packet[curr_pos++] = sizeof(filesize);
    memcpy(packet + curr_pos, &filesize, sizeof(filesize));
    curr_pos += sizeof(filesize);

    packet[curr_pos++] = PACKET_T_FILENAME;

    unsigned char filename_len = strlen(filename) + 1;
    packet[curr_pos++] = filename_len;
    memcpy(packet + curr_pos, filename, filename_len);
    curr_pos += filename_len;

    return curr_pos;
}

unsigned int packet_create_data(unsigned char *packet, unsigned char seq_nr, unsigned char *data, unsigned short size) {
    packet[0] = PACKET_C_DATA;
    packet[1] = seq_nr;
    
    unsigned int curr_pos = 2;
    memcpy(packet + curr_pos, size, sizeof(size));
    curr_pos += sizeof(size);

    memcpy(packet + curr_pos, data, size);
    return curr_pos + size;
}

