#ifndef TRACE_PRINTER_H
#define TRACE_PRINTER_H

#include <cstddef>
#include <string>
#include <vector>

#include "Instruction.h"
#include "PipelineTrace.h"
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

    // Print the IF/ID/EX/MEM/WB pipeline history table.
    static void printPipelineTrace(const std::vector<PipelineTrace>& traceHistory);
};

#endif
