#include <iostream>
#include <vector>

#include "CPU.h"

int main() {
    CPU cpu;

    const std::vector<Instruction> program = {
        Instruction(Opcode::MOV, 0, 0, 0, 7),
        Instruction(Opcode::MOV, 1, 0, 0, 3),
        Instruction(Opcode::ADD, 2, 0, 1),
        Instruction(Opcode::MOV, 3, 0, 0, 8),
        Instruction(Opcode::CMP, 0, 2, 3),
        Instruction(Opcode::JZ, 0, 0, 0, 8),
        Instruction(Opcode::MOV, 4, 0, 0, -1),
        Instruction(Opcode::JMP, 0, 0, 0, 9),
        Instruction(Opcode::STORE, 0, 2, 0, 10),
        Instruction(Opcode::LOAD, 5, 0, 0, 10),
        Instruction(Opcode::HALT)
    };

    cpu.loadProgram(program);
    cpu.run();

    std::cout << "CPU simulation finished." << std::endl;
    return 0;
}
