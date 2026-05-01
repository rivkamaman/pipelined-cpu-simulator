#ifndef CPU_H
#define CPU_H

#include <cstddef>
#include <vector>

#include "ALU.h"
#include "ControlUnit.h"
#include "Instruction.h"
#include "Memory.h"
#include "Registers.h"

// CPU ties together fetch, decode, execute, registers, memory, and the ALU.
class CPU {
public:
    // Create a CPU with configurable data memory and register file sizes.
    CPU(std::size_t memorySize = 1024, std::size_t registerCount = 8);

    // Load a new program and reset execution state.
    void loadProgram(const std::vector<Instruction>& instructions);

    // Execute instructions until HALT or the program counter leaves the program.
    void run();

    // Report whether the CPU has stopped.
    bool isHalted() const;

private:
    // Execute one full fetch-decode-execute cycle.
    void step();

    // Copy the next program instruction into currentInstruction.
    void fetch();

    // Produce control signals for currentInstruction.
    void decode();

    // Drive the datapath using currentSignals.
    void execute();

    // Print the trace line for the instruction that just ran.
    void printState(std::size_t executedPc) const;

    // Instruction memory for this simple simulator.
    std::vector<Instruction> program;

    // Instruction currently moving through the CPU cycle.
    Instruction currentInstruction;

    // Control signals generated for currentInstruction.
    ControlSignals currentSignals;

    // CPU datapath components and execution state.
    Registers registers;
    Memory memory;
    ALU alu;
    std::size_t programCounter;
    std::size_t cycle;
    bool halted;
    bool zeroFlag;
};

#endif
