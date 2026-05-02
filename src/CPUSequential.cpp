#include "CPU.h"

namespace {

std::string stageOrEmptyMarker(const std::string& stage) {
    return stage.empty() ? "-" : stage;
}

} // namespace

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
