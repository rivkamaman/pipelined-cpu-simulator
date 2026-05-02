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

    // Resolve each source independently; one operand may forward while the other reads normally.
    decision.forwardA = resolveSource(idex.instruction.getSrc1(), exmem, memwb);
    decision.forwardB = resolveSource(idex.instruction.getSrc2(), exmem, memwb);
    return decision;
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
