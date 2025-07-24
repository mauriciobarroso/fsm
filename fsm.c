/**
 ******************************************************************************
 * @file           : fsm.c
 * @author         : Mauricio Barroso Benavides
 * @date           : Jun 2, 2024
 * @brief          : This file provides code for the configuration and control
 *                   of FSM
 ******************************************************************************
 * @attention
 *
 * MIT License
 *
 * Copyright (c) 2024 Mauricio Barroso Benavides
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "fsm.h"

/* External variables --------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define FSM_STATE_NONE UINT8_MAX
#define FSM_TIMEOUT_MAX UINT32_MAX - 1

/* Private function prototypes -----------------------------------------------*/
static uint8_t get_next_state(fsm_trans_list_t *trans_list,
							  uint8_t current_state, uint32_t elapsed_ms);
static void execute_action(uint8_t current_state,
						   fsm_action_t actions[][FSM_ACTION_TYPE_MAX],
						   fsm_action_type_t type);
static bool eval_events(const fsm_events_t *events, uint32_t elapsed_ms);

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 */
fsm_err_t fsm_init(fsm_t *const me, uint8_t init_state, fsm_time_t get_ms) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the FSM init state is valid */
	if (init_state >= FSM_STATES_NUM_MAX) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Set default values */
	me->current_state = init_state;
	me->prev_state = FSM_STATE_NONE;
	me->trans_list.len = 0;
	me->get_ms = get_ms;
	me->entry_ms = 0;

	for (uint8_t i = 0; i < FSM_STATES_NUM_MAX; i++) {
		for (uint8_t j = 0; j < FSM_ACTION_TYPE_MAX; j++) {
			me->state_actions[i][j].fn = NULL;
			me->state_actions[i][j].arg = NULL;
		}
	}

	/* Return success */
	return FSM_ERR_OK;
}

/**
 * @brief Function to add a transition betwen state to FSM instance.
 */
fsm_err_t fsm_add_transition(fsm_t *const me, fsm_trans_t **trans,
							 uint8_t present_state, uint8_t next_state) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the FSM states are valid */
	if (present_state >= FSM_STATES_NUM_MAX ||
		next_state >= FSM_STATES_NUM_MAX) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the transition is valid */
	if (present_state == next_state) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the maximum transition number was reached */
	if (me->trans_list.len >= FSM_TRANS_NUM_MAX) {
		return FSM_ERR_TRANS_LIST_FULL;
	}

	/* Set states transition */
	me->trans_list.trans[me->trans_list.len].present_state = present_state;
	me->trans_list.trans[me->trans_list.len].next_state = next_state;

	/* Set events default values */
	me->trans_list.trans[me->trans_list.len].events.val = NULL;
	me->trans_list.trans[me->trans_list.len].events.cmp = 0;
	me->trans_list.trans[me->trans_list.len].events.eval = NULL;
	me->trans_list.trans[me->trans_list.len].events.op = FSM_OP_AND;
	me->trans_list.trans[me->trans_list.len].events.timeout = 0;

	/* Set transition action default values */
	me->trans_list.trans[me->trans_list.len].action.fn = NULL;
	me->trans_list.trans[me->trans_list.len].action.arg = NULL;

	/* Assign current transition to trans output parameter */
	*trans = &me->trans_list.trans[me->trans_list.len];

	/* Increment transitions list lenght */
	me->trans_list.len++;

	/* Return success */
	return FSM_ERR_OK;
}

/**
 * @brief Function to add the events and condition to perform a transition for a
 * FSM instance.
 */
fsm_err_t fsm_set_events(fsm_t *const me, fsm_trans_t *trans, int *val, int cmp,
						 fsm_eval_t eval, uint32_t timeout, fsm_op_t op) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the transition pointer is valid */
	if (trans == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the condition operator is valid */
	if (op != FSM_OP_OR && op != FSM_OP_AND) {
		return FSM_ERR_INVALID_PARAM;
	}

	/**/
	trans->events.val = val;
	trans->events.cmp = cmp;
	trans->events.eval = eval;
	trans->events.timeout = timeout;
	trans->events.op = op;

	/* Return success */
	return FSM_ERR_OK;
}

/**
 * @brief Function to register an action for a FSM state transition.
 */
fsm_err_t fsm_register_trans_action(fsm_t *const me, fsm_trans_t *trans,
									fsm_fn_t fn, void *arg) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the transition pointer is valid */
	if (trans == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Assign the action function pointer */
	trans->action.fn = fn;
	trans->action.arg = arg;

	/* Return success */
	return FSM_ERR_OK;
}

/**
 * @brief Function to register callbacks for a FSM state.
 */
fsm_err_t fsm_register_state_actions(fsm_t *const me, uint8_t state,
									 fsm_fn_t entry_fn, void *entry_arg,
									 fsm_fn_t update_fn, void *update_arg,
									 fsm_fn_t exit_fn, void *exit_arg) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Check if the FSM state is valid */
	if (state >= FSM_STATES_NUM_MAX) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Assign actions parameters */
	me->state_actions[state][FSM_ACTION_TYPE_ENTRY].fn = entry_fn;
	me->state_actions[state][FSM_ACTION_TYPE_ENTRY].arg = entry_arg;
	me->state_actions[state][FSM_ACTION_TYPE_UPDATE].fn = update_fn;
	me->state_actions[state][FSM_ACTION_TYPE_UPDATE].arg = update_arg;
	me->state_actions[state][FSM_ACTION_TYPE_EXIT].fn = exit_fn;
	me->state_actions[state][FSM_ACTION_TYPE_EXIT].arg = exit_arg;

	/* Return success */
	return FSM_ERR_OK;
}

/**
 * @brief Function to run FSM instance.
 */
fsm_err_t fsm_run(fsm_t *const me) {
	/* Check if the FSM instance is valid */
	if (me == NULL) {
		return FSM_ERR_INVALID_PARAM;
	}

	/* Read the current time */
	uint32_t now_ms = me->get_ms ? me->get_ms() : 0;

	/* Execute the enter action if the current FSM state comes from a different
	state and update the previous FSM state. In other case execute the update
	action */
	if (me->current_state != me->prev_state) {
		me->entry_ms = now_ms;
		execute_action(me->current_state, me->state_actions,
					   FSM_ACTION_TYPE_ENTRY);
		me->prev_state = me->current_state;
	} else {
		execute_action(me->current_state, me->state_actions,
					   FSM_ACTION_TYPE_UPDATE);
	}

	/* Evaluate the transition event and get the next FSM state. If the current
	FSM state change then execute the exit action */
	uint32_t elapsed_ms = FSM_TIMEOUT_MAX;

	if (me->get_ms) {
		if (now_ms < me->entry_ms) {
			elapsed_ms = (FSM_TIMEOUT_MAX - me->entry_ms) + now_ms + 1;
		} else {
			elapsed_ms = now_ms - me->entry_ms;
		}
	}

	uint8_t next_state =
		get_next_state(&me->trans_list, me->current_state, elapsed_ms);

	if (next_state != me->current_state) {
		execute_action(me->current_state, me->state_actions,
					   FSM_ACTION_TYPE_EXIT);
		me->prev_state = me->current_state;
		me->current_state = next_state;
	}

	/* Return success */
	return FSM_ERR_OK;
}

/* Private functions ---------------------------------------------------------*/
static uint8_t get_next_state(fsm_trans_list_t *trans_list,
							  uint8_t current_state, uint32_t elapsed_ms) {
	for (uint8_t i = 0; i < trans_list->len; i++) {
		/* Find coincidences for current state */
		fsm_trans_t *trans = &trans_list->trans[i];
		if (trans->present_state == current_state) {
			if (eval_events(&trans->events, elapsed_ms)) {
				if (trans->action.fn != NULL) {
					trans->action.fn(trans->action.arg);
				}

				/* Return the next state */
				return trans->next_state;
			}
		}
	}

	/* Return the current state as next state */
	return current_state;
}

static void execute_action(uint8_t current_state,
						   fsm_action_t actions[][FSM_ACTION_TYPE_MAX],
						   fsm_action_type_t type) {
	/* Check if actions type is valid*/
	if (type < FSM_ACTION_TYPE_ENTRY || type >= FSM_ACTION_TYPE_MAX) {
		return;
	}

	/* Check if the current FSM state callback was registered */
	if (actions[current_state][type].fn != NULL) {
		actions[current_state][type].fn(actions[current_state][type].arg);
	}
}

static bool eval_events(const fsm_events_t *events, uint32_t elapsed_ms) {
	if (events == NULL) {
		return false;
	}

	// Definimos el elemento neutro según el operador
	bool identity = (events->op == FSM_OP_AND) ? true : false;

	// ¿Está definida la comparación?
	bool cmp_defined = (events->eval != NULL && events->val != NULL);
	// ¿Está definido el timeout?
	bool to_defined = (events->timeout > 0);

	// Si está definido, lo evaluamos; si no, usamos el neutro
	bool cmp_ok =
		cmp_defined ? events->eval(*(events->val), events->cmp) : identity;

	bool to_ok = to_defined ? (elapsed_ms >= events->timeout) : identity;

	// Combinamos según AND u OR
	if (events->op == FSM_OP_AND) {
		return (cmp_ok && to_ok);
	} else { // FSM_OP_OR
		return (cmp_ok || to_ok);
	}
}

/***************************** END OF FILE ************************************/
