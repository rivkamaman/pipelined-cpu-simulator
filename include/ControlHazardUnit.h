#ifndef CONTROL_HAZARD_UNIT_H
#define CONTROL_HAZARD_UNIT_H

#include <cstddef>

#include "PipelineRegisters.h"

struct ControlHazardDecision {
    bool flush = false;
    bool redirectPc = false;
    std::size_t targetPc = 0;
};

class ControlHazardUnit {
public:
    static ControlHazardDecision resolve(const IDEX& idex, bool zeroFlag);
};

#endif
