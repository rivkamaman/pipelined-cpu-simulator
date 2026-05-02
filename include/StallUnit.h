#ifndef STALL_UNIT_H
#define STALL_UNIT_H

#include "PipelineRegisters.h"

class StallUnit {
public:
    // A load-use stall is needed when IF/ID consumes a value still being loaded in ID/EX.
    static bool shouldStallForLoadUse(const IDEX& idex, const IFID& ifid);

private:
    static bool readsRegister(const Instruction& instruction, int reg);
};

#endif
