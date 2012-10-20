#ifndef __STATE_MACHINE_H__
# define __STATE_MACHINE_H__

#include "type_def.h"

#include "utils/pt.h"


typedef u8 stm_event_t;

typedef struct stm_state {
	u8 (*action)(pt_t* pt, void* args);	// action to execute on entering the state
	void* args;				// argument to pass to action thread
	struct stm_transition* transition;	// possible transition from state
} stm_state_t;

typedef struct stm_transition {
	stm_event_t ev;		// event triggering the transition
	stm_state_t* st;	// state to go due to the transition
	struct stm_transition* tr;	// pointer the chained transition
} stm_transition_t;

// generic state machine type
typedef struct {
	stm_state_t* state;		// current state of the machine
	pt_t pt;			// protothread context associated to the state action
	void* args;			// argument(s) to the protothread
	u8 (*thread)(pt_t* pt, void* args);	// current running protothread
} stm_t;

// initialize the given state machine
extern u8 STM_init(stm_t* stm, stm_state_t* state);

// run the protothread action
extern void STM_run(stm_t* stm);

// post an event to the state machine
extern u8 STM_event(stm_t* stm, stm_event_t ev);

#endif	// __STATE_MACHINE_H__
