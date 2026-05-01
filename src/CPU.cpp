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

void CPU::runPipelined() {
    while (!fetchHalted || ifid.valid || idex.valid || exmem.valid || memwb.valid) {
        stepPipelined();
    }

    halted = true;
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
    std::size_t memoryWidth = 3;
    std::size_t writeBackWidth = 2;

    for (std::size_t index = 0; index < traceHistory.size(); ++index) {
        cycleWidth = std::max(cycleWidth, std::to_string(index).size());
        fetchWidth = std::max(fetchWidth, traceHistory[index].fetch.size());
        decodeWidth = std::max(decodeWidth, traceHistory[index].decode.size());
        executeWidth = std::max(executeWidth, traceHistory[index].execute.size());
        memoryWidth = std::max(memoryWidth, traceHistory[index].memory.size());
        writeBackWidth = std::max(writeBackWidth, traceHistory[index].writeBack.size());
    }

    const std::size_t totalWidth =
        cycleWidth + fetchWidth + decodeWidth + executeWidth + memoryWidth + writeBackWidth + 15;

    std::cout << std::left
              << std::setw(static_cast<int>(cycleWidth)) << "Cycle"
              << " | "
              << std::setw(static_cast<int>(fetchWidth)) << "IF"
              << " | "
              << std::setw(static_cast<int>(decodeWidth)) << "ID"
              << " | "
              << std::setw(static_cast<int>(executeWidth)) << "EX"
              << " | "
              << std::setw(static_cast<int>(memoryWidth)) << "MEM"
              << " | "
              << std::setw(static_cast<int>(writeBackWidth)) << "WB"
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
                  << " | "
                  << std::setw(static_cast<int>(memoryWidth)) << trace.memory
                  << " | "
                  << std::setw(static_cast<int>(writeBackWidth)) << trace.writeBack
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
    trace.memory = "-";
    trace.writeBack = "-";

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
    executeWithSignals(currentInstruction, currentSignals, nullptr);
}

void CPU::stepPipelined() {
    PipelineTrace trace;
    trace.fetch = "-";
    trace.decode = "-";
    trace.execute = "-";
    trace.memory = "-";
    trace.writeBack = "-";

    // A) WB
    if (memwb.valid) {
        trace.writeBack = memwb.instruction.toString();
    }
    writeBackStage(memwb);

    // B) MEM
    if (exmem.valid) {
        trace.memory = exmem.instruction.toString();
    }
    MEMWB nextMemwb = memoryStage(exmem);

    // C) EX
    if (idex.valid) {
        trace.execute = idex.instruction.toString();
    }
    EXMEM nextExmem = executeStage(idex);

    // D) ID
    IDEX nextIdex;
    if (!pipelineFlushRequested) {
        if (ifid.valid) {
            trace.decode = ifid.instruction.toString();
        }
        nextIdex = decodeStage(ifid);
    }

    // E) IF
    IFID nextIfid;
    if (!pipelineFlushRequested) {
        nextIfid = fetchStage();
        if (nextIfid.valid) {
            trace.fetch = nextIfid.instruction.toString();
        }
    }

    ifid = nextIfid;
    idex = nextIdex;
    exmem = nextExmem;
    memwb = nextMemwb;

    pipelineFlushRequested = false;
    traceHistory.push_back(trace);
    ++cycle;
}

IFID CPU::fetchStage() {
    IFID output;
    if (fetchHalted) {
        return output;
    }

    if (programCounter >= program.size()) {
        fetchHalted = true;
        return output;
    }

    output.valid = true;
    output.pc = programCounter;
    output.instruction = program[programCounter];
    ++programCounter;
    return output;
}

IDEX CPU::decodeStage(const IFID& input) {
    IDEX output;
    if (!input.valid) {
        return output;
    }

    output.valid = true;
    output.pc = input.pc;
    output.instruction = input.instruction;
    output.signals = ControlUnit::decode(input.instruction);
    return output;
}

EXMEM CPU::executeStage(const IDEX& input) {
    EXMEM output;
    if (!input.valid) {
        return output;
    }

    output.valid = true;
    output.pc = input.pc;
    output.instruction = input.instruction;
    output.signals = input.signals;

    const std::size_t src1 = static_cast<std::size_t>(input.instruction.getSrc1());
    const std::size_t src2 = static_cast<std::size_t>(input.instruction.getSrc2());
    const int immediate = input.instruction.getImmediate();

    if (input.signals.halt) {
        fetchHalted = true;
        return output;
    }

    if (input.signals.isJump) {
        programCounter = static_cast<std::size_t>(immediate);
        pipelineFlushRequested = true;
        output.valid = false;
        return output;
    }

    if (input.signals.isBranch) {
        bool taken = false;
        if (input.signals.branchType == BranchType::JZ && zeroFlag) {
            taken = true;
        }
        if (input.signals.branchType == BranchType::JNZ && !zeroFlag) {
            taken = true;
        }

        if (taken) {
            programCounter = static_cast<std::size_t>(immediate);
            pipelineFlushRequested = true;
        }

        output.valid = false;
        return output;
    }

    if (input.signals.memRead || input.signals.memWrite) {
        output.aluResult = immediate;
        output.storeData = registers.read(src1);
        return output;
    }

    if (input.signals.aluOp == ALUOp::NONE && !input.signals.regWrite) {
        return output;
    }

    int result = immediate;
    switch (input.signals.aluOp) {
        case ALUOp::ADD:
            result = alu.add(registers.read(src1), registers.read(src2));
            break;
        case ALUOp::ADDI:
            result = alu.addImmediate(registers.read(src1), immediate);
            break;
        case ALUOp::SUB:
            result = alu.sub(registers.read(src1), registers.read(src2));
            break;
        case ALUOp::AND:
            result = alu.bitwiseAnd(registers.read(src1), registers.read(src2));
            break;
        case ALUOp::OR:
            result = alu.bitwiseOr(registers.read(src1), registers.read(src2));
            break;
        case ALUOp::CMP:
            zeroFlag = alu.equal(registers.read(src1), registers.read(src2));
            break;
        case ALUOp::NONE:
            break;
    }

    output.aluResult = result;

    // CMP does not write back.
    if (input.signals.aluOp == ALUOp::CMP) {
        output.valid = false;
    }

    return output;
}

MEMWB CPU::memoryStage(const EXMEM& input) {
    MEMWB output;
    if (!input.valid) {
        return output;
    }

    output.valid = true;
    output.pc = input.pc;
    output.instruction = input.instruction;
    output.signals = input.signals;

    if (input.signals.memRead) {
        output.writeBackData = memory.read(static_cast<std::size_t>(input.aluResult));
        return output;
    }

    if (input.signals.memWrite) {
        memory.write(static_cast<std::size_t>(input.aluResult), input.storeData);
        output.valid = false;
        return output;
    }

    output.writeBackData = input.aluResult;
    return output;
}

void CPU::writeBackStage(const MEMWB& input) {
    if (!input.valid) {
        return;
    }

    if (input.signals.regWrite) {
        registers.write(
            static_cast<std::size_t>(input.instruction.dst),
            input.writeBackData
        );
    }
}

void CPU::executeWithSignals(
    const Instruction& instruction,
    const ControlSignals& signals,
    bool* controlTransferTaken
) {
    if (controlTransferTaken != nullptr) {
        *controlTransferTaken = false;
    }

    // From here down, execution follows control signals instead of opcodes.
    const std::size_t src1 = static_cast<std::size_t>(instruction.getSrc1());
    const std::size_t src2 = static_cast<std::size_t>(instruction.getSrc2());
    const int immediate = instruction.getImmediate();

    if (signals.halt) {
        halted = true;
        return;
    }

    if (signals.memRead) {
        registers.write(
            static_cast<std::size_t>(instruction.dst),
            memory.read(static_cast<std::size_t>(immediate))
        );
        return;
    }

    if (signals.memWrite) {
        memory.write(
            static_cast<std::size_t>(immediate),
            registers.read(src1)
        );
        return;
    }

    if (signals.isJump) {
        programCounter = static_cast<std::size_t>(immediate);
        if (controlTransferTaken != nullptr) {
            *controlTransferTaken = true;
        }
        return;
    }

    if (signals.isBranch) {
        // Conditional branches update the PC only when the zero flag matches.
        if (signals.branchType == BranchType::JZ && zeroFlag) {
            programCounter = static_cast<std::size_t>(immediate);
            if (controlTransferTaken != nullptr) {
                *controlTransferTaken = true;
            }
        }

        if (signals.branchType == BranchType::JNZ && !zeroFlag) {
            programCounter = static_cast<std::size_t>(immediate);
            if (controlTransferTaken != nullptr) {
                *controlTransferTaken = true;
            }
        }

        return;
    }

    // MOV reaches this path with ALUOp::NONE and regWrite set.
    if (signals.aluOp == ALUOp::NONE && !signals.regWrite) {
        return;
    }

    // Immediate is the default write-back value for MOV.
    int result = immediate;

    switch (signals.aluOp) {
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

    if (signals.regWrite) {
        registers.write(static_cast<std::size_t>(instruction.dst), result);
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
        trace.memory = "-";
        trace.writeBack = "-";

        traceHistory.push_back(trace);
        pipelineDecodeStage = pipelineFetchStage;
        pipelineFetchStage.clear();
    }
}
