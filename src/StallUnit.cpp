#include "StallUnit.h"

#include "ControlUnit.h"

bool StallUnit::shouldStallForLoadUse(const IDEX& idex, const IFID& ifid) {
    // Only a valid load in ID/EX can create the one-cycle load-use timing gap.
    if (!idex.valid || !ifid.valid || !idex.signals.memRead) {
        return false;
    }

    // The younger IF/ID instruction must wait if it reads the load destination.
    return readsRegister(ifid.instruction, idex.instruction.dst);
}

bool StallUnit::readsRegister(const Instruction& instruction, int reg) {
    if (reg < 0) {
        return false;
    }

    const ControlSignals signals = ControlUnit::decode(instruction);

    // STORE reads src1 as the value to write to memory.
    if (signals.memWrite) {
        return instruction.getSrc1() == reg;
    }

    // Use control signals so the stall unit stays aligned with ControlUnit decoding.
    switch (signals.aluOp) {
        case ALUOp::ADD:
        case ALUOp::SUB:
        case ALUOp::AND:
        case ALUOp::OR:
        case ALUOp::CMP:
            return instruction.getSrc1() == reg || instruction.getSrc2() == reg;
        case ALUOp::ADDI:
            return instruction.getSrc1() == reg;
        case ALUOp::NONE:
            return false;
    }

    return false;
}
