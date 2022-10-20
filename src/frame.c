#include "frame.h"

#include <stdio.h>
#include <strings.h>

static unsigned char command_addr, response_addr;

void frame_set_role(LinkLayerRole role) {
    if (role == LlTx) {
        command_addr = A_TX_CMD;
        response_addr = A_TX_RES;
    } else {
        command_addr = A_RX_CMD;
        response_addr = A_RX_RES;
    }
}

unsigned char frame_get_command_addr() {
    return command_addr;
}

unsigned char frame_get_response_addr() {
    return response_addr;
}

unsigned int frame_create_unnumbered(unsigned char *frame, UnnumberedFrameType type) {
    unsigned char ctrl, addr;
    switch (type) {
        case SET:
            ctrl = C_SET;
            addr = command_addr;
            break;

        case DISC:
            ctrl = C_DISC;
            addr = command_addr;
            break;

        case UA:
            ctrl = C_UA;
            addr = response_addr;
            break;

        default:
            printf("frame_create_unnumbered: unknown frame type\n");
            return 0;
    }

    return frame_assemble(frame, addr, ctrl, NULL, 0);
}

unsigned int frame_create_supervision(unsigned char *frame, SupervisionFrameType type, unsigned char sequence_nr) {
    unsigned char ctrl, addr;
    switch (type) {
        case RR:
            ctrl = C_RR(sequence_nr);
            addr = response_addr;
            break;

        case REJ:
            ctrl = C_REJ(sequence_nr);
            addr = response_addr;
            break;

        default:
            printf("frame_create_supervision: unknown frame type\n");
            return 0;
    }

    return frame_assemble(frame, addr, ctrl, NULL, 0);
}

unsigned int frame_create_information(unsigned char *frame, unsigned char sequence_nr, const unsigned char *data, unsigned int size) {
    return frame_assemble(frame, command_addr, C_I(sequence_nr), data, size);
}

unsigned int frame_assemble(unsigned char *frame, unsigned char addr, unsigned char ctrl, const unsigned char *data, unsigned int size) {
    frame[0] = FLAG;
    frame[1] = addr;
    frame[2] = ctrl;
    frame[3] = addr ^ ctrl;
    
    unsigned int flag_pos = 4;
    if (size > 0) {
        unsigned char bcc2 = frame_check_bcc2(data, size);
        unsigned int new_data_size = frame_copy_stuffed(frame + 4, data, size);
        unsigned int new_bcc2_size = frame_copy_stuffed(frame + new_data_size + 4, &bcc2, sizeof(bcc2));
        flag_pos += new_bcc2_size + new_data_size;
    }

    frame[flag_pos] = FLAG;
    return flag_pos + 1; // size = position of the ending flag + 1 (because it's an index)
}


unsigned int frame_copy_stuffed(unsigned char *dest, const unsigned char *src, unsigned int size) {
    unsigned int new_size = 0;

    for (int i = 0, j = 0; i < size; i++, j++) {
        if (src[i] == FLAG) {
            dest[j] = ESC; 
            j++;
            dest[j] = ESC_FLAG;
        } else if (src[i] == ESC) {
            dest[j] = ESC;
            j++;
            dest[j] = ESC_ESC;
        } else {
            dest[j] = src[i];
        }

        new_size = j;
    }

    return new_size;
}

unsigned int frame_copy_destuffed(unsigned char *dest, const unsigned char *src, unsigned int size) {
    unsigned int new_size = 0;
    
    for (int i = 0, j = 0; i < size; i++, j++) {
        if (src[i] == ESC) {
            i++;
            if (src[i] == ESC_FLAG) {
                dest[j] = FLAG;
            } else if (src[i] == ESC_ESC) {
                dest[j] = ESC;
            }
        } else {
            dest[j] = src[i];
        }

        new_size = j;
    }

    return new_size;
}

unsigned char frame_check_bcc2(const unsigned char* data, unsigned int size) {
    unsigned char bcc2 = data[0];
    for (int i = 1; i < size; i++) {
        bcc2 ^= data[i];
    }

    return bcc2;
}