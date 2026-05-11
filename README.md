# Pipelined CPU Simulator

A cycle-accurate MIPS-like RISC CPU simulator written in C++.

This project simulates both sequential execution and real 5-stage pipelined execution while modeling core computer architecture concepts such as pipeline registers, hazards, forwarding, stalls, flushes, branch prediction, and BTB-based control flow prediction.

---

# Features

- Sequential CPU execution mode
- Real pipelined CPU execution mode
- 5-stage pipeline architecture
- Hazard detection
- Data forwarding
- Load-use stalls
- Pipeline flushing
- Dynamic branch prediction
- Branch Target Buffer (BTB)
- Custom MIPS-like ISA
- Cycle-by-cycle pipeline tracing
- CPU performance statistics

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

Run tests:

```bash
make test
```

Clean build files:

```bash
make clean
```

---

# Project Architecture

The project is organized around a modular CPU simulation architecture.

```text
src/
├── CPU.cpp
├── CPUSequential.cpp
├── CPUPipeline.cpp
├── BTB.cpp
├── CPUStatistics.cpp
├── ControlUnit.cpp
├── HazardUnit.cpp
├── StallUnit.cpp
├── ForwardingUnit.cpp
├── Assembler.cpp
├── TracePrinter.cpp
└── main.cpp
```

---

# Execution Modes

## Sequential Mode

Executes one instruction completely before starting the next.

Useful for:
- Functional validation
- Instruction debugging
- Comparing against pipelined execution

---

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

Pipeline registers:

```text
IF/ID
ID/EX
EX/MEM
MEM/WB
```

Each cycle computes the next pipeline state and updates all pipeline registers simultaneously, similar to real hardware timing behavior.

---

# Instruction Set

The simulator uses a custom MIPS-like ISA.

## Arithmetic / Logic

- `ADD`
- `ADDI`
- `SUB`
- `AND`
- `OR`

## Memory

- `LW`
- `SW`

## Branch / Control Flow

- `BEQ`
- `BNE`
- `J`

## Legacy Compatibility Instructions

Older custom instructions are still supported internally for backward compatibility:

- `MOV`
- `LOAD`
- `STORE`
- `CMP`
- `JMP`
- `JZ`
- `JNZ`

## Misc

- `NOP`
- `HALT`

---

# Control Signals

Instructions are decoded into control signals that determine pipeline behavior.

Example:

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

This design keeps execution logic modular and hardware-inspired.

---

# Hazard Handling

## Data Hazards

The simulator handles data hazards using:

- Hazard Detection Unit
- Forwarding Unit
- Load-use stalls
- Register dependency checks

Supported forwarding paths:

```text
EX/MEM → EX
MEM/WB → EX
```

Branch instructions (`BEQ` / `BNE`) also support operand forwarding for correct comparisons during EX stage execution.

---

## Control Hazards

Control hazards are handled using:

- Branch resolution in EX stage
- Program counter redirection
- Pipeline flushing
- Branch prediction recovery

---

# Branch Prediction & BTB

The pipelined CPU includes:

- Dynamic 2-bit branch predictor
- Branch Target Buffer (BTB)
- Prediction tracking
- Misprediction recovery

The BTB predicts future control flow targets during instruction fetch to reduce pipeline stalls caused by branches.

Statistics tracked include:

- Total branch predictions
- Correct predictions
- Mispredictions
- Branch prediction accuracy
- Flushes caused by incorrect predictions

---

# Pipeline Trace Output

The simulator prints a cycle-by-cycle pipeline trace.

Example:

```text
Cycle | IF | ID | EX | MEM | WB
--------------------------------
1     | ADD
2     | SUB | ADD
3     | LW  | SUB | ADD
```

This makes it easier to visualize instruction-level parallelism and pipeline behavior.

---

# Statistics

At the end of execution, the simulator reports:

- Total cycles
- Completed instructions
- CPI
- Number of stalls
- Number of flushes
- Forwarding events
- Branch prediction statistics

---

# Example Program

```asm
ADDI R0,R0,7
ADDI R1,R0,3
ADD R2,R0,R1
ADDI R3,R0,10

BEQ R2,R3,equal

ADDI R4,R0,-1
J done

equal:
SW R2,10(R0)
LW R5,10(R0)

done:
HALT
```

---

# Future Improvements

Possible future extensions:

- Proper base+offset memory execution
- Hardwired zero register (`R0`)
- Cache simulation
- Instruction encoding / binary loading
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
- BTB design
- Low-level system simulation
- Hardware-inspired software architecture
