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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef enum {
	FSM_EVENT_VAL_NA = -1,
	FSM_EVENT_VAL_CLEAR = 0,
	FSM_EVENT_VAL_SET = 1,
} fsm_event_val_t;

typedef struct {
	fsm_event_val_t *val;
	bool pol;
} fsm_event_t;

typedef void (*fsm_action_t)(void);

typedef struct {
	int present_state;
	int next_state;
	fsm_event_t event[CONFIG_FSM_EVENTS_NUM];
	fsm_action_t action;
} fsm_row_t;

typedef struct {
	int current_state;
	fsm_row_t *row_list;
} fsm_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 *
 * @param me       : Pointer to a fsm_t instance
 * @param row_list : Pointer to row list
 */
void fsm_init(fsm_t *const me, fsm_row_t *row_list);

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
