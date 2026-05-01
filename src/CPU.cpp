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
    currentSignals = ControlUnit::decode(currentInstruction);
}

void CPU::execute() {
    if (currentSignals.halt) {
        halted = true;
        return;
    }

    if (currentSignals.memRead) {
        registers.write(
            static_cast<std::size_t>(currentInstruction.dst),
            memory.read(static_cast<std::size_t>(currentInstruction.immediate))
        );
        return;
    }

    if (currentSignals.memWrite) {
        memory.write(
            static_cast<std::size_t>(currentInstruction.immediate),
            registers.read(static_cast<std::size_t>(currentInstruction.src1))
        );
        return;
    }

    if (currentSignals.isJump) {
        programCounter = static_cast<std::size_t>(currentInstruction.immediate);
        return;
    }

    if (currentSignals.isBranch) {
        if (currentSignals.branchType == BranchType::JZ && zeroFlag) {
            programCounter = static_cast<std::size_t>(currentInstruction.immediate);
        }

        if (currentSignals.branchType == BranchType::JNZ && !zeroFlag) {
            programCounter = static_cast<std::size_t>(currentInstruction.immediate);
        }

        return;
    }

    if (currentSignals.aluOp == ALUOp::NONE && !currentSignals.regWrite) {
        return;
    }

    int result = currentInstruction.immediate;

    switch (currentSignals.aluOp) {
        case ALUOp::ADD:
            result = alu.add(
                registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                registers.read(static_cast<std::size_t>(currentInstruction.src2))
            );
            break;
        case ALUOp::SUB:
            result = alu.sub(
                registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                registers.read(static_cast<std::size_t>(currentInstruction.src2))
            );
            break;
        case ALUOp::CMP:
            zeroFlag = alu.equal(
                registers.read(static_cast<std::size_t>(currentInstruction.src1)),
                registers.read(static_cast<std::size_t>(currentInstruction.src2))
            );
            break;
        case ALUOp::NONE:
            break;
    }

    if (currentSignals.regWrite) {
        registers.write(static_cast<std::size_t>(currentInstruction.dst), result);
    }
}

void CPU::printState(std::size_t executedPc) const {
    TracePrinter::printCycle(cycle, executedPc, currentInstruction, registers, zeroFlag);
}
