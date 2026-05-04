#include "ControlHazardUnit.h"

ControlHazardDecision ControlHazardUnit::resolve(const IDEX& idex, bool zeroFlag) {
    ControlHazardDecision decision;

    if (!idex.valid) {
        return decision;
    }

    const std::size_t target = static_cast<std::size_t>(idex.instruction.getImmediate());

    // An unconditional jump always redirects the PC and flushes younger wrong-path work.
    if (idex.signals.isJump) {
        decision.flush = true;
        decision.redirectPc = true;
        decision.targetPc = target;
        return decision;
    }

    (void)zeroFlag;
    return decision;
}
