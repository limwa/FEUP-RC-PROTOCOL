#pragma once

// MISC
#define TRUE 1
#define FALSE 0
#define BIT(n) (1 << (n))

// FRAMES
#ifndef MAX_PAYLOAD_SIZE
#include "link_layer.h"
#endif

#define MAX_FRAME_SIZE (1 + 2 * (3 + MAX_PAYLOAD_SIZE + 1) + 1)

#define FLAG 0x7E

#define A_TX_CMD 0x03
#define A_TX_RES 0x01
#define A_RX_CMD 0x01
#define A_RX_RES 0x03

#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_I(n) ((n) * BIT(6))
#define C_RR(n) (((n) * BIT(7)) | 0x05)
#define C_REJ(n) (((n) * BIT(7)) | 0x01)

#define ESC 0x7D
#define ESC_FLAG 0x5E
#define ESC_ESC 0x5D

// SERIAL PORT CONFIG
#define SERIAL_TIMEOUT 20

// APPLICATION LAYER
#define PACKET_C_DATA 0x01
#define PACKET_C_START 0x02
#define PACKET_C_END 0x03
#define PACKET_T_FILESIZE 0x00
#define PACKET_T_FILENAME 0x01

// TESTING
// #define SIM_FER 20 // Simulate frame error rate (0-100)
// #define SIM_T_PROP 2000 // Simulate transmission propagation time (ms)



// #define TESTING
#define TESTING_USE_FILENAME 1