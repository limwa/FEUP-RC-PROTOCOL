#pragma once

#include "state_set.h"
#include "state_ua.h"

typedef struct {
    int (*is_frame)();
    void (*clear)();
    void (*read)(unsigned char byte);
} state_machine;

state_machine state_machine_set = { state_is_set, state_clear_set, state_read_set };
state_machine state_machine_ua = { state_is_ua, state_clear_ua, state_read_ua };
