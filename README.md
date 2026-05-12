# Pipelined CPU Simulator

A cycle-accurate MIPS-like RISC CPU simulator written in C++.

This project models both sequential and real pipelined CPU execution while simulating core computer architecture concepts such as hazards, forwarding, stalls, flushes, speculative execution, and dynamic branch prediction.

---

# Features

- Sequential and pipelined CPU execution
- 5-stage pipeline (`IF → ID → EX → MEM → WB`)
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

Build:

```bash
make
```

Run:

```bash
./cpu_simulator
```

Run tests:

```bash
make test
```

---

# Project Architecture

```text
src/
├── CPU.cpp
├── CPUSequential.cpp
├── CPUPipeline.cpp
├── BranchPredictor.cpp
├── ControlUnit.cpp
├── HazardUnit.cpp
├── StallUnit.cpp
├── ForwardingUnit.cpp
├── Assembler.cpp
├── TracePrinter.cpp
└── main.cpp
```

---

# Instruction Set

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

## Misc

- `NOP`
- `HALT`

---

# Hazard Handling

## Data Hazards

Handled using:
- Hazard Detection Unit
- Forwarding Unit
- Load-use stalls

Forwarding paths:

```text
EX/MEM → EX
MEM/WB → EX
```

## Control Hazards

Handled using:
- Branch resolution in EX stage
- Pipeline flushing
- Branch prediction recovery

---

# Branch Prediction & BTB

The simulator includes:
- Dynamic 2-bit branch predictor
- Branch Target Buffer (BTB)
- Speculative instruction fetch
- Misprediction recovery

Prediction states:

```text
0 = Strongly Not Taken
1 = Weakly Not Taken
2 = Weakly Taken
3 = Strongly Taken
```

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

# Statistics

The simulator tracks:
- Total cycles
- CPI
- Stalls
- Flushes
- Forwarding events
- Branch prediction accuracy

---

# Future Improvements

- Proper base+offset memory execution
- Hardwired zero register (`R0`)
- Cache simulation
- Instruction encoding
- Superscalar execution
- Out-of-order execution
- Verilog implementation for hardware comparison
