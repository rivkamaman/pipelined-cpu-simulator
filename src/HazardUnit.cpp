#include "HazardUnit.h"

#include "ControlUnit.h"

bool HazardUnit::hasDataHazard(const IDEX& idex, const IFID& ifid) {
    return writerHazardsWithReader(idex.valid, idex.signals, idex.instruction, ifid);
}

bool HazardUnit::hasDataHazard(const EXMEM& exmem, const IFID& ifid) {
    return writerHazardsWithReader(exmem.valid, exmem.signals, exmem.instruction, ifid);
}

bool HazardUnit::hasDataHazard(const MEMWB& memwb, const IFID& ifid) {
    return writerHazardsWithReader(memwb.valid, memwb.signals, memwb.instruction, ifid);
}

bool HazardUnit::hasDataHazard(
    const IDEX& idex,
    const EXMEM& exmem,
    const MEMWB& memwb,
    const IFID& ifid
) {
    return hasDataHazard(idex, ifid)
        || hasDataHazard(exmem, ifid)
        || hasDataHazard(memwb, ifid);
}

bool HazardUnit::readsRegister(const Instruction& instruction, int reg) {
    if (reg < 0) {
        return false;
    }

    const ControlSignals signals = ControlUnit::decode(instruction);

    if (signals.memWrite) {
        return instruction.getSrc1() == reg;
    }

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

bool HazardUnit::writerHazardsWithReader(
    bool writerValid,
    const ControlSignals& writerSignals,
    const Instruction& writerInstruction,
    const IFID& ifid
) {
    if (!writerValid || !ifid.valid || !writerSignals.regWrite) {
        return false;
    }

    return readsRegister(ifid.instruction, writerInstruction.dst);
}
