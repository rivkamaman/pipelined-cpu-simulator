# Pipelined CPU Simulator

A cycle-accurate MIPS-like RISC CPU simulator written in C++.

This project models both sequential and real pipelined CPU execution while simulating core computer architecture concepts such as hazard handling, data forwarding, stalls, flushes, speculative execution, and dynamic branch prediction.

---

# Features

- Sequential CPU execution mode
- Real 5-stage pipelined execution (`IF -> ID -> EX -> MEM -> WB`)
- Signal-driven datapath execution
- Modular hazard handling units
- Dynamic branch prediction with BTB
- Custom MIPS-like ISA
- Cycle-by-cycle pipeline tracing
- CPU execution statistics

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

# Architecture

- `ControlUnit::decode()` centralizes control signal generation.
- `Instruction` exposes semantic helper methods for:
  - register reads/writes
  - memory access behavior
  - control-flow behavior
  - instruction format classification
- CPU execution is signal-driven instead of relying on opcode checks spread across the pipeline.
- ALU behavior is separated from pipeline/control logic.
- Hazard handling is modularized into dedicated units.

Pipeline-related units:
- `HazardUnit` detects RAW dependencies
- `ForwardingUnit` resolves RAW hazards through bypassing
- `StallUnit` inserts load-use bubbles
- `ControlHazardUnit` handles branch/jump recovery and flushes
- `BranchPredictor` implements BTB-based dynamic prediction
- `CPUStatistics` tracks execution metrics

---

# Execution Modes

## 1. Sequential Execution

- Executes one instruction completely before starting the next.
- Useful for correctness validation and debugging.
- Uses:
  - `CPU::run()`
  - `CPU::step()`

---

## 2. Pipelined Execution

Uses:
- `CPU::runPipelined()`
- `CPU::stepPipelined()`

Pipeline stages:

1. IF — Instruction Fetch
2. ID — Decode / Register Fetch
3. EX — Execute / Address / Branch Resolution
4. MEM — Memory Access
5. WB — Register Write Back

Pipeline registers:

```text
IF/ID
ID/EX
EX/MEM
MEM/WB
```

The simulator computes next-stage state before committing pipeline registers, approximating real hardware clock behavior.

---

# Hazard Handling

## Data Hazards

Handled using:
- RAW hazard detection
- EX/MEM forwarding
- MEM/WB forwarding
- Load-use stalls

Forwarding paths:

```text
EX/MEM -> EX
MEM/WB -> EX
```

Load-use hazards insert a one-cycle bubble:
- `IFID` is frozen
- PC fetch is stalled
- `IDEX` receives a bubble/NOP

---

## Control Hazards

Handled using:
- EX-stage branch resolution
- Pipeline flushing
- Program counter redirection
- Branch prediction recovery

Mispredicted branches/jumps flush incorrect younger instructions while older pipeline work drains normally.

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

Branch prediction occurs during fetch and is validated during EX-stage branch resolution.

---

# Memory Addressing

`LW` and `SW` use MIPS-like base+offset addressing:

```asm
LW R1,4(R2)
SW R3,8(R4)
```

Address calculation:

```text
effective_address = base_register + offset
```

For stores:
- `src1` = value register
- `src2` = base register

---

# Example Program

```asm
ADDI R1,R0,7
ADDI R2,R0,3
ADD R3,R1,R2
ADDI R4,R0,10

BEQ R3,R4,equal

ADDI R5,R0,-1
J done

equal:
SW R3,10(R0)
LW R6,10(R0)

done:
HALT
```

---

# Statistics

The simulator tracks:
- Total cycles
- Completed instructions
- CPI
- Stall count
- Flush count
- Forwarding events
- Branch prediction accuracy

---

# Project Structure

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

# Testing

Tests are implemented using `assert` and include:
- assembler parsing
- sequential execution
- pipelined execution
- forwarding correctness
- load-use stalls
- branch/jump flushing
- BTB prediction behavior
- HALT pipeline drain behavior

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
