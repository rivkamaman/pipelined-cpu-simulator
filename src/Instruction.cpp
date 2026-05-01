#include "Instruction.h"

#include <sstream>

Instruction::Instruction(
    Opcode opcode,
    int dst,
    int src1,
    int src2,
    int immediate
)
    // Store the fixed instruction fields exactly as supplied by the program.
    : opcode(opcode),
      dst(dst),
      src1(src1),
      src2(src2),
      immediate(immediate) {
}

int Instruction::getSrc1() const {
    return src1;
}

int Instruction::getSrc2() const {
    return src2;
}

int Instruction::getImmediate() const {
    return immediate;
}

std::string Instruction::toString() const {
    std::ostringstream output;

    // Recreate readable assembly text for execution and pipeline traces.
    switch (opcode) {
        case Opcode::MOV:
            output << "MOV R" << dst << "," << immediate;
            break;
        case Opcode::ADD:
            output << "ADD R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::SUB:
            output << "SUB R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::LOAD:
            output << "LOAD R" << dst << "," << immediate;
            break;
        case Opcode::STORE:
            output << "STORE R" << src1 << "," << immediate;
            break;
        case Opcode::CMP:
            output << "CMP R" << src1 << ",R" << src2;
            break;
        case Opcode::JMP:
            output << "JMP " << immediate;
            break;
        case Opcode::JZ:
            output << "JZ " << immediate;
            break;
        case Opcode::JNZ:
            output << "JNZ " << immediate;
            break;
        case Opcode::HALT:
            output << "HALT";
            break;
    }

    return output.str();
}
