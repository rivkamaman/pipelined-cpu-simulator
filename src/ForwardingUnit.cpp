#include "ForwardingUnit.h"

ForwardingDecision ForwardingUnit::resolve(
    const IDEX& idex,
    const EXMEM& exmem,
    const MEMWB& memwb
) {
    ForwardingDecision decision;

    if (!idex.valid) {
        return decision;
    }

    // Resolve only operands the instruction really reads.
    if (readsSrc1(idex)) {
        decision.forwardA = resolveSource(idex.instruction.getSrc1(), exmem, memwb);
    }
    if (readsSrc2(idex)) {
        decision.forwardB = resolveSource(idex.instruction.getSrc2(), exmem, memwb);
    }
    return decision;
}

bool ForwardingUnit::readsSrc1(const IDEX& idex) {
    return idex.signals.memWrite
        || idex.signals.aluOp == ALUOp::ADD
        || idex.signals.aluOp == ALUOp::ADDI
        || idex.signals.aluOp == ALUOp::SUB
        || idex.signals.aluOp == ALUOp::AND
        || idex.signals.aluOp == ALUOp::OR
        || idex.signals.aluOp == ALUOp::CMP;
}

bool ForwardingUnit::readsSrc2(const IDEX& idex) {
    return idex.signals.aluOp == ALUOp::ADD
        || idex.signals.aluOp == ALUOp::SUB
        || idex.signals.aluOp == ALUOp::AND
        || idex.signals.aluOp == ALUOp::OR
        || idex.signals.aluOp == ALUOp::CMP;
}

int ForwardingUnit::resolveSource(int src, const EXMEM& exmem, const MEMWB& memwb) {
    // EX/MEM is newer than MEM/WB, so it wins when both write the same register.
    if (canForwardFromExmem(exmem, src)) {
        return FROM_EXMEM;
    }

    if (canForwardFromMemwb(memwb, src)) {
        return FROM_MEMWB;
    }

    return NO_FORWARD;
}

bool ForwardingUnit::canForwardFromExmem(const EXMEM& exmem, int src) {
    return exmem.valid
        && exmem.signals.regWrite
        // A load's EX/MEM value is its address; the loaded data exists after MEM.
        && !exmem.signals.memRead
        && exmem.instruction.dst == src;
}

bool ForwardingUnit::canForwardFromMemwb(const MEMWB& memwb, int src) {
    return memwb.valid
        && memwb.signals.regWrite
        && memwb.instruction.dst == src;
}
