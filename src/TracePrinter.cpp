#include "TracePrinter.h"

#include <iostream>
#include <sstream>

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
              << " | " << instructionToString(instruction)
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

std::string TracePrinter::instructionToString(const Instruction& instruction) {
    std::ostringstream output;

    // Trace formatting is presentation-only; execution logic lives elsewhere.
    switch (instruction.opcode) {
        case Opcode::MOV:
            output << "MOV R" << instruction.dst << ", " << instruction.immediate;
            break;
        case Opcode::ADD:
            output << "ADD R" << instruction.dst
                   << ", R" << instruction.src1
                   << ", R" << instruction.src2;
            break;
        case Opcode::SUB:
            output << "SUB R" << instruction.dst
                   << ", R" << instruction.src1
                   << ", R" << instruction.src2;
            break;
        case Opcode::LOAD:
            output << "LOAD R" << instruction.dst << ", [" << instruction.immediate << "]";
            break;
        case Opcode::STORE:
            output << "STORE R" << instruction.src1 << ", [" << instruction.immediate << "]";
            break;
        case Opcode::CMP:
            output << "CMP R" << instruction.src1
                   << ", R" << instruction.src2;
            break;
        case Opcode::JMP:
            output << "JMP " << instruction.immediate;
            break;
        case Opcode::JZ:
            output << "JZ " << instruction.immediate;
            break;
        case Opcode::JNZ:
            output << "JNZ " << instruction.immediate;
            break;
        case Opcode::HALT:
            output << "HALT";
            break;
    }

    return output.str();
}
