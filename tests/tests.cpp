#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Assembler.h"
#include "CPU.h"

void testAssemblerMovParsing() {
    const Instruction instruction = Assembler::parseLine("MOV R2 42");

    assert(instruction.opcode == Opcode::MOV);
    assert(instruction.dst == 2);
    assert(instruction.getImmediate() == 42);
}

void testAssemblerAddParsing() {
    const Instruction instruction = Assembler::parseLine("ADD R2 R0 R1");

    assert(instruction.opcode == Opcode::ADD);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerAddiParsing() {
    const Instruction instruction = Assembler::parseLine("ADDI R2 R0 5");

    assert(instruction.opcode == Opcode::ADDI);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getImmediate() == 5);
}

void testAssemblerAndParsing() {
    const Instruction instruction = Assembler::parseLine("AND R3 R0 R1");

    assert(instruction.opcode == Opcode::AND);
    assert(instruction.dst == 3);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerOrParsing() {
    const Instruction instruction = Assembler::parseLine("OR R4 R0 R1");

    assert(instruction.opcode == Opcode::OR);
    assert(instruction.dst == 4);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerBranchParsing() {
    const Instruction instruction = Assembler::parseLine("JZ 8");

    assert(instruction.opcode == Opcode::JZ);
    assert(instruction.getImmediate() == 8);
}

void testAssemblerLabelBranchParsing() {
    const std::vector<std::string> lines = {
        "MOV R0 7",
        "JZ done",
        "MOV R1 -1",
        "done:",
        "HALT"
    };

    const std::vector<Instruction> program = Assembler::assembleLines(lines);

    assert(program.size() == 4);
    assert(program[1].opcode == Opcode::JZ);
    assert(program[1].getImmediate() == 3);
}

void testAssemblerAssembleLines() {
    const std::vector<std::string> lines = {
        "MOV R0 5",
        "MOV R1 3",
        "ADD R2 R0 R1",
        "",
        "# comment-only line",
        "HALT"
    };

    const std::vector<Instruction> program = Assembler::assembleLines(lines);

    assert(program.size() == 4);
    assert(program[0].opcode == Opcode::MOV);
    assert(program[1].opcode == Opcode::MOV);
    assert(program[2].opcode == Opcode::ADD);
    assert(program[3].opcode == Opcode::HALT);
}

void testAssemblerInvalidOpcodeThrows() {
    bool threw = false;

    try {
        Assembler::parseLine("BAD R0 1");
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAssemblerProgramRunsOnCpu() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "MOV R1 3",
        "ADD R2 R0 R1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
}

void testAssemblerAddSubFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/add_sub.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
    assert(cpu.getRegisterValue(3) == 2);
}

void testAssemblerLoadStoreFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/load_store.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 42);
    assert(cpu.getRegisterValue(1) == 42);
}

void testAssemblerLabelsFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/labels.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(2) == 42);
}

void testAssemblerUnknownLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"JMP missing", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAssemblerDuplicateLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"again:", "MOV R0 1", "again:", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAddSub() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 5},     // R0 = 5
        {Opcode::MOV, 1, 0, 0, 3},     // R1 = 3
        {Opcode::ADD, 2, 0, 1, 0},     // R2 = 8
        {Opcode::SUB, 3, 0, 1, 0},     // R3 = 2
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
    assert(cpu.getRegisterValue(3) == 2);
}

void testJZBranchTaken() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 7},
        {Opcode::MOV, 1, 0, 0, 7},
        {Opcode::CMP, 0, 1, 0, 0},     // zeroFlag = true
        {Opcode::JZ, 0, 0, 0, 5},      // jump to HALT
        {Opcode::MOV, 2, 0, 0, 99},    // skipped
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getZeroFlag() == true);
    assert(cpu.getRegisterValue(2) == 0);
}

void testJNZBranchTaken() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 7},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::CMP, 0, 1, 0, 0},     // zeroFlag = false
        {Opcode::JNZ, 0, 0, 0, 5},     // jump to HALT
        {Opcode::MOV, 2, 0, 0, 99},    // skipped
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getZeroFlag() == false);
    assert(cpu.getRegisterValue(2) == 0);
}

void testLoadStore() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 42},    // R0 = 42
        {Opcode::STORE, 0, 0, 0, 10},  // memory[10] = R0
        {Opcode::LOAD, 1, 0, 0, 10},   // R1 = memory[10]
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(1) == 42);
}

void testAddi() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 10},
        {Opcode::ADDI, 1, 0, 0, 5},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(1) == 15);
}

void testAnd() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 6},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::AND, 2, 0, 1, 0},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(2) == 2);
}

void testOr() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 6},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::OR, 2, 0, 1, 0},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(2) == 7);
}

int main() {
    testAssemblerMovParsing();
    testAssemblerAddParsing();
    testAssemblerAddiParsing();
    testAssemblerAndParsing();
    testAssemblerOrParsing();
    testAssemblerBranchParsing();
    testAssemblerLabelBranchParsing();
    testAssemblerAssembleLines();
    testAssemblerInvalidOpcodeThrows();
    testAssemblerProgramRunsOnCpu();
    testAssemblerAddSubFileRunsOnCpu();
    testAssemblerLoadStoreFileRunsOnCpu();
    testAssemblerLabelsFileRunsOnCpu();
    testAssemblerUnknownLabelThrows();
    testAssemblerDuplicateLabelThrows();

    testAddSub();
    testJZBranchTaken();
    testJNZBranchTaken();
    testLoadStore();
    testAddi();
    testAnd();
    testOr();

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
