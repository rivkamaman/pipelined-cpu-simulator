#include "CPU.h"

#include <iostream>

#include "ForwardingUnit.h"
#include "HazardUnit.h"
#include "StallUnit.h"

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

    const bool stallForLoadUse = !pipelineFlushRequested
        && StallUnit::shouldStallForLoadUse(idex, ifid);

    // D) ID
    IDEX nextIdex;
    if (!pipelineFlushRequested) {
        if (HazardUnit::hasDataHazard(idex, ifid)) {
            std::cout << "Warning: RAW hazard detected between "
                      << idex.instruction.toString()
                      << " and "
                      << ifid.instruction.toString()
                      << std::endl;
        }
        if (HazardUnit::hasDataHazard(exmem, ifid)) {
            std::cout << "Warning: RAW hazard detected between "
                      << exmem.instruction.toString()
                      << " and "
                      << ifid.instruction.toString()
                      << std::endl;
        }
        if (HazardUnit::hasDataHazard(memwb, ifid)) {
            std::cout << "Warning: RAW hazard detected between "
                      << memwb.instruction.toString()
                      << " and "
                      << ifid.instruction.toString()
                      << std::endl;
        }
        if (ifid.valid) {
            trace.decode = ifid.instruction.toString();
        }
        // On a load-use stall, leave nextIdex invalid: that inserts the bubble.
        if (!stallForLoadUse) {
            nextIdex = decodeStage(ifid);
        }
    }

    // E) IF
    IFID nextIfid;
    if (!pipelineFlushRequested) {
        if (stallForLoadUse) {
            // Freeze IF/ID and skip fetchStage(), which also freezes the PC.
            nextIfid = ifid;
            trace.fetch = "STALL";
        } else {
            nextIfid = fetchStage();
            if (nextIfid.valid) {
                trace.fetch = nextIfid.instruction.toString();
            }
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
    const ForwardingDecision forwarding = ForwardingUnit::resolve(input, exmem, memwb);

    // Start with register-file operands, then replace only the sources that need bypassing.
    int operandA = registers.read(src1);
    int operandB = registers.read(src2);

    if (forwarding.forwardA == FROM_EXMEM) {
        operandA = exmem.aluResult;
    } else if (forwarding.forwardA == FROM_MEMWB) {
        operandA = memwb.writeBackData;
    }

    if (forwarding.forwardB == FROM_EXMEM) {
        operandB = exmem.aluResult;
    } else if (forwarding.forwardB == FROM_MEMWB) {
        operandB = memwb.writeBackData;
    }

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
        output.storeData = operandA;
        return output;
    }

    if (input.signals.aluOp == ALUOp::NONE && !input.signals.regWrite) {
        return output;
    }

    int result = immediate;
    switch (input.signals.aluOp) {
        case ALUOp::ADD:
            result = alu.add(operandA, operandB);
            break;
        case ALUOp::ADDI:
            result = alu.addImmediate(operandA, immediate);
            break;
        case ALUOp::SUB:
            result = alu.sub(operandA, operandB);
            break;
        case ALUOp::AND:
            result = alu.bitwiseAnd(operandA, operandB);
            break;
        case ALUOp::OR:
            result = alu.bitwiseOr(operandA, operandB);
            break;
        case ALUOp::CMP:
            zeroFlag = alu.equal(operandA, operandB);
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
