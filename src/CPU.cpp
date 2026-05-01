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
    // Loading a program also resets all CPU execution state.
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

std::int32_t CPU::getRegisterValue(std::size_t index) const {
    return registers.read(index);
}

bool CPU::getZeroFlag() const {
    return zeroFlag;
}

void CPU::step() {
    // Running past the program acts like reaching HALT.
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
    // Fetch increments the PC so jumps and branches can overwrite it later.
    currentInstruction = program[programCounter];
    ++programCounter;
}

void CPU::decode() {
    // The control unit centralizes opcode-specific decisions.
    currentSignals = ControlUnit::decode(currentInstruction);
}

void CPU::execute() {
    // From here down, execution follows control signals instead of opcodes.
    const std::size_t src1 = static_cast<std::size_t>(currentInstruction.getSrc1());
    const std::size_t src2 = static_cast<std::size_t>(currentInstruction.getSrc2());
    const int immediate = currentInstruction.getImmediate();

    if (currentSignals.halt) {
        halted = true;
        return;
    }

    if (currentSignals.memRead) {
        registers.write(
            static_cast<std::size_t>(currentInstruction.dst),
            memory.read(static_cast<std::size_t>(immediate))
        );
        return;
    }

    if (currentSignals.memWrite) {
        memory.write(
            static_cast<std::size_t>(immediate),
            registers.read(src1)
        );
        return;
    }

    if (currentSignals.isJump) {
        programCounter = static_cast<std::size_t>(immediate);
        return;
    }

    if (currentSignals.isBranch) {
        if (currentSignals.branchType == BranchType::JZ && zeroFlag) {
            programCounter = static_cast<std::size_t>(immediate);
        }

        if (currentSignals.branchType == BranchType::JNZ && !zeroFlag) {
            programCounter = static_cast<std::size_t>(immediate);
        }

        return;
    }

    // MOV reaches this path with ALUOp::NONE and regWrite set.
    if (currentSignals.aluOp == ALUOp::NONE && !currentSignals.regWrite) {
        return;
    }

    // Immediate is the default write-back value for MOV.
    int result = immediate;

    switch (currentSignals.aluOp) {
        case ALUOp::ADD:
            result = alu.add(
                registers.read(src1),
                registers.read(src2)
            );
            break;
        case ALUOp::SUB:
            result = alu.sub(
                registers.read(src1),
                registers.read(src2)
            );
            break;
        case ALUOp::CMP:
            zeroFlag = alu.equal(
                registers.read(src1),
                registers.read(src2)
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
