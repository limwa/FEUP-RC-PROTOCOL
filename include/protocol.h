#pragma once

#include "link_layer.h"
#include "frame.h"
#include "state.h"

typedef struct {
    LinkLayerRole role;
    int fd;
    int timeout;
    int tries;
} ProtocolOptions;

typedef struct {
    unsigned char bytes[MAX_FRAME_SIZE];
    unsigned int size;
    volatile int tries_left;
} LastFrame;

void protocol_setup(ProtocolOptions options);

int protocol_send_frame(const unsigned char *frame, unsigned int size, int retry_on_timeout);
int protocol_read_frame(StateMachine *machines, unsigned int size, int reset_timeout_on_success);

void protocol_handle_timeout(int signal);
void protocol_reset_timeout();

/////////////////////////////////////////////////////////////////////////

int protocol_connect();
int protocol_information_read(unsigned char *data, unsigned int length);
int protocol_information_send(const unsigned char *data, unsigned int length);
int protocol_disconnect();