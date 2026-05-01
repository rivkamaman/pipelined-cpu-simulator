#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include "Instruction.h"

enum class ALUOp {
    NONE,
    ADD,
    SUB,
    CMP
};

enum class BranchType {
    NONE,
    JZ,
    JNZ
};

struct ControlSignals {
    bool regWrite = false;
    bool memRead = false;
    bool memWrite = false;
    bool isBranch = false;
    bool isJump = false;
    bool halt = false;
    ALUOp aluOp = ALUOp::NONE;
    BranchType branchType = BranchType::NONE;
};

class ControlUnit {
public:
    static ControlSignals decode(const Instruction& instruction);
};

#endif
