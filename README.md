# FSM

## Features

* Flat state machine model (no nested states by default)
* Composite events combining comparisons and logical operators (AND/OR)
* Supports Mealy, Moore, and mixed state outputs
* Small memory footprint (minimal dynamic allocations)
* Built‑in internal timeout events for delay‑driven transitions

## Examples

* [Multi‑function button](examples/multi_function_button/)

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
