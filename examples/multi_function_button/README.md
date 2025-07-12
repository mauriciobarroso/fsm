# Multi-Function Button Example

This example shows how to implement a multi-function button handler using a finite state machine (FSM). It supports:

- A configurable **debounce** mechanism to eliminate contact bounce  
- Detection of **single click**, **double click**, and **long press**  
- Customizable callback functions for each event  
- Simple integration in a periodic loop via `fsm_run`  
- A self-contained demonstration with no external dependencies beyond the FSM core  

## FSM diagram
```mermaid
stateDiagram-v2
  direction TB
  [*] --> IDLE
  IDLE --> DEBOUNCE:!GPIO
  DEBOUNCE --> IDLE:TIMEOUT & GPIO
  DEBOUNCE --> PRESSED:TIMEOUT & !GPIO
  PRESSED --> RELEASED:GPIO
  RELEASED --> SINGLE:TIMEOUT
  PRESSED --> LONG:TIMEOUT
  RELEASED --> DOUBLE:!GPIO
  SINGLE --> IDLE
  DOUBLE --> IDLE:GPIO
  LONG --> IDLE:GPIO
```

## States transition list
| Current  | Next     | Events           |
| -------- | -------- | ---------------- |
| IDLE     | DEBOUNCE | !GPIO            |
| DEBOUNCE | IDLE     | TIMEOUT & GPIO   |
| DEBOUNCE | PRESSED  | TIMEOUT & !GPIO  |
| PRESSED  | RELEASED | GPIO             |
| PRESSED  | LONG     | TIMEOUT          |
| RELEASED | SINGLE   | TIMEOUT          |
| RELEASED | DOUBLE   | !GPIO            |
| SINGLE   | IDLE     | (auto–immediate) |
| DOUBLE   | IDLE     | GPIO             |
| LONG     | IDLE     | GPIO             |

## States actions
| State    | Entry                 | Update          | Exit |
| -------- | --------------------- | --------------- | ---- |
| IDLE     | N/A                   | Read GPIO level | N/A  |
| DEBOUNCE | N/A                   | Read GPIO level | N/A  |
| PRESSED  | N/A                   | Read GPIO level | N/A  |
| RELEASED | N/A                   | Read GPIO level | N/A  |
| SINGLE   | Print "Single Click!" | N/A             | N/A  |
| DOUBLE   | Print "Double Click!" | Read GPIO level | N/A  |
| LONG     | Print "Long Click!"   | Read GPIO level | N/A  |

## License
MIT License

Copyright (c) 2025 Mauricio Barroso Benavides

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
