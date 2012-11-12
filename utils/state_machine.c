#include "state_machine.h"

// ------------------------------------------
// private functions
//

static void* STM_state_get_action(const stm_state_t* st)
{
	return (void*)pgm_read_word(&st->action);
}


static const struct stm_transition* STM_state_get_transition(const stm_state_t* st)
{
	return (const struct stm_transition*)pgm_read_word(&st->transition);
}


static void* STM_state_get_args(const stm_state_t* st)
{
	return (void*)pgm_read_word(&st->args);
}


static const stm_state_t* STM_transition_get_state(const stm_transition_t* tr)
{
	return (const stm_state_t*)pgm_read_word(&tr->st);
}


static stm_event_t STM_transition_get_event(const stm_transition_t* tr)
{
	return (stm_event_t)pgm_read_byte(&tr->ev);
}


static const stm_transition_t* STM_transition_get_transition(const stm_transition_t* tr)
{
	return (const stm_transition_t*)pgm_read_word(&tr->tr);
}


// ------------------------------------------
// public functions
//

// initialize the given state machine
u8 STM_init(stm_t* stm, const stm_state_t* st)
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

	stm->state = (stm_state_t*)st;
	PT_INIT(&stm->pt);
	stm->args = STM_state_get_args(stm->state);
	stm->thread = STM_state_get_action(st);

	return OK;
}


// run the protothread action
void STM_run(stm_t* stm)
{
	(void)PT_SCHEDULE(stm->thread(&stm->pt, stm->args));
}


// post an event to the state machine
u8 STM_event(stm_t* stm, const stm_event_t ev)
{
	// if not valid state machine
	if ( stm == NULL ) {
		// KO
		return KO;
	}

	// for each transition from the current state
	for ( const stm_transition_t* tr = STM_state_get_transition(stm->state); tr != NULL; tr = STM_transition_get_transition(tr) ) {
		// if the transition is valid
		if ( ev == STM_transition_get_event(tr) ) {
			// transit
			stm->state = (stm_state_t*)STM_transition_get_state(tr);

			// do the associated action
			u8 (*action)(pt_t*, void*) = STM_state_get_action(stm->state);
			if ( action != NULL ) {
				PT_INIT(&stm->pt);
				stm->args = STM_state_get_args(stm->state);
				stm->thread = action;
			}

			// and exit on success
			return OK;
		}
	}

	return KO;
}

