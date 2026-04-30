#ifndef CPU_H
#define CPU_H

#include <cstddef>
#include <vector>

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
    void printTrace(const Instruction& instruction, std::size_t executedPc) const;

    Memory memory;
    Registers registers;
    std::vector<Instruction> program;
    std::size_t programCounter;
    std::size_t cycle;
    bool halted;
    bool zeroFlag;
};

#endif
