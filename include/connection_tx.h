#pragma once

#include "connection.h"

int conn_tx_handshake();

int conn_tx_send_set();
int conn_tx_read_ua();
int conn_tx_disconnect();
