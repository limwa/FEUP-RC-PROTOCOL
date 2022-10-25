#include "packet.h"

unsigned int packet_create_control(unsigned char *packet, ControlPacket ctrl) {
    packet[0] = ctrl.type;

    packet[1] = PACKET_T_FILESIZE;

    unsigned int curr_pos = 2;
    packet[curr_pos++] = sizeof(ctrl.filesize);
    memcpy(packet + curr_pos, &ctrl.filesize, sizeof(ctrl.filesize));
    curr_pos += sizeof(ctrl.filesize);

    packet[curr_pos++] = PACKET_T_FILENAME;

    unsigned char filename_len = strlen((char*) ctrl.filename) + 1;
    packet[curr_pos++] = filename_len;
    memcpy(packet + curr_pos, ctrl.filename, filename_len);
    curr_pos += filename_len;

    return curr_pos;
}

unsigned int packet_create_data(unsigned char *packet, DataPacket data) {
    packet[0] = PACKET_C_DATA;
    packet[1] = data.seq_nr;
    
    unsigned int curr_pos = 2;
    memcpy(packet + curr_pos, &data.payload_size, sizeof(data.payload_size));
    curr_pos += sizeof(data.payload_size);

    memcpy(packet + curr_pos, data.payload, data.payload_size);

    return curr_pos + data.payload_size;
}

int packet_read_control(ControlPacket *result, unsigned char *packet) {
    unsigned char c = packet[0];

    switch (c) {
        case PACKET_C_START:
            result->type = P_START;
            break;
            
        case PACKET_C_END:
            result->type = P_END;
            break;

        default:
            printf("packet_read_control: unrecognized packet type (%x)\n", c);
            return -1;
    }

    unsigned int curr_pos = 1;
    for (int i = 0; i < 2; i++) {

        unsigned char t = packet[curr_pos++];
        unsigned char length;
        switch (t) {
            case PACKET_T_FILESIZE:
                length = packet[curr_pos++];
                if (length > sizeof(result->filesize)) {
                    printf("packet_read_control: filesize too big (%d bytes)\n", length);
                    return -1;
                }

                memcpy(&result->filesize, packet + curr_pos, length);
                curr_pos += sizeof(result->filesize);
                break;

            case PACKET_T_FILENAME:
                length = packet[curr_pos++];
                memcpy(result->filename, packet + curr_pos, length);
                curr_pos += length;
                break;

            default:
                printf("packet_read_control: unrecognized packet type (%x)\n", t);
                return -1;
        }
    }

    return 1;
}

int packet_read_data(DataPacket *result, unsigned char *packet) {
    unsigned char c = packet[0];
    if (c != PACKET_C_DATA) {
        printf("packet_read_data: unrecognized packet type (%x)\n", c);
        return -1;
    }
    
    result->seq_nr = packet[1];

    unsigned int curr_pos = 2;

    memcpy(&result->payload_size, packet + curr_pos, sizeof(result->payload_size));
    curr_pos += sizeof(result->payload_size);
    
    memcpy(&result->payload, packet + curr_pos, result->payload_size);
    
    return 1;
}
