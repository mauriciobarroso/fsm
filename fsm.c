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
static int get_next_state(fsm_trans_list_t *trans_list, int current_state);
static void execute_action(int current_state, fsm_actions_list_t *actions_list,
                           fsm_action_type_t type);

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 */
int fsm_init(fsm_t *const me, int init_state) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return -1;
  }

  /* Set default values */
  me->current_state = init_state;
  me->prev_state = me->current_state - 1;

  me->actions_list.actions = NULL;
  me->actions_list.len = 0;

  me->trans_list.trans = NULL;
  me->trans_list.len = 0;

  return 0;
}

/**
 * @brief Function to add a transition betwen state to FSM instance.
 */
int fsm_add_transition(fsm_t *const me, fsm_trans_t **trans, int from_state,
                       int next_state, fsm_op_t op) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return -1;
  }

  /* Allocate memory for the new transition and check*/
  fsm_trans_t *ptr =
      realloc(me->trans_list.trans, (me->trans_list.len + 1) * sizeof *ptr);

  if (ptr == NULL) {
    return -1;
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
  me->trans_list.trans[index].op = op;

  /**/
  *trans = &me->trans_list.trans[index];

  /* Return 0 for success */
  return 0;
}

/**
 * @brief Function to add an event for a transition for a FSM instance.
 */
int fsm_add_event(fsm_t *const me, fsm_trans_t *trans, int *val, int cmp) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return -1;
  }

  /* Check if the transition pointer is valid */
  if (trans == NULL) {
    return -1;
  }

  /* Check if the transition is part of the FSM */
  fsm_trans_t *base = me->trans_list.trans;
  size_t len = me->trans_list.len;
  if (!(trans >= base && trans < base + len)) {
    return -1;
  }

  /* Allocate memory for the new event and check */
  fsm_event_t *ptr = realloc(trans->events_list.events,
                             (trans->events_list.len + 1) * sizeof *ptr);

  if (ptr == NULL) {
    return -1;
  }

  /* Assign the reallocated memory and add 1 to len */
  trans->events_list.events = ptr;
  trans->events_list.len++;

  /* Set the values for the new event element */
  size_t index = trans->events_list.len - 1;
  trans->events_list.events[index].val = val;
  trans->events_list.events[index].cmp = cmp;

  /* Return 0 for success */
  return 0;
}

/**
 * @brief Function to register an action for a FSM state transition.
 */
int fsm_register_trans_action(fsm_t *const me, fsm_trans_t *trans,
                              fsm_action_t action) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return -1;
  }

  /* Check if the transition pointer is valid */
  if (trans == NULL) {
    return -1;
  }

  /* Assign the action function pointer */
  trans->action = action;

  /* Return 0 for success */
  return 0;
}

/**
 * @brief Function to register callbacks for a FSM state.
 */
int fsm_register_state_actions(fsm_t *const me, int state, fsm_action_t enter,
                               fsm_action_t update, fsm_action_t exit) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return -1;
  }

  /* Check if the FSM state is valid */
  if (state < 0) {
    return -1;
  }

  if (state >= me->actions_list.len) {
    /* Allocate */
    fsm_action_t(*ptr)[3] =
        realloc(me->actions_list.actions, (state + 1) * sizeof *ptr);

    if (ptr == NULL) {
      return -1;
    }

    me->actions_list.actions = ptr;
    me->actions_list.len = state + 1;
  }

  me->actions_list.actions[state][FSM_ACTION_TYPE_ENTER] = enter;
  me->actions_list.actions[state][FSM_ACTION_TYPE_UPDATE] = update;
  me->actions_list.actions[state][FSM_ACTION_TYPE_EXIT] = exit;

  /* Return 0 for success */
  return 0;
}

/**
 * @brief Function to run FSM instance.
 */
void fsm_run(fsm_t *const me) {
  /* Check if the FSM instance is valid */
  if (me == NULL) {
    return;
  }

  /* Execute the enter action if the current FSM state comes from a different
  state and update the previous FSM state. In other case execute the update
  action */
  if (me->current_state != me->prev_state) {
    execute_action(me->current_state, &me->actions_list, FSM_ACTION_TYPE_ENTER);
    me->prev_state = me->current_state;
  } else {
    execute_action(me->current_state, &me->actions_list,
                   FSM_ACTION_TYPE_UPDATE);
  }

  /* Evaluate the transition event and get the next FSM state. If the current
  FSM state change then execute the exit action */
  int next_state = get_next_state(&me->trans_list, me->current_state);

  if (next_state != me->current_state) {
    execute_action(me->current_state, &me->actions_list, FSM_ACTION_TYPE_EXIT);
    me->prev_state = me->current_state;
    me->current_state = next_state;
  }
}

/* Private functions ---------------------------------------------------------*/
static int get_next_state(fsm_trans_list_t *trans_list, int current_state) {
  for (size_t i = 0; i < trans_list->len; i++) {
    /* Find coincidences for current state */
    fsm_trans_t *trans = &trans_list->trans[i];
    if (trans->present_state == current_state) {
      /* Set condition initial value according the operator */
      bool condition = trans->op == FSM_OP_AND ? true : false;

      /* Evaluate all transition events */
      if (!trans->events_list.len) {
        condition = false;
      } else {
        for (size_t j = 0; j < trans->events_list.len; j++) {
          fsm_event_t event = trans->events_list.events[j];
          if (event.val != NULL) {
            if (trans->op == FSM_OP_AND) {
              condition &= (*event.val == event.cmp);
            } else {
              condition |= (*event.val == event.cmp);
            }
          }
        }
      }

      if (condition) {
        return trans->next_state;
      }
    }
  }

  /* Return the next state */
  return current_state;
}

static void execute_action(int current_state, fsm_actions_list_t *actions_list,
                           fsm_action_type_t type) {
  if (type < FSM_ACTION_TYPE_ENTER || type >= FSM_ACTION_TYPE_MAX) {
    return;
  }

  if (current_state < actions_list->len) {
    if (actions_list->actions[current_state][type] != NULL) {
      actions_list->actions[current_state][type]();
    }
  }
}

/***************************** END OF FILE ************************************/
