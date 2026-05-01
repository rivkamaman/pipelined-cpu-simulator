#ifndef TRACE_PRINTER_H
#define TRACE_PRINTER_H

#include <cstddef>
#include <string>

#include "Instruction.h"
#include "Registers.h"

// Formats and prints the simulator trace after each CPU cycle.
class TracePrinter {
public:
    // Print one cycle, including PC, instruction text, registers, and zero flag.
    static void printCycle(
        std::size_t cycle,
        std::size_t pc,
        const Instruction& instruction,
        const Registers& registers,
        bool zeroFlag
    );
};

#endif
