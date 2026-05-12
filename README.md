# Simple RISC CPU Simulator

C++ project implementing a small RISC-style CPU with:
- Sequential execution mode
- Real pipelined execution mode (`IF -> ID -> EX -> MEM -> WB`)
- Signal-driven datapath execution (opcode decoding is centralized in `ControlUnit`)

## Features

- Instruction set:
  - `ADD`, `ADDI`, `SUB`, `AND`, `OR`
  - `LW`, `SW`
  - `BEQ`, `BNE`, `J`
  - `NOP`, `HALT`
- Assembler:
  - Parses `.asm` text into `Instruction` objects
  - Supports labels with two-pass parsing
- Trace output:
  - Sequential mode prints cycle/register state and pipeline-style trace
  - Pipelined mode prints real per-cycle pipeline state:
    - `Cycle | IF | ID | EX | MEM | WB`
  - Pipeline runs also print a final CPU statistics summary with cycles, completed instructions, CPI, stalls, flushes, and forwarding events

## Architecture

- `ControlUnit::decode()` centralizes control signal generation.
- `Instruction` exposes semantic helpers such as register read/write and memory/control-flow behavior.
- CPU execute behavior is signal-driven.
- ALU is separate from control and CPU flow logic.
- Hazard logic is split into focused units:
  - `HazardUnit` detects RAW dependencies for diagnostics
  - `ForwardingUnit` resolves ALU/register RAW hazards by bypassing from `EXMEM` or `MEMWB`
  - `StallUnit` inserts a one-cycle bubble for load-use hazards
  - `ControlHazardUnit` resolves jumps/branches and requests pipeline flushes
  - `CPUStatistics` collects execution counters during pipelined runs

## Execution Modes

### 1. Sequential mode

- Uses `CPU::run()` / `CPU::step()`.
- Executes one instruction per step.
- Existing sequential behavior remains unchanged.

### 2. Pipelined mode (5-stage)

- Uses `CPU::runPipelined()` / `CPU::stepPipelined()`.
- Stages:
  1. IF fetches instruction
  2. ID decodes control signals
  3. EX computes ALU/address/branch decisions
  4. MEM performs memory access
  5. WB writes register results

### Current pipeline constraints

- RAW hazards are detected and printed as warnings.
- ALU/register RAW hazards are handled with forwarding from `EXMEM` and `MEMWB`.
- Load-use hazards are handled with a one-cycle stall:
  - `IFID` is frozen
  - PC is frozen by skipping fetch
  - a bubble is inserted into `IDEX`
- Branch and jump prediction uses a small BTB with 2-bit counters in pipelined mode.
- Mispredicted jumps/branches are handled with EX-stage flush:
  - wrong-path `IFID` / `IDEX` work is cleared
  - older `EXMEM` / `MEMWB` work drains normally

Forwarding does not remove every possible delay: an immediate consumer after `LW` still needs the automatic load-use stall because loaded data is not available until after MEM.

### Pipeline statistics

At the end of a pipelined trace, the simulator prints:

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

Tests are implemented with `assert` and include:
- assembler parsing checks
- sequential execution checks
- pipelined execution checks (ALU, LW/SW, forwarding, load-use stalls, branch/jump flush, BTB prediction, HALT drain)
