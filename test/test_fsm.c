/**
 ******************************************************************************
 * @file           : test_fsm.c
 * @author         : Mauricio Barroso Benavides
 * @date           : Jul 11, 2024
 * @brief          : Unity test cases for the FSM library
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
#include "fsm.h"
#include "unity.h"

/* Private typedef -----------------------------------------------------------*/
// --- State definitions ---
enum { STATE_S0 = 0, STATE_S1, STATE_S2 };

/* Private variables ---------------------------------------------------------*/
static uint32_t fake_time;

/* Counters for original tests */
static int enter_init_cnt, update_init_cnt, exit_init_cnt;
static int enter_run_cnt, update_run_cnt, exit_run_cnt;
static int enter_stop_cnt, update_stop_cnt, exit_stop_cnt;

/* Counters for new timeout tests */
static int enter_s0_cnt, update_s0_cnt, exit_s0_cnt;
static int enter_s1_cnt, exit_s1_cnt;
static int enter_s2_cnt;

/* Private function prototypes -----------------------------------------------*/
static bool eval_eq(int a, int b) { return a == b; }
static uint32_t get_fake_time(void) { return fake_time; }

// --- Callback stubs for timeout tests ---
static void cb_enter_s0(void) { enter_s0_cnt++; }
static void cb_update_s0(void) { update_s0_cnt++; }
static void cb_exit_s0(void) { exit_s0_cnt++; }
static void cb_enter_s1(void) { enter_s1_cnt++; }
static void cb_exit_s1(void) { exit_s1_cnt++; }
static void cb_enter_s2(void) { enter_s2_cnt++; }

void setUp(void) {
	// Reset fake time
	fake_time = 0;
	// Reset original counters
	enter_init_cnt = update_init_cnt = exit_init_cnt = 0;
	enter_run_cnt = update_run_cnt = exit_run_cnt = 0;
	enter_stop_cnt = update_stop_cnt = exit_stop_cnt = 0;
	// Reset timeout test counters
	enter_s0_cnt = update_s0_cnt = exit_s0_cnt = 0;
	enter_s1_cnt = exit_s1_cnt = 0;
	enter_s2_cnt = 0;
}

void tearDown(void) {
	// Nothing to tear down
}

/* Test cases ----------------------------------------------------------------*/
void test_timeout_transition_only(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, cb_update_s0,
							   cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_add_event_timeout(&fsm, trans, 100);

	/* Before the timeout, execute S0 enter and update actions */
	for (int i = 0; i < 5; i++) {
		fake_time = i * 10;
		fsm_run(&fsm);
	}

	TEST_ASSERT_EQUAL_INT(1, enter_s0_cnt);
	TEST_ASSERT_EQUAL_INT(5, update_s0_cnt);
	TEST_ASSERT_EQUAL_INT(0, exit_s0_cnt);

	/* Transition on 100 timeout and execute S0 exit action*/
	fake_time = 100;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);

	/* In the next FSM run execute S1 enter action */
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
}

void test_and_transition_with_timeout(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	int var = 0;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_set_event_op(&fsm, trans, FSM_OP_AND);
	fsm_add_event_cmp(&fsm, trans, &var, 1, eval_eq);
	fsm_add_event_timeout(&fsm, trans, 50);
	fsm_run(&fsm);

	/* Only var event, no transition */
	var = 1;
	fake_time = 30;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(0, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(0, enter_s1_cnt);

	/* Only timeout event, no transition */
	var = 0;
	fake_time = 50;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(0, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(0, enter_s1_cnt);

	/* Both events, transition */
	var = 1;
	fake_time = 60;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
}

void test_or_transition_with_timeout(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	int var = 0;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_set_event_op(&fsm, trans, FSM_OP_OR);
	fsm_add_event_cmp(&fsm, trans, &var, 1, eval_eq);
	fsm_add_event_timeout(&fsm, trans, 50);

	// var==1 antes de timeout: debe transicionar
	var = 1;
	fsm_run(&fsm);
	fake_time = 10;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);

	// reset para siguiente escenario
	setUp();
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_set_event_op(&fsm, trans, FSM_OP_OR);
	fsm_add_event_cmp(&fsm, trans, &var, 1, eval_eq);
	fsm_add_event_timeout(&fsm, trans, 50);

	// timeout sin var==1: debe transicionar
	var = 0;
	fsm_run(&fsm);
	fake_time = 60;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
}

void test_timeout_reset_on_reenter_state(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	// S0 -> S1 en timeout 20ms
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_add_event_timeout(&fsm, trans, 20);
	// S1 -> S0 inmediato
	fsm_add_transition(&fsm, &trans, STATE_S1, STATE_S0);

	// Primera ida a S1
	fsm_run(&fsm);
	fake_time = 20;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);

	// Regreso a S0 reinicia contador
	fsm_run(&fsm);
	setUp(); // resetea fake_time y contadores
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_add_event_timeout(&fsm, trans, 20);

	// Sin esperar 20ms: no debe transicionar
	fsm_run(&fsm);
	fake_time = 15;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(0, exit_s0_cnt);
}

void test_multiple_timeouts_choose_earliest(void) {
	fsm_t fsm;
	fsm_trans_t *t1 = NULL, *t2 = NULL;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_register_state_actions(&fsm, STATE_S2, cb_enter_s2, NULL, NULL);
	// S0->S1 en 30ms, S0->S2 en 60ms
	fsm_add_transition(&fsm, &t1, STATE_S0, STATE_S1);
	fsm_add_event_timeout(&fsm, t1, 60);
	fsm_add_transition(&fsm, &t2, STATE_S0, STATE_S2);
	fsm_add_event_timeout(&fsm, t2, 30);

	// Avanzar a 35ms: debe ir a S1
	fsm_run(&fsm);
	fake_time = 60;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
	TEST_ASSERT_EQUAL_INT(0, enter_s2_cnt);
}

void test_large_time_jump(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	fsm_init(&fsm, STATE_S0, get_fake_time);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, NULL);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	fsm_add_event_timeout(&fsm, trans, 50);

	// Salto de 200ms: debe transicionar
	fsm_run(&fsm);
	fake_time = 200;
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
}

void test_timeout_without_time_fn_does_not_crash(void) {
	fsm_t fsm;
	fsm_trans_t *trans = NULL;
	// Inicializar sin función de tiempo
	fsm_init(&fsm, STATE_S0, NULL);
	fsm_register_state_actions(&fsm, STATE_S0, cb_enter_s0, NULL, cb_exit_s0);
	fsm_register_state_actions(&fsm, STATE_S1, cb_enter_s1, NULL, cb_exit_s1);
	fsm_add_transition(&fsm, &trans, STATE_S0, STATE_S1);
	// Intentar añadir timeout
	fsm_add_event_timeout(&fsm, trans, 50);

	// Ejecutar: no debe colgar ni transicionar
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, exit_s0_cnt);
	fsm_run(&fsm);
	TEST_ASSERT_EQUAL_INT(1, enter_s1_cnt);
}

/* Main ----------------------------------------------------------------------*/
int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_timeout_transition_only);
	RUN_TEST(test_and_transition_with_timeout);
	RUN_TEST(test_or_transition_with_timeout);
	RUN_TEST(test_timeout_reset_on_reenter_state);
	RUN_TEST(test_multiple_timeouts_choose_earliest);
	RUN_TEST(test_large_time_jump);
	RUN_TEST(test_timeout_without_time_fn_does_not_crash);
	return UNITY_END();
}

/* Private function definitions ----------------------------------------------*/

/***************************** END OF FILE ************************************/
