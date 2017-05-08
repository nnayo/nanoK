#ifndef __NNK_STATE_MACHINE_H__
# define __NNK_STATE_MACHINE_H__

#include "type_def.h"

#include "utils/pt.h"

#include <avr/pgmspace.h>


struct nnk_stm_state {
	u8 (*action)(pt_t* pt, void* args);     // action to execute on entering the state
	void* args;                             // argument to pass to action thread
	const struct nnk_stm_transition* tr;        // possible transition from state
} PROGMEM;

struct nnk_stm_transition {
	const u8 ev;                            // event triggering the transition
	const struct nnk_stm_state* st;             // state to go due to the transition
	const struct nnk_stm_transition* tr;        // pointer the chained transition
} PROGMEM;

// generic state machine type
struct nnk_stm {
	struct nnk_stm_state* state;                // current state of the machine
	pt_t pt;                                // protothread context associated to the state action
	void* args;                             // argument(s) to the protothread
	u8 (*thread)(pt_t* pt, void* args);     // current running protothread
};

// initialize the given state machine
u8 nnk_stm_init(struct nnk_stm* stm, const struct nnk_stm_state* state);

// run the protothread action
void nnk_stm_run(struct nnk_stm* stm);

// post an event to the state machine
u8 nnk_stm_event(struct nnk_stm* stm, const u8 ev);

#endif	// __NNK_STATE_MACHINE_H__
