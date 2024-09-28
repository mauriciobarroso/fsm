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
#include "include/fsm.h"

/* External variables --------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Function to initialize a FSM instance.
 */
void fsm_init(fsm_t *const me)
{
	/* Set current state to initial state */
	me->current_state = 0;

  me->rows = NULL;
  me->rows_num = 0;
}

/**
 * @brief Function to add row to FSM instance.
 */
void fsm_row_add(fsm_t *const me, fsm_row_t *row)
{
  if (me == NULL) {
    return;
  }

  if (row == NULL) {
    return;
  }

  fsm_row_t *ptr = (fsm_row_t *)realloc(me->rows, (me->rows_num + 1) * sizeof(fsm_row_t));

  if (ptr == NULL) {
    return;
  }

  me->rows = ptr;
  me->rows[me->rows_num] = *row;
  me->rows_num++;
}

/**
 * @brief Function to run FSM instance.
 */
void fsm_run(fsm_t *const me)
{
	for (uint8_t i = 0; i < me->rows_num; i++) {
		fsm_row_t row = me->rows[i];
		if (row.present_state == me->current_state) {
			bool condition = true;
			for (uint8_t j = 0; j < 2; j++) { /* todo: define in macros */
				fsm_event_t event = row.events[j];
				if (event.val != NULL) {
					if (*event.val != FSM_EVENT_VAL_NA) {
						bool val = event.pol ? *event.val : !(*event.val);

            if (row.cond == FSM_COND_AND) {
						  condition &= val;
            }
            else {
              condition |= val;
            }
					}
					else {
						condition = false;
					}
				}
			}

			if (condition) {
				if (row.action != NULL) {
					row.action();
				}

				me->current_state = row.next_state;
			}
		}
	}
}

/* Private functions ---------------------------------------------------------*/

/***************************** END OF FILE ************************************/

