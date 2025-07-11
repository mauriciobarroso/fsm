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

/* Private function prototypes -----------------------------------------------*/
static int get_next_state(fsm_trans_list_t *trans_list, int current_state,
                          uint32_t elapsed_ms);
static void execute_action(int current_state, fsm_actions_list_t *actions_list,
                           fsm_action_type_t type);
static bool eval_events(fsm_trans_t *trans);
static bool eval_timeout(fsm_trans_t *trans, uint32_t elapsed_time);

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 */
fsm_err_t fsm_init(fsm_t *const me, int init_state, fsm_time_t get_ms) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Set default values */
  me->current_state = init_state;
  me->prev_state = me->current_state - 1;
  me->actions_list.actions = NULL;
  me->actions_list.len = 0;
  me->trans_list.trans = NULL;
  me->trans_list.len = 0;
  me->get_ms = get_ms;
  me->entry_ms = 0;

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to add a transition betwen state to FSM instance.
 */
fsm_err_t fsm_add_transition(fsm_t *const me, fsm_trans_t **trans,
                             int from_state, int next_state) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check is the transition states are valid */
  if (from_state == next_state) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Allocate memory for the new transition and check*/
  fsm_trans_t *ptr =
      realloc(me->trans_list.trans, (me->trans_list.len + 1) * sizeof *ptr);

  if (ptr == NULL) {
    return FSM_ERR_NO_MEM;
  }

  /* Assign the reallocated memory and add 1 to len */
  me->trans_list.trans = ptr;
  me->trans_list.len++;

  /* Set the values for the new transition element */
  size_t index = me->trans_list.len - 1;
  me->trans_list.trans[index].events_list.events = NULL;
  me->trans_list.trans[index].events_list.len = 0;
  me->trans_list.trans[index].present_state = from_state;
  me->trans_list.trans[index].next_state = next_state;
  me->trans_list.trans[index].op = FSM_OP_AND; /* default operator */
  me->trans_list.trans[index].action = NULL;
  me->trans_list.trans[index].timeout = 0;

  /* Assign the last transition added to transition out parameter */
  *trans = &me->trans_list.trans[index];

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to set the operator to evaluate the transition events.
 */
fsm_err_t fsm_set_event_op(fsm_t *const me, fsm_trans_t *trans, fsm_op_t op) {
  /* Check if the FSM operator is valid */
  if (op < 0 || op >= FSM_OP_MAX) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Set the new operator */
  trans->op = op;

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to add an event for a transition for a FSM instance.
 */
fsm_err_t fsm_add_event_cmp(fsm_t *const me, fsm_trans_t *trans, int *val, int cmp,
                        fsm_eval_t eval) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the transition pointer is valid */
  if (trans == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the event value pointer is valid */
  if (val == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the evaluation function is valid */
  if (eval == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the transition is part of the FSM */
  fsm_trans_t *base = me->trans_list.trans;
  size_t len = me->trans_list.len;
  if (!(trans >= base && trans < base + len)) {
    return FSM_ERR_FAIL;
  }

  /* Allocate memory for the new event and check */
  fsm_event_t *ptr = realloc(trans->events_list.events,
                             (trans->events_list.len + 1) * sizeof *ptr);

  if (ptr == NULL) {
    return FSM_ERR_NO_MEM;
  }

  /* Assign the reallocated memory and add 1 to len */
  trans->events_list.events = ptr;
  trans->events_list.len++;

  /* Set the values for the new event element */
  size_t index = trans->events_list.len - 1;
  trans->events_list.events[index].val = val;
  trans->events_list.events[index].cmp = cmp;
  trans->events_list.events[index].eval = eval;

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to add a timeout event for a transition for a FSM instance.
 */
fsm_err_t fsm_add_event_timeout(fsm_t *const me, fsm_trans_t *trans,
                                uint32_t timeout) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the transition pointer is valid */
  if (trans == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the pointer to get ms is valid */
  if (me->get_ms == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Assign new tiemout */
  trans->timeout = timeout;

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to register an action for a FSM state transition.
 */
fsm_err_t fsm_register_trans_action(fsm_t *const me, fsm_trans_t *trans,
                                    fsm_action_t action) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the transition pointer is valid */
  if (trans == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Assign the action function pointer */
  trans->action = action;

  /* Return success */
  return FSM_ERR_OK;
}

/**
 * @brief Function to register callbacks for a FSM state.
 */
fsm_err_t fsm_register_state_actions(fsm_t *const me, int state,
                                     fsm_action_t enter, fsm_action_t update,
                                     fsm_action_t exit) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return FSM_ERR_INVALID_PARAM;
  }

  /* Check if the FSM state is valid */
  if (state < 0) {
    return FSM_ERR_INVALID_PARAM;
  }

  if (state >= me->actions_list.len) {
    /* Allocate */
    fsm_action_t(*ptr)[3] =
        realloc(me->actions_list.actions, (state + 1) * sizeof *ptr);

    if (ptr == NULL) {
      return FSM_ERR_NO_MEM;
    }

    me->actions_list.actions = ptr;
    me->actions_list.len = state + 1;
  }

  me->actions_list.actions[state][FSM_ACTION_TYPE_ENTER] = enter;
  me->actions_list.actions[state][FSM_ACTION_TYPE_UPDATE] = update;
  me->actions_list.actions[state][FSM_ACTION_TYPE_EXIT] = exit;

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
    execute_action(me->current_state, &me->actions_list, FSM_ACTION_TYPE_ENTER);
    me->prev_state = me->current_state;
  } else {
    execute_action(me->current_state, &me->actions_list,
                   FSM_ACTION_TYPE_UPDATE);
  }

  /* Evaluate the transition event and get the next FSM state. If the current
  FSM state change then execute the exit action */
  int next_state =
      get_next_state(&me->trans_list, me->current_state, now_ms - me->entry_ms);

  if (next_state != me->current_state) {
    execute_action(me->current_state, &me->actions_list, FSM_ACTION_TYPE_EXIT);
    me->prev_state = me->current_state;
    me->current_state = next_state;
  }

  /* Return success */
  return FSM_ERR_OK;
}

/* Private functions ---------------------------------------------------------*/
static int get_next_state(fsm_trans_list_t *trans_list, int current_state,
                          uint32_t elapsed_ms) {
  for (size_t i = 0; i < trans_list->len; i++) {
    /* Find coincidences for current state */
    fsm_trans_t *trans = &trans_list->trans[i];
    if (trans->present_state == current_state) {
      if (!trans->events_list.len && !trans->timeout) {
        goto TRANSITION;
      }

      /* Set condition initial value according the operator */
      bool res = 0;
      bool cmp_res = 0;
      bool timeout_res = 0;

      /* Evaluate all transition events */
      cmp_res = eval_events(trans);

      /* Evalute timeout event */
      timeout_res = eval_timeout(trans, elapsed_ms);

      if (trans->op == FSM_OP_AND) {
        res = cmp_res & timeout_res;
      } else {
        res = cmp_res | timeout_res;
      }

      if (res) {
      TRANSITION:
        /* Execute the transition action */
        if (trans->action != NULL) {
          trans->action();
        }

        /* Return the next state */
        return trans->next_state;
      }
    }
  }

  /* Return the current state as next state */
  return current_state;
}

static void execute_action(int current_state, fsm_actions_list_t *actions_list,
                           fsm_action_type_t type) {
  /* Check if actions type is valid*/
  if (type < FSM_ACTION_TYPE_ENTER || type >= FSM_ACTION_TYPE_MAX) {
    return;
  }

  /* Check if the current FSM state callback was registered */
  if (current_state < actions_list->len) {
    if (actions_list->actions[current_state][type] != NULL) {
      actions_list->actions[current_state][type]();
    }
  }
}

static bool eval_events(fsm_trans_t *trans) {
  bool ret = trans->op == FSM_OP_AND ? 1 : 0;

  if (!trans->events_list.len) {
    return ret;
  }

  for (size_t i = 0; i < trans->events_list.len; i++) {
    fsm_event_t event = trans->events_list.events[i];
    if (event.val != NULL) {
      /* Perform the comparation */
      if (trans->op == FSM_OP_AND) {
        ret &= event.eval(*event.val, event.cmp);
      } else {
        ret |= event.eval(*event.val, event.cmp);
      }
    }
  }

  return ret;
}

static bool eval_timeout(fsm_trans_t *trans, uint32_t elapsed_time) {
  bool ret = trans->op == FSM_OP_AND ? 1 : 0;

  if (!trans->timeout) {
    return ret;
  }

  ret = elapsed_time >= trans->timeout;

  return ret;
}

/***************************** END OF FILE ************************************/
