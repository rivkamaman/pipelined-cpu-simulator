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

## Architecture

- `ControlUnit::decode()` is the only opcode-dependent stage.
- CPU execute behavior is signal-driven.
- ALU is separate from control and CPU flow logic.

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

- No hazard detection
- No forwarding
- No stalls
- No branch prediction
- Branch/jump control hazards are handled by EX-stage flush of younger stages

Because there is no automatic data hazard handling, tests/programs should use `NOP` spacing between dependent instructions.

## Build and Run

- `make run` — build and run main program
- `make test` — build and run test suite

## Testing

Tests are implemented with `assert` and include:
- assembler parsing checks
- sequential execution checks
- pipelined execution checks (ALU, LOAD/STORE, branch flush, HALT drain)
