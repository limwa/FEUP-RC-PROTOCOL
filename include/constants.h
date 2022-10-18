#pragma once

#define TRUE 1
#define FALSE 0

#define MAX_FRAME_SIZE 2048

#define BIT(n) (1 << (n))

#define FLAG 0x7E

#define A_TX_CMD 0x03
#define A_TX_RES 0x01
#define A_RX_CMD 0x01
#define A_RX_RES 0x03

#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_I(n) ((n) * BIT(6))
#define C_RR(n) ((n) & BIT(7)) | 0x05
#define C_REJ(n) ((n) & BIT(7)) | 0x01

#define ESC 0x7D
#define ESC_FLAG 0x5E
#define ESC_ESC 0x5D
