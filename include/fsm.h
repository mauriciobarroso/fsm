/**
 ******************************************************************************
 * @file           : fsm.h
 * @author         : Mauricio Barroso Benavides
 * @date           : Jun 2, 2024
 * @brief          : This file contains all the definitios, data types and
 *                   function prototypes for fsm.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FSM_H_
#define FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Exported macro ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum {
  FSM_ACTION_TYPE_ENTER = 0,
  FSM_ACTION_TYPE_UPDATE,
  FSM_ACTION_TYPE_EXIT,
  FSM_ACTION_TYPE_MAX,
} fsm_action_type_t;

typedef enum { FSM_OP_OR = 0, FSM_OP_AND, FSM_OP_MAX } fsm_op_t;

typedef struct {
  int *val;
  int cmp;
} fsm_event_t;

typedef void (*fsm_action_t)(void);

typedef struct {
  fsm_action_t (*actions)[3];
  size_t len;
} fsm_actions_list_t;

typedef struct {
  int present_state;
  int next_state;

  struct {
    fsm_event_t *events;
    size_t len;
  } events_list;

  fsm_op_t op;
  fsm_action_t action;
} fsm_trans_t;

typedef struct {
  fsm_trans_t *trans;
  size_t len;
} fsm_trans_list_t;

typedef struct {
  int current_state;
  int prev_state;
  fsm_trans_list_t trans_list;
  fsm_actions_list_t actions_list;

} fsm_t;

/* Exported constants --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 *
 * @param me         : Pointer to a fsm_t instance
 * @param init_state : FSM initial state
 *
 * @return 0:success, -1:fail
 */
int fsm_init(fsm_t *const me, int init_state);

/**
 * @brief Function to define and add a transition betwen 2 states for a FSM
 *        instance.
 *
 * @param me         : Pointer to a fsm_t instance
 * @param trans      : Pointer to a fsm_trans_t variable that stores the
                       transition data
 * @param from_state : FSM state from
 * @param next_state : FSM state to go
 * @param op         : Operator to evaluate the transition events
 *
 * @return 0:success, -1:fail
 */
int fsm_add_transition(fsm_t *const me, fsm_trans_t **trans, int from_state,
                       int next_state, fsm_op_t op);

/**
 * @brief Function to add an event for a transition for a FSM instance.
 *
 * @param me    : Pointer to a fsm_t instance
 * @param trans : Pointer to a trans_t variable to add the event
 * @param val   : Pointer to a int variable
 * @param cmp   : Value to compare val
 *
 * @return 0:success, -1:fail
 */
int fsm_add_event(fsm_t *const me, fsm_trans_t *trans, int *val, int cmp);

/**
 * @brief Function to register an action for a FSM state transition.
 *
 * @param me     : Pointer to a fsm_t instance
 * @param trans  : Pointer to a trans_t variable to check its transition
 * @param action : Action to execute when the trans events are met
 *
 * @return 0:success, -1:fail
 */
int fsm_register_trans_action(fsm_t *const me, fsm_trans_t *trans,
                              fsm_action_t action);

/**
 * @brief Function to register callbacks for a FSM state.
 *
 * @param me     : Pointer to a fsm_t instance
 * @param state  : FSM state to register the callback
 * @param enter  : Callback to execute when state is invoked
 * @param update : Callback to execute while state is present
 * @param exit   : Callback to execute when state is changed
 *
 * @return 0:success, -1:fail
 */
int fsm_register_state_actions(fsm_t *const me, int state, fsm_action_t enter,
                               fsm_action_t update, fsm_action_t exit);

/**
 * @brief Function to run FSM instance.
 *
 * @param me : Pointer to a fsm_t instance
 */
void fsm_run(fsm_t *const me);

#ifdef __cplusplus
}
#endif

#endif /* FSM_H_ */

/***************************** END OF FILE ************************************/
