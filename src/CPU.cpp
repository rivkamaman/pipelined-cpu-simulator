#include "CPU.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

Instruction::Instruction(
    Opcode opcode,
    std::uint8_t destination,
    std::uint8_t source1,
    std::uint8_t source2,
    std::int32_t immediate
)
    : opcode(opcode),
      destination(destination),
      source1(source1),
      source2(source2),
      immediate(immediate) {
}

std::string Instruction::toString() const {
    std::ostringstream output;

    switch (opcode) {
        case Opcode::NOP:
            output << "NOP";
            break;
        case Opcode::MOV:
            output << "MOV R" << static_cast<int>(destination) << ", " << immediate;
            break;
        case Opcode::ADD:
            output << "ADD R" << static_cast<int>(destination)
                   << ", R" << static_cast<int>(source1)
                   << ", R" << static_cast<int>(source2);
            break;
        case Opcode::SUB:
            output << "SUB R" << static_cast<int>(destination)
                   << ", R" << static_cast<int>(source1)
                   << ", R" << static_cast<int>(source2);
            break;
        case Opcode::LOAD:
            output << "LOAD R" << static_cast<int>(destination) << ", [" << immediate << "]";
            break;
        case Opcode::STORE:
            output << "STORE R" << static_cast<int>(source1) << ", [" << immediate << "]";
            break;
        case Opcode::CMP:
            output << "CMP R" << static_cast<int>(source1)
                   << ", R" << static_cast<int>(source2);
            break;
        case Opcode::JMP:
            output << "JMP " << immediate;
            break;
        case Opcode::JZ:
            output << "JZ " << immediate;
            break;
        case Opcode::JNZ:
            output << "JNZ " << immediate;
            break;
        case Opcode::HALT:
            output << "HALT";
            break;
    }

    return output.str();
}

CPU::CPU(std::size_t memorySize, std::size_t registerCount)
    : memory(memorySize),
      registers(registerCount),
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
    const Instruction instruction = program[programCounter++];
    execute(instruction);
    ++cycle;
    printTrace(instruction, executedPc);
}

void CPU::execute(const Instruction& instruction) {
    switch (instruction.opcode) {
        case Opcode::NOP:
            break;
        case Opcode::MOV:
            registers.write(instruction.destination, instruction.immediate);
            break;
        case Opcode::ADD:
            registers.write(
                instruction.destination,
                registers.read(instruction.source1) + registers.read(instruction.source2)
            );
            break;
        case Opcode::SUB:
            registers.write(
                instruction.destination,
                registers.read(instruction.source1) - registers.read(instruction.source2)
            );
            break;
        case Opcode::LOAD:
            registers.write(instruction.destination, memory.read(static_cast<std::size_t>(instruction.immediate)));
            break;
        case Opcode::STORE:
            memory.write(static_cast<std::size_t>(instruction.immediate), registers.read(instruction.source1));
            break;
        case Opcode::CMP:
            zeroFlag = registers.read(instruction.source1) == registers.read(instruction.source2);
            break;
        case Opcode::JMP:
            programCounter = static_cast<std::size_t>(instruction.immediate);
            break;
        case Opcode::JZ:
            if (zeroFlag) {
                programCounter = static_cast<std::size_t>(instruction.immediate);
            }
            break;
        case Opcode::JNZ:
            if (!zeroFlag) {
                programCounter = static_cast<std::size_t>(instruction.immediate);
            }
            break;
        case Opcode::HALT:
            halted = true;
            break;
        default:
            throw std::runtime_error("Unknown opcode");
    }
}

void CPU::printTrace(const Instruction& instruction, std::size_t executedPc) const {
    std::cout << "Cycle " << cycle
              << " | PC=" << executedPc
              << " | " << instruction.toString()
              << '\n';

    for (std::size_t index = 0; index < registers.count(); ++index) {
        std::cout << "R" << index << "=" << registers.read(index);
        if (index + 1 < registers.count()) {
            std::cout << ' ';
        }
    }

    std::cout << " | ZF=" << (zeroFlag ? 1 : 0) << "\n\n";
}
