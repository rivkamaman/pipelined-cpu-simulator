#include "CPU.h"

#include "TracePrinter.h"

CPU::CPU(std::size_t memorySize, std::size_t registerCount)
    : registers(registerCount),
      memory(memorySize),
      programCounter(0),
      cycle(0),
      halted(false),
      zeroFlag(false) {
}

void CPU::loadProgram(const std::vector<Instruction>& instructions) {
    program = instructions;
    programCounter = 0;
    cycle = 0;
    halted = false;
    zeroFlag = false;
}

void CPU::run() {
    while (!halted) {
        step();
    }
}

bool CPU::isHalted() const {
    return halted;
}

void CPU::step() {
    if (programCounter >= program.size()) {
        halted = true;
        return;
    }

    const std::size_t executedPc = programCounter;

    fetch();
    decode();
    execute();
    ++cycle;
    printState(executedPc);
}

void CPU::fetch() {
    currentInstruction = program[programCounter];
    ++programCounter;
}

void CPU::decode() {
}

void CPU::execute() {
    switch (currentInstruction.opcode) {
        case Opcode::MOV:
            registers.write(static_cast<std::size_t>(currentInstruction.dst), currentInstruction.immediate);
            break;
        case Opcode::ADD:
            registers.write(
                static_cast<std::size_t>(currentInstruction.dst),
                alu.add(
                    registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                    registers.read(static_cast<std::size_t>(currentInstruction.src2))
                )
            );
            break;
        case Opcode::SUB:
            registers.write(
                static_cast<std::size_t>(currentInstruction.dst),
                alu.sub(
                    registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                    registers.read(static_cast<std::size_t>(currentInstruction.src2))
                )
            );
            break;
        case Opcode::LOAD:
            registers.write(
                static_cast<std::size_t>(currentInstruction.dst),
                memory.read(static_cast<std::size_t>(currentInstruction.immediate))
            );
            break;
        case Opcode::STORE:
            memory.write(
                static_cast<std::size_t>(currentInstruction.immediate),
                registers.read(static_cast<std::size_t>(currentInstruction.src1))
            );
            break;
        case Opcode::CMP:
            zeroFlag = alu.equal(
                registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                registers.read(static_cast<std::size_t>(currentInstruction.src2))
            );
            break;
        case Opcode::JMP:
            programCounter = static_cast<std::size_t>(currentInstruction.immediate);
            break;
        case Opcode::JZ:
            if (zeroFlag) {
                programCounter = static_cast<std::size_t>(currentInstruction.immediate);
            }
            break;
        case Opcode::JNZ:
            if (!zeroFlag) {
                programCounter = static_cast<std::size_t>(currentInstruction.immediate);
            }
            break;
        case Opcode::HALT:
            halted = true;
            break;
    }
}

void CPU::printState(std::size_t executedPc) const {
    TracePrinter::printCycle(cycle, executedPc, currentInstruction, registers, zeroFlag);
}
