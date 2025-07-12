/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Mauricio Barroso Benavides
 * @date           : Jul 7, 2025
 * @brief          : Multi function button FSM example
 ******************************************************************************
 * @attention
 *
 * MIT License
 *
 * Copyright (c) 2025 Mauricio Barroso Benavides
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
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>

/* Component includes */
#include "fsm.h"

/* Device specific includes */
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_timer.h"

/* Macros --------------------------------------------------------------------*/
#define TICK_MS 10
#define BUTTON_DEBOUNCE_MS 40
#define BUTTON_WAIT_DOUBLE_MS 100
#define BUTTON_WAIT_LONG_MS 3000
#define BUTTON_SHORT_PRESS_MS 2000
#define BUTTON_MEDIUM_PRESS_MS 5000
#define BUTTON_LONG_PRESS_MS 10000
#define BUTTON_GPIO_NUM CONFIG_BUTTON_GPIO_NUM

/* Typedef -------------------------------------------------------------------*/
typedef enum {
	BUTTON_STATE_IDLE = 0,
	BUTTON_STATE_DEBOUNCE,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_SINGLE,
	BUTTON_STATE_DOUBLE,
	BUTTON_STATE_LONG
} button_states_t;

/* Private variables ---------------------------------------------------------*/
static fsm_t button_fsm;
static int gpio_level;

/* Private function prototypes -----------------------------------------------*/
/* Device specific funtions */
static bool dev_gpio_init(int gpio);
static bool dev_get_gpio_level(int gpio);
static void dev_delay_ms(uint32_t ms);
static uint32_t dev_get_ms(void);

/* Event evaluation functions */
static bool eval_eq(int a, int b);

/* Button calback functions */
static void on_press_single(void);
static void on_press_double(void);
static void on_press_long(void);

/* FSM callbacks functions */
static void on_check_gpio(void);

/* Main ----------------------------------------------------------------------*/
void app_main(void) {
	dev_gpio_init(BUTTON_GPIO_NUM);

	/* Initialize button FSM instance */
	fsm_init(&button_fsm, BUTTON_STATE_IDLE, dev_get_ms);

	/* Add all the transitions and events for the button FSM instance */
	fsm_trans_t *trans = NULL;
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_IDLE,
					   BUTTON_STATE_DEBOUNCE);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 0, eval_eq);

	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_DEBOUNCE,
					   BUTTON_STATE_IDLE);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 1, eval_eq);
	fsm_add_event_timeout(&button_fsm, trans, BUTTON_DEBOUNCE_MS);

	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_DEBOUNCE,
					   BUTTON_STATE_PRESSED);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 0, eval_eq);
	fsm_add_event_timeout(&button_fsm, trans, BUTTON_DEBOUNCE_MS);

	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_PRESSED,
					   BUTTON_STATE_RELEASED);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 1, eval_eq);

	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_RELEASED,
					   BUTTON_STATE_SINGLE);
	fsm_add_event_timeout(&button_fsm, trans, BUTTON_WAIT_DOUBLE_MS);
	
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_RELEASED,
					   BUTTON_STATE_DOUBLE);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 0, eval_eq);
	
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_SINGLE,
					   BUTTON_STATE_IDLE);
					   
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_DOUBLE,
					   BUTTON_STATE_IDLE);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 1, eval_eq);
	
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_PRESSED,
					   BUTTON_STATE_LONG);
	fsm_add_event_timeout(&button_fsm, trans, BUTTON_WAIT_LONG_MS);
	
	fsm_add_transition(&button_fsm, &trans, BUTTON_STATE_LONG,
					   BUTTON_STATE_IDLE);
	fsm_add_event_cmp(&button_fsm, trans, &gpio_level, 1, eval_eq);		

	/* Register states callbacks */
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_IDLE, NULL,
							   on_check_gpio, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_DEBOUNCE, NULL,
							   on_check_gpio, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_PRESSED, NULL,
							   on_check_gpio, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_RELEASED, NULL,
							   on_check_gpio, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_SINGLE, on_press_single,
							   NULL, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_DOUBLE, on_press_double,
							   on_check_gpio, NULL);
	fsm_register_state_actions(&button_fsm, BUTTON_STATE_LONG, on_press_long,
							   on_check_gpio, NULL);

	/* Inifinite loop */
	for (;;) {
		/* Execute the button FSM every TICK_MS ms */
		fsm_run(&button_fsm);
		dev_delay_ms(TICK_MS);
	}
}

/* Private function definition -----------------------------------------------*/
/* Device specific funtions */
static bool dev_gpio_init(int gpio) {
	gpio_config_t gpio_cfg;
	gpio_cfg.pin_bit_mask = 1ULL << BUTTON_GPIO_NUM;
	gpio_cfg.mode = GPIO_MODE_INPUT;
	gpio_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_cfg.pull_down_en = GPIO_PULLUP_DISABLE;
	gpio_cfg.intr_type = GPIO_INTR_DISABLE;
	return gpio_config(&gpio_cfg);
}

static bool dev_get_gpio_level(int gpio) { return gpio_get_level(gpio); }

static void dev_delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(TICK_MS)); }

static uint32_t dev_get_ms(void) { return esp_timer_get_time() / 1000; }

/* Event evaluation functions */
static bool eval_eq(int a, int b) { return (a == b); }

/* Button calback functions */
static void on_press_single(void) { printf("Single click!\r\n"); }

static void on_press_double(void) { printf("Double click!\r\n"); }

static void on_press_long(void) { printf("Long click!\r\n"); }

/* FSM callbacks functions */
static void on_check_gpio(void) {
	gpio_level = dev_get_gpio_level(BUTTON_GPIO_NUM);
}

/***************************** END OF FILE ************************************/