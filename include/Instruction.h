#ifndef INSTRUCTION_H
#define INSTRUCTION_H

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

struct Instruction {
    Opcode opcode;
    int dst;
    int src1;
    int src2;
    int immediate;

    Instruction(
        Opcode opcode = Opcode::HALT,
        int dst = 0,
        int src1 = 0,
        int src2 = 0,
        int immediate = 0
    );
};

#endif
