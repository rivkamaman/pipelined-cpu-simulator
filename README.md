# Simple RISC CPU Simulator

C++ project implementing a small RISC-style CPU with:
- Sequential execution mode
- Real pipelined execution mode (`IF -> ID -> EX -> MEM -> WB`)
- Signal-driven datapath execution (opcode decoding is centralized in `ControlUnit`)

## Features

- Instruction set:
  - `MOV`, `ADD`, `ADDI`, `SUB`, `AND`, `OR`
  - `LOAD`, `STORE`
  - `CMP`, `JMP`, `JZ`, `JNZ`
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

- `ControlUnit::decode()` is the only opcode-dependent stage.
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
- Taken jumps/branches are handled with EX-stage flush:
  - wrong-path `IFID` / `IDEX` work is cleared
  - older `EXMEM` / `MEMWB` work drains normally
- No branch prediction

Forwarding does not remove every possible delay: an immediate consumer after `LOAD` still needs the automatic load-use stall because loaded data is not available until after MEM.

### Pipeline statistics

At the end of a pipelined trace, the simulator prints:

```text
=== CPU Statistics ===
Cycles:       10
Instructions: 4
CPI:          2.50
Stalls:       1
Flushes:      0
Forwardings:  2
```

## Build and Run

- `make run` — build and run main program
- `make test` — build and run test suite

## Testing

Tests are implemented with `assert` and include:
- assembler parsing checks
- sequential execution checks
- pipelined execution checks (ALU, LOAD/STORE, forwarding, load-use stalls, branch/jump flush, HALT drain)
