#pragma once

#include "connection.h"

int conn_rx_handshake();

int conn_rx_read_set();
int conn_rx_send_ua();
int conn_rx_disconnect();
