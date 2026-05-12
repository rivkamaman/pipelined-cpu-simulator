#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

// Instruction operation codes supported by the simulator.
enum class Opcode {
    ADD,
    ADDI,
    SUB,
    AND,
    OR,
    LW,
    SW,
    BEQ,
    BNE,
    J,
    NOP,
    HALT
};

enum class InstructionFormat {
    RType,
    IType,
    JType,
    Special
};

// A simple fixed-format instruction.
struct Instruction {
    // Operation selected by the program.
    Opcode opcode;

    // Destination register index, used by register-writing instructions.
    int dst;

    // First source register index.
    int src1;

    // Second source register index.
    int src2;

    // Immediate value, used for constants, memory addresses, and jump targets.
    int immediate;

    // Build an instruction. Defaults to HALT for a safe empty instruction.
    Instruction(
        Opcode opcode = Opcode::HALT,
        int dst = 0,
        int src1 = 0,
        int src2 = 0,
        int immediate = 0
    );

    // Return the first source register index.
    int getSrc1() const;

    // Return the second source register index.
    int getSrc2() const;

    // Return the immediate field used for constants, addresses, and targets.
    int getImmediate() const;

    // Semantic helpers used by pipeline units so opcode behavior is centralized.
    bool writesRegister() const;
    bool readsSrc1() const;
    bool readsSrc2() const;
    bool isBranch() const;
    bool isJump() const;
    bool isMemoryRead() const;
    bool isMemoryWrite() const;
    InstructionFormat getFormat() const;

    // Convert the instruction into readable assembly-like text.
    std::string toString() const;
};

#endif
