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

bool Instruction::writesRegister() const {
    switch (opcode) {
        case Opcode::ADD:
        case Opcode::ADDI:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::LW:
            return true;
        default:
            return false;
    }
}

bool Instruction::readsSrc1() const {
    switch (opcode) {
        case Opcode::ADD:
        case Opcode::ADDI:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::LW:
        case Opcode::SW:
        case Opcode::BEQ:
        case Opcode::BNE:
            return true;
        default:
            return false;
    }
}

bool Instruction::readsSrc2() const {
    switch (opcode) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::SW:
        case Opcode::BEQ:
        case Opcode::BNE:
            return true;
        default:
            return false;
    }
}

bool Instruction::isBranch() const {
    return opcode == Opcode::BEQ || opcode == Opcode::BNE;
}

bool Instruction::isJump() const {
    return opcode == Opcode::J;
}

bool Instruction::isMemoryRead() const {
    return opcode == Opcode::LW;
}

bool Instruction::isMemoryWrite() const {
    return opcode == Opcode::SW;
}

InstructionFormat Instruction::getFormat() const {
    switch (opcode) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
            return InstructionFormat::RType;
        case Opcode::ADDI:
        case Opcode::LW:
        case Opcode::SW:
        case Opcode::BEQ:
        case Opcode::BNE:
            return InstructionFormat::IType;
        case Opcode::J:
            return InstructionFormat::JType;
        case Opcode::NOP:
        case Opcode::HALT:
            return InstructionFormat::Special;
    }

    return InstructionFormat::Special;
}

std::string Instruction::toString() const {
    std::ostringstream output;

    // Recreate readable assembly text for execution and pipeline traces.
    switch (opcode) {
        case Opcode::ADD:
            output << "ADD R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::ADDI:
            output << "ADDI R" << dst << ",R" << src1 << "," << immediate;
            break;
        case Opcode::SUB:
            output << "SUB R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::AND:
            output << "AND R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::OR:
            output << "OR R" << dst << ",R" << src1 << ",R" << src2;
            break;
        case Opcode::LW:
            output << "LW R" << dst << "," << immediate << "(R" << src1 << ")";
            break;
        case Opcode::SW:
            output << "SW R" << src1 << "," << immediate << "(R" << src2 << ")";
            break;
        case Opcode::BEQ:
            output << "BEQ R" << src1 << ",R" << src2 << "," << immediate;
            break;
        case Opcode::BNE:
            output << "BNE R" << src1 << ",R" << src2 << "," << immediate;
            break;
        case Opcode::J:
            output << "J " << immediate;
            break;
        case Opcode::NOP:
            output << "NOP";
            break;
        case Opcode::HALT:
            output << "HALT";
            break;
    }

    return output.str();
}
