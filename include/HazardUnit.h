#ifndef HAZARD_UNIT_H
#define HAZARD_UNIT_H

#include "PipelineRegisters.h"

// Detects pipeline hazards without changing execution or timing.
class HazardUnit {
public:
    static bool hasDataHazard(const IDEX& idex, const IFID& ifid);
    static bool hasDataHazard(const EXMEM& exmem, const IFID& ifid);
    static bool hasDataHazard(const MEMWB& memwb, const IFID& ifid);

    static bool hasDataHazard(
        const IDEX& idex,
        const EXMEM& exmem,
        const MEMWB& memwb,
        const IFID& ifid
    );

private:
    static bool readsRegister(const Instruction& instruction, int reg);
    static bool writerHazardsWithReader(
        bool writerValid,
        const ControlSignals& writerSignals,
        const Instruction& writerInstruction,
        const IFID& ifid
    );
};

#endif
