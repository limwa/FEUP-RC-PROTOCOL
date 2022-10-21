#include "state.h"

StateMachine state_machine_set = { state_is_set, state_clear_set, state_read_set };
StateMachine state_machine_ua = { state_is_ua, state_clear_ua, state_read_ua };
StateMachine state_machine_i = { state_is_i, state_clear_i, state_read_i };
StateMachine state_machine_rr = { state_is_rr, state_clear_rr, state_read_rr };
StateMachine state_machine_rej = { state_is_rej, state_clear_rej, state_read_rej };
StateMachine state_machine_disc = { state_is_disc, state_clear_disc, state_read_disc };
