#include "StallUnit.h"

bool StallUnit::shouldStallForLoadUse(const IDEX& idex, const IFID& ifid) {
    // Only a valid load in ID/EX can create the one-cycle load-use timing gap.
    if (!idex.valid || !ifid.valid || !idex.instruction.isMemoryRead()) {
        return false;
    }

    // The younger IF/ID instruction must wait if it reads the load destination.
    return (ifid.instruction.readsSrc1()
            && ifid.instruction.getSrc1() == idex.instruction.dst)
        || (ifid.instruction.readsSrc2()
            && ifid.instruction.getSrc2() == idex.instruction.dst);
}
