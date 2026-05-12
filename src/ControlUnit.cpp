#include "ControlUnit.h"

ControlSignals ControlUnit::decode(const Instruction& instruction) {
    ControlSignals signals;

    // Decode is the only opcode-dependent stage in the CPU.
    switch (instruction.opcode) {
        case Opcode::ADD:
            signals.regWrite = true;
            signals.aluOp = ALUOp::ADD;
            break;
        case Opcode::ADDI:
            signals.regWrite = true;
            signals.aluOp = ALUOp::ADDI;
            break;
        case Opcode::SUB:
            signals.regWrite = true;
            signals.aluOp = ALUOp::SUB;
            break;
        case Opcode::AND:
            signals.regWrite = true;
            signals.aluOp = ALUOp::AND;
            break;
        case Opcode::OR:
            signals.regWrite = true;
            signals.aluOp = ALUOp::OR;
            break;
        case Opcode::LW:
            signals.memRead = true;
            signals.regWrite = true;
            break;
        case Opcode::SW:
            signals.memWrite = true;
            break;
        case Opcode::BEQ:
            signals.isBranch = true;
            signals.branchType = BranchType::BEQ;
            break;
        case Opcode::BNE:
            signals.isBranch = true;
            signals.branchType = BranchType::BNE;
            break;
        case Opcode::J:
            signals.isJump = true;
            break;
        case Opcode::NOP:
            break;
        case Opcode::HALT:
            signals.halt = true;
            break;
    }

    return signals;
}
