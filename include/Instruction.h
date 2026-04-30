#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>

enum class Opcode {
    NOP,
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
    std::uint8_t destination;
    std::uint8_t source1;
    std::uint8_t source2;
    std::int32_t immediate;

    Instruction(
        Opcode opcode = Opcode::NOP,
        std::uint8_t destination = 0,
        std::uint8_t source1 = 0,
        std::uint8_t source2 = 0,
        std::int32_t immediate = 0
    );

    std::string toString() const;
};

#endif
