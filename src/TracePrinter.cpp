#include "TracePrinter.h"

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
