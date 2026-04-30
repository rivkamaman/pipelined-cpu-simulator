#ifndef TRACE_PRINTER_H
#define TRACE_PRINTER_H

#include <cstddef>
#include <string>

#include "Instruction.h"
#include "Registers.h"

class TracePrinter {
public:
    static void printCycle(
        std::size_t cycle,
        std::size_t pc,
        const Instruction& instruction,
        const Registers& registers,
        bool zeroFlag
    );

private:
    static std::string instructionToString(const Instruction& instruction);
};

#endif
