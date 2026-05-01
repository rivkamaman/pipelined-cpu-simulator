#include "CPU.h"

#include "TracePrinter.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace {

std::string stageOrEmptyMarker(const std::string& stage) {
    return stage.empty() ? "-" : stage;
}

} // namespace

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
    traceHistory.clear();
    pipelineFetchStage.clear();
    pipelineDecodeStage.clear();
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
    // Start with header widths, then widen each column to fit trace contents.
    std::size_t cycleWidth = 5;
    std::size_t fetchWidth = 5;
    std::size_t decodeWidth = 6;
    std::size_t executeWidth = 7;

    for (std::size_t index = 0; index < traceHistory.size(); ++index) {
        cycleWidth = std::max(cycleWidth, std::to_string(index).size());
        fetchWidth = std::max(fetchWidth, traceHistory[index].fetch.size());
        decodeWidth = std::max(decodeWidth, traceHistory[index].decode.size());
        executeWidth = std::max(executeWidth, traceHistory[index].execute.size());
    }

    const std::size_t totalWidth =
        cycleWidth + fetchWidth + decodeWidth + executeWidth + 9;

    std::cout << std::left
              << std::setw(static_cast<int>(cycleWidth)) << "Cycle"
              << " | "
              << std::setw(static_cast<int>(fetchWidth)) << "Fetch"
              << " | "
              << std::setw(static_cast<int>(decodeWidth)) << "Decode"
              << " | "
              << std::setw(static_cast<int>(executeWidth)) << "Execute"
              << '\n';
    std::cout << std::string(totalWidth, '-') << '\n';

    for (std::size_t index = 0; index < traceHistory.size(); ++index) {
        const PipelineTrace& trace = traceHistory[index];

        std::cout << std::left
                  << std::setw(static_cast<int>(cycleWidth)) << index
                  << " | "
                  << std::setw(static_cast<int>(fetchWidth)) << trace.fetch
                  << " | "
                  << std::setw(static_cast<int>(decodeWidth)) << trace.decode
                  << " | "
                  << std::setw(static_cast<int>(executeWidth)) << trace.execute
                  << '\n';
    }

    std::cout << '\n';
}

void CPU::step() {
    // Running past the program acts like reaching HALT.
    if (programCounter >= program.size()) {
        halted = true;
        return;
    }

    const std::size_t executedPc = programCounter;
    // Keep the previous cycle's stage names so the trace can show pipeline flow.
    const std::string previousFetchStage = pipelineFetchStage;
    const std::string previousDecodeStage = pipelineDecodeStage;
    PipelineTrace trace;

    fetch();
    trace.fetch = currentInstruction.toString();

    decode();
    trace.decode = stageOrEmptyMarker(previousFetchStage);

    execute();
    trace.execute = stageOrEmptyMarker(previousDecodeStage);

    traceHistory.push_back(trace);
    pipelineDecodeStage = previousFetchStage;
    pipelineFetchStage = trace.fetch;

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
        // Conditional branches update the PC only when the zero flag matches.
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
        case ALUOp::ADDI:
            result = alu.addImmediate(
                registers.read(src1),
                immediate
            );
            break;
        case ALUOp::SUB:
            result = alu.sub(
                registers.read(src1),
                registers.read(src2)
            );
            break;
        case ALUOp::AND:
            result = alu.bitwiseAnd(
                registers.read(src1),
                registers.read(src2)
            );
            break;
        case ALUOp::OR:
            result = alu.bitwiseOr(
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

void CPU::flushPipelineTrace() {
    // After HALT, add cycles for instructions already shown in fetch/decode.
    while (!pipelineFetchStage.empty() || !pipelineDecodeStage.empty()) {
        PipelineTrace trace;
        trace.fetch = "-";
        trace.decode = stageOrEmptyMarker(pipelineFetchStage);
        trace.execute = stageOrEmptyMarker(pipelineDecodeStage);

        traceHistory.push_back(trace);
        pipelineDecodeStage = pipelineFetchStage;
        pipelineFetchStage.clear();
    }
}
