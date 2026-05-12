#include "HazardUnit.h"

bool HazardUnit::hasDataHazard(const IDEX& idex, const IFID& ifid) {
    return writerHazardsWithReader(idex.valid, idex.instruction, ifid);
}

bool HazardUnit::hasDataHazard(const EXMEM& exmem, const IFID& ifid) {
    return writerHazardsWithReader(exmem.valid, exmem.instruction, ifid);
}

bool HazardUnit::hasDataHazard(const MEMWB& memwb, const IFID& ifid) {
    return writerHazardsWithReader(memwb.valid, memwb.instruction, ifid);
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

bool HazardUnit::writerHazardsWithReader(
    bool writerValid,
    const Instruction& writerInstruction,
    const IFID& ifid
) {
    if (!writerValid || !ifid.valid || !writerInstruction.writesRegister()) {
        return false;
    }

    return (ifid.instruction.readsSrc1()
            && ifid.instruction.getSrc1() == writerInstruction.dst)
        || (ifid.instruction.readsSrc2()
            && ifid.instruction.getSrc2() == writerInstruction.dst);
}
