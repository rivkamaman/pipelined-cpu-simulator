#include "TracePrinter.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

void TracePrinter::printCycle(
    std::size_t cycle,
    std::size_t pc,
    const Instruction& instruction,
    const Registers& registers,
    bool zeroFlag
) {
    // Print the cycle header and decoded instruction text.
    std::cout << "Cycle " << cycle
              << " | PC=" << pc
              << " | " << instruction.toString()
              << '\n';

    // Print every register so state changes are visible cycle by cycle.
    for (std::size_t index = 0; index < registers.count(); ++index) {
        std::cout << "R" << index << "=" << registers.read(index);
        if (index + 1 < registers.count()) {
            std::cout << ' ';
        }
    }

    std::cout << " | ZF=" << (zeroFlag ? 1 : 0) << "\n\n";
}

void TracePrinter::printPipelineTrace(const std::vector<PipelineTrace>& traceHistory) {
    // Start with header widths, then widen each column to fit trace contents.
    std::size_t cycleWidth = 5;
    std::size_t fetchWidth = 5;
    std::size_t decodeWidth = 6;
    std::size_t executeWidth = 7;
    std::size_t memoryWidth = 3;
    std::size_t writeBackWidth = 2;

    for (std::size_t index = 0; index < traceHistory.size(); ++index) {
        cycleWidth = std::max(cycleWidth, std::to_string(index).size());
        fetchWidth = std::max(fetchWidth, traceHistory[index].fetch.size());
        decodeWidth = std::max(decodeWidth, traceHistory[index].decode.size());
        executeWidth = std::max(executeWidth, traceHistory[index].execute.size());
        memoryWidth = std::max(memoryWidth, traceHistory[index].memory.size());
        writeBackWidth = std::max(writeBackWidth, traceHistory[index].writeBack.size());
    }

    const std::size_t totalWidth =
        cycleWidth + fetchWidth + decodeWidth + executeWidth + memoryWidth + writeBackWidth + 15;

    std::cout << std::left
              << std::setw(static_cast<int>(cycleWidth)) << "Cycle"
              << " | "
              << std::setw(static_cast<int>(fetchWidth)) << "IF"
              << " | "
              << std::setw(static_cast<int>(decodeWidth)) << "ID"
              << " | "
              << std::setw(static_cast<int>(executeWidth)) << "EX"
              << " | "
              << std::setw(static_cast<int>(memoryWidth)) << "MEM"
              << " | "
              << std::setw(static_cast<int>(writeBackWidth)) << "WB"
              << '\n';
    std::cout << std::string(totalWidth, '-') << '\n';

    for (std::size_t index = 0; index < traceHistory.size(); ++index) {
        const PipelineTrace& trace = traceHistory[index];

        std::cout << std::left
                  << std::setw(static_cast<int>(cycleWidth)) << index
                  << " | "
                  << std::setw(static_cast<int>(fetchWidth)) << trace.fetch
                  << " | "
                  << std::setw(static_cast<int>(decodeWidth)) << trace.decode
                  << " | "
                  << std::setw(static_cast<int>(executeWidth)) << trace.execute
                  << " | "
                  << std::setw(static_cast<int>(memoryWidth)) << trace.memory
                  << " | "
                  << std::setw(static_cast<int>(writeBackWidth)) << trace.writeBack
                  << '\n';
    }

    std::cout << '\n';
}
