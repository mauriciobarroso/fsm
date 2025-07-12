# FSM

A lightweight C library for building and executing finite state machines with minimal overhead. Easily define states, transitions, composite events, and associate entry, update, and exit actions for each state.

## Features

* Flat state machine model (no nested states by default)
* Composite events combining comparisons and logical operators (AND/OR)
* Supports Mealy, Moore, and mixed state outputs
* Small memory footprint (minimal dynamic allocations)
* Built‑in internal timeout events for delay‑driven transitions

## Examples

* Multi‑function button handler (single, double, long press)
* Debounce filter with timeout confirmation
* Generic GPIO‑driven state transitions

## How to use

A quick guide to set up a simple FSM with two states (`IDLE`, `RUNNING`), a timeout transition, and a completion flag.

1. **Define FSM states**

   ```c
   typedef enum {
     STATE_IDLE = 0,
     STATE_RUNNING
   } state_t;
   ```

2. **Declare FSM instance and event variables**

   ```c
   fsm_t fsm;
   uint32_t get_time_ms(void) { return 0; /* dummy time source */ }
   int done_flag = 0;
   ```

3. **Initialize the FSM**

   ```c
   fsm_init(&fsm, STATE_IDLE, get_time_ms);
   ```

4. **Add transitions with their events**

   * **IDLE → RUNNING** on a 100 ms timeout

     ```c
     fsm_trans_t *t;
     fsm_add_transition(&fsm, &t, STATE_IDLE, STATE_RUNNING);
     fsm_add_event_timeout(&fsm, t, 100);
     ```
   * **RUNNING → IDLE** when `done_flag == 1`

     ```c
     fsm_add_transition(&fsm, &t, STATE_RUNNING, STATE_IDLE);
     fsm_add_event_cmp(&fsm, t, &done_flag, 1, eval_eq);
     ```

5. **Register state action callbacks**

   ```c
   void on_enter_idle(void)    { /* reset variables */ }
   void on_update_idle(void)   { /* poll sensors or inputs */ }
   void on_enter_running(void) { /* start task */ }
   void on_exit_running(void)  { /* cleanup */ }

   fsm_register_state_actions(&fsm, STATE_IDLE,
                              on_enter_idle,
                              on_update_idle,
                              NULL);
   fsm_register_state_actions(&fsm, STATE_RUNNING,
                              on_enter_running,
                              NULL,
                              on_exit_running);
   ```

6. **Run the FSM in a loop**

   ```c
   while (1) {
     fsm_run(&fsm);
     delay_ms(10); /* dummy wait */
   }
   ```

## Roadmap

* **Hierarchical states** (nested/forked substates)
* **Parallel regions** (orthogonal state machines)
* **State history** (shallow and deep history semantics)
* **Event deferral** and external event queues
* **Mermaid or PlantUML exporter** for auto‑generated diagrams
* **Thread safety** enhancements and reentrancy checks
* **Expanded examples**: traffic light, alarm system, protocol parser

## License

MIT License

Copyright (c) 2025 Mauricio Barroso Benavides

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
