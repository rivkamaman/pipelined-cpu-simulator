#include <iostream>

#include "Assembler.h"
#include "CPU.h"

int main() {
    // Create a CPU with the default memory and register sizes.
    CPU cpu;

    // Assemble the text program into Instruction objects for the CPU.
    const auto program = Assembler::assembleFile("program.asm");

    // Load and run the sample program, printing a trace after each instruction.
    cpu.loadProgram(program);
    cpu.run();

    std::cout << "CPU simulation finished." << std::endl;
    return 0;
}
