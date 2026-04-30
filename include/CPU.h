#ifndef CPU_H
#define CPU_H

#include <cstddef>
#include <vector>

#include "ALU.h"
#include "Instruction.h"
#include "Memory.h"
#include "Registers.h"

class CPU {
public:
    CPU(std::size_t memorySize = 1024, std::size_t registerCount = 8);

    void loadProgram(const std::vector<Instruction>& instructions);
    void run();
    bool isHalted() const;

private:
    void step();
    void execute(const Instruction& instruction);
    void printState(const Instruction& instruction, std::size_t executedPc) const;

    std::vector<Instruction> program;
    Registers registers;
    Memory memory;
    ALU alu;
    std::size_t programCounter;
    std::size_t cycle;
    bool halted;
    bool zeroFlag;
};

#endif
