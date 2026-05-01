#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Instruction operation codes supported by the simulator.
enum class Opcode {
    MOV,
    ADD,
    SUB,
    LOAD,
    STORE,
    CMP,
    JMP,
    JZ,
    JNZ,
    HALT
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
};

#endif
