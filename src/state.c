#include "state.h"

StateMachine state_machine_set = { state_is_set, state_clear_set, state_read_set };
StateMachine state_machine_ua = { state_is_ua, state_clear_ua, state_read_ua };
StateMachine state_machine_i = { state_is_i, state_clear_i, state_read_i };
StateMachine state_machine_rr = { state_is_rr, state_clear_rr, state_read_rr };
