#ifndef FORWARDING_UNIT_H
#define FORWARDING_UNIT_H

#include "PipelineRegisters.h"

enum ForwardingSource {
    NO_FORWARD = 0,
    // Use the newest ALU result from the previous pipeline stage.
    FROM_EXMEM = 1,
    // Use the value that is about to be written back.
    FROM_MEMWB = 2
};

struct ForwardingDecision {
    // Forwarding choice for src1.
    int forwardA = NO_FORWARD;

    // Forwarding choice for src2.
    int forwardB = NO_FORWARD;
};

class ForwardingUnit {
public:
    static ForwardingDecision resolve(
        const IDEX& idex,
        const EXMEM& exmem,
        const MEMWB& memwb
    );

private:
    static bool readsSrc1(const IDEX& idex);
    static bool readsSrc2(const IDEX& idex);
    static int resolveSource(int src, const EXMEM& exmem, const MEMWB& memwb);
    static bool canForwardFromExmem(const EXMEM& exmem, int src);
    static bool canForwardFromMemwb(const MEMWB& memwb, int src);
};

#endif
