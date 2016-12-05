#include "state_machine.h"

// ------------------------------------------
// private functions
//

static void* nnk_stm_state_get_action(const struct nnk_stm_state* st)
{
	return (void*)pgm_read_word(&st->action);
}


static const struct nnk_stm_transition* nnk_stm_state_get_transition(const struct nnk_stm_state* st)
{
	return (const struct nnk_stm_transition*)pgm_read_word(&st->tr);
}


static void* nnk_stm_state_get_args(const struct nnk_stm_state* st)
{
	return (void*)pgm_read_word(&st->args);
}


static const struct nnk_stm_state* nnk_stm_transition_get_state(const struct nnk_stm_transition* tr)
{
	return (const struct nnk_stm_state*)pgm_read_word(&tr->st);
}


static u8 nnk_stm_transition_get_event(const struct nnk_stm_transition* tr)
{
	return pgm_read_byte(&tr->ev);
}


static const struct nnk_stm_transition* nnk_stm_transition_get_transition(const struct nnk_stm_transition* tr)
{
	return (const struct nnk_stm_transition*)pgm_read_word(&tr->tr);
}


// ------------------------------------------
// public functions
//

// initialize the given state machine
u8 nnk_stm_init(struct nnk_stm* stm, const struct nnk_stm_state* st)
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

	stm->state = (struct nnk_stm_state*)st;
	PT_INIT(&stm->pt);
	stm->args = nnk_stm_state_get_args(stm->state);
	stm->thread = nnk_stm_state_get_action(st);

	return OK;
}


// run the protothread action
void nnk_stm_run(struct nnk_stm* stm)
{
	(void)PT_SCHEDULE(stm->thread(&stm->pt, stm->args));
}


// post an event to the state machine
u8 nnk_stm_event(struct nnk_stm* stm, const u8 ev)
{
	// if not valid state machine
	if ( stm == NULL ) {
		// KO
		return KO;
	}

	// for each transition from the current state
	for ( const struct nnk_stm_transition* tr = nnk_stm_state_get_transition(stm->state); tr != NULL; tr = nnk_stm_transition_get_transition(tr) ) {
		// if the transition is valid
		if ( ev == nnk_stm_transition_get_event(tr) ) {
			// transit
			stm->state = (struct nnk_stm_state*)nnk_stm_transition_get_state(tr);

			// do the associated action
			u8 (*action)(pt_t*, void*) = nnk_stm_state_get_action(stm->state);
			if ( action != NULL ) {
				PT_INIT(&stm->pt);
				stm->args = nnk_stm_state_get_args(stm->state);
				stm->thread = action;
			}

			// and exit on success
			return OK;
		}
	}

	return KO;
}

