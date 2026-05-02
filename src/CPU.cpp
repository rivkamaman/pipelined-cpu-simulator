#include "CPU.h"

#include "TracePrinter.h"

CPU::CPU(std::size_t memorySize, std::size_t registerCount)
    : registers(registerCount),
      memory(memorySize),
      programCounter(0),
      cycle(0),
      halted(false),
      fetchHalted(false),
      pipelineFlushRequested(false),
      zeroFlag(false) {
}

void CPU::loadProgram(const std::vector<Instruction>& instructions) {
    // Loading a program also resets all CPU execution state.
    program = instructions;
    programCounter = 0;
    cycle = 0;
    halted = false;
    fetchHalted = false;
    pipelineFlushRequested = false;
    zeroFlag = false;
    stats.reset();
    traceHistory.clear();
    pipelineFetchStage.clear();
    pipelineDecodeStage.clear();
    ifid = IFID{};
    idex = IDEX{};
    exmem = EXMEM{};
    memwb = MEMWB{};
}

void CPU::run() {
    while (!halted) {
        step();
    }

    flushPipelineTrace();
    printPipelineTrace();
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

void CPU::printPipelineTrace() const {
    TracePrinter::printPipelineTrace(traceHistory);
    if (stats.hasCycles()) {
        printStats();
    }
}

void CPU::printState(std::size_t executedPc) const {
    TracePrinter::printCycle(cycle, executedPc, currentInstruction, registers, zeroFlag);
}

void CPU::printStats() const {
    stats.print();
}
