#pragma once

#include "connection.h"

int conn_tx_handshake();

int conn_tx_send_set();
int conn_tx_read_ua();

int byte_destuffing(unsigned char* data, unsigned int size);  // tirar daqui
int byte_stuffing(unsigned char* data, unsigned int size);  // tirar daqui
int check_bcc2(unsigned char* data, unsigned int size); // tirar daqui
