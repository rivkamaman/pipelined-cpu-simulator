#include "ControlUnit.h"

ControlSignals ControlUnit::decode(const Instruction& instruction) {
    ControlSignals signals;

    // Decode is the only opcode-dependent stage in the CPU.
    switch (instruction.opcode) {
        case Opcode::MOV:
            signals.regWrite = true;
            break;
        case Opcode::ADD:
            signals.regWrite = true;
            signals.aluOp = ALUOp::ADD;
            break;
        case Opcode::SUB:
            signals.regWrite = true;
            signals.aluOp = ALUOp::SUB;
            break;
        case Opcode::LOAD:
            signals.memRead = true;
            signals.regWrite = true;
            break;
        case Opcode::STORE:
            signals.memWrite = true;
            break;
        case Opcode::CMP:
            signals.aluOp = ALUOp::CMP;
            break;
        case Opcode::JMP:
            signals.isJump = true;
            break;
        case Opcode::JZ:
            signals.isBranch = true;
            signals.branchType = BranchType::JZ;
            break;
        case Opcode::JNZ:
            signals.isBranch = true;
            signals.branchType = BranchType::JNZ;
            break;
        case Opcode::HALT:
            signals.halt = true;
            break;
    }

    return signals;
}
