#include "state_machine.h"

// ------------------------------------------
// private functions
//

static void* STM_state_get_action(stm_state_t* st)
{
	return st->action;
}


static void* STM_state_get_transition(stm_state_t* st)
{
	return st->transition;
}


static stm_state_t* STM_transition_get_state(stm_transition_t* tr)
{
	return tr->st;
}


static stm_event_t STM_transition_get_event(stm_transition_t* tr)
{
	return tr->ev;
}


static stm_transition_t* STM_transition_get_transition(stm_transition_t* tr)
{
	return tr->tr;
}


// ------------------------------------------
// public functions
//

// initialize the given state machine
u8 STM_init(stm_t* stm, stm_state_t* st)
{
	// if the state machine doesn't exist
	if ( stm == NULL ) {
		// KO
		return KO;
	}

	// if the initial state is invalid
	if ( st == NULL ) {
		// KO
		return KO;
	}

	stm->state = st;

	return OK;
}


// run the protothread action
void STM_run(stm_t* stm)
{
	(void)PT_SCHEDULE(stm->thread(&stm->pt, stm->args));
}


// post an event to the state machine
u8 STM_event(stm_t* stm, stm_event_t ev)
{
	// if not valid state machine
	if ( stm == NULL ) {
		// KO
		return KO;
	}

	// for each transition from the current state
	for ( stm_transition_t* tr = STM_state_get_transition(stm->state); tr != NULL; tr = STM_transition_get_transition(tr) ) {
		// if the transition is valid
		if ( ev == STM_transition_get_event(tr) ) {
			// transit
			stm->state = STM_transition_get_state(tr);

			// do the associated action
			u8 (*action)(pt_t*, void*) = STM_state_get_action(stm->state);
			if ( action != NULL ) {
				PT_INIT(&stm->pt);
				stm->args = STM_state_get_action(stm->state);
				stm->thread = action;
			}

			// and exit on success
			return OK;
		}
	}

	return KO;
}

