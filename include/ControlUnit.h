#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include "Instruction.h"

// ALU operation selected during decode.
enum class ALUOp {
    NONE,
    ADD,
    SUB,
    CMP
};

// Conditional branch behavior selected during decode.
enum class BranchType {
    NONE,
    JZ,
    JNZ
};

// Control lines produced by the control unit for one instruction.
struct ControlSignals {
    // Write the computed/read value into the destination register.
    bool regWrite = false;

    // Read from data memory using the instruction immediate as the address.
    bool memRead = false;

    // Write to data memory using the instruction immediate as the address.
    bool memWrite = false;

    // Treat the instruction as a conditional branch.
    bool isBranch = false;

    // Treat the instruction as an unconditional jump.
    bool isJump = false;

    // Stop the CPU after this instruction.
    bool halt = false;

    // ALU operation to perform, if any.
    ALUOp aluOp = ALUOp::NONE;

    // Branch condition to test, if this is a branch instruction.
    BranchType branchType = BranchType::NONE;
};

// Converts opcodes into control signals used by the CPU datapath.
class ControlUnit {
public:
    // Decode one instruction. This is the only component that checks opcodes.
    static ControlSignals decode(const Instruction& instruction);
};

#endif
