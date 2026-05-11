# Pipelined CPU Simulator

A cycle-accurate RISC-style CPU simulator written in C++.

This project simulates both sequential execution and real 5-stage pipelined execution, focusing on core computer architecture concepts such as pipeline registers, hazards, forwarding, stalls, flushes, and branch prediction.

---

# How to Run

Build the project:

```bash
make
```

Run the simulator:

```bash
./cpu_simulator
```

Clean build files:

```bash
make clean
```

---

# Project Architecture

The project is organized around a CPU simulation core, an assembler, pipeline execution logic, and debugging/statistics utilities.

```text
src/
├── CPU.cpp
├── CPUPipeline.cpp
├── ControlUnit.cpp
├── HazardUnit.cpp
├── ForwardingUnit.cpp
├── Assembler.cpp
├── TracePrinter.cpp
└── main.cpp
```

---

# Execution Modes

## Sequential Mode

Executes one instruction completely before starting the next.

This mode is useful for:
- Validating instruction behavior
- Debugging programs
- Comparing results against pipelined execution

## Pipelined Mode

Simulates a real 5-stage CPU pipeline:

```text
IF → ID → EX → MEM → WB
```

Where:

- IF — Instruction Fetch
- ID — Instruction Decode / Register Fetch
- EX — Execute / ALU
- MEM — Memory Access
- WB — Write Back

---

# Pipeline Design

The simulator uses explicit pipeline registers:

```text
IF/ID
ID/EX
EX/MEM
MEM/WB
```

Each cycle computes the next state of the pipeline and updates all pipeline registers together, similar to real hardware timing behavior.

Execution behavior is controlled using decoded control signals rather than placing opcode-specific logic throughout the pipeline.

---

# Control Signals

Each instruction is decoded into control signals such as:

```cpp
struct ControlSignals {
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool isBranch;
    bool isJump;
    ALUOp aluOp;
};
```

These signals determine which actions are active in each stage.

For example:
- An arithmetic instruction enables ALU execution and register write-back.
- A load instruction enables memory read and register write-back.
- A store instruction enables memory write but does not write back to a register.
- A branch or jump instruction may redirect the program counter and flush younger instructions.

---

# Supported Instructions

## Arithmetic / Logic

- `MOV`
- `ADD`
- `ADDI`
- `SUB`
- `AND`
- `OR`
- `CMP`

## Memory

- `LOAD`
- `STORE`

## Control Flow

- `JMP`
- `JZ`
- `JNZ`

## Misc

- `NOP`
- `HALT`

---

# Hazard Handling

## Data Hazards

The simulator handles data hazards using:

- Hazard detection
- Forwarding
- Load-use stalls
- Register dependency checks

Supported forwarding paths:

```text
EX/MEM → EX
MEM/WB → EX
```

## Control Hazards

Control hazards are handled using:

- Branch resolution
- Program counter redirection
- Pipeline flushing
- Branch prediction recovery

---

# Branch Prediction

The pipelined CPU includes a dynamic 2-bit branch predictor.

The simulator tracks:
- Branch predictions
- Correct predictions
- Mispredictions
- Branch prediction accuracy
- Flushes caused by incorrect predictions

---

# Trace Output

The simulator prints a cycle-by-cycle pipeline trace.

Example:

```text
Cycle | IF | ID | EX | MEM | WB
--------------------------------
1     | ADD
2     | SUB | ADD
3     | LOAD| SUB | ADD
```

This makes it easier to observe how instructions move through the pipeline.

---

# Statistics

At the end of execution, the simulator reports:

- Total cycles
- Completed instructions
- CPI
- Number of stalls
- Number of flushes
- Forwarding events
- Branch prediction accuracy

---

# Future Improvements

Possible extensions:

- Cache simulation
- Superscalar execution
- Out-of-order execution
- Tomasulo algorithm
- Scoreboarding
- Multi-level branch prediction
- Graph-based pipeline visualization
- Verilog implementation for hardware comparison

---

# Learning Goals

This project was built to explore:

- Computer architecture
- CPU pipeline execution
- Instruction-level parallelism
- Data hazards
- Control hazards
- Forwarding
- Branch prediction
- Low-level system simulation
- Hardware-inspired software design
