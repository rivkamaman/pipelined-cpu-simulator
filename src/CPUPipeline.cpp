#include "CPU.h"

void CPU::runPipelined() {
    while (!fetchHalted || ifid.valid || idex.valid || exmem.valid || memwb.valid) {
        stepPipelined();
    }

    halted = true;
    printPipelineTrace();
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
