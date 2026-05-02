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

    if (!idex.signals.isBranch) {
        return decision;
    }

    // Branches are resolved in EX because the condition flags are known there.
    const bool taken =
        (idex.signals.branchType == BranchType::JZ && zeroFlag)
        || (idex.signals.branchType == BranchType::JNZ && !zeroFlag);

    if (taken) {
        decision.flush = true;
        decision.redirectPc = true;
        decision.targetPc = target;
    }

    return decision;
}
