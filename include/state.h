#pragma once

#include "state_set.h"
#include "state_ua.h"
#include "state_i.h"
#include "state_rr.h"
#include "state_rej.h"
#include "state_disc.h"

typedef struct {
    int (*is_frame)();
    void (*clear)();
    void (*read)(unsigned char byte);
} StateMachine;

extern StateMachine state_machine_set;
extern StateMachine state_machine_ua;
extern StateMachine state_machine_i;
extern StateMachine state_machine_rr;
extern StateMachine state_machine_rej;
extern StateMachine state_machine_disc;
