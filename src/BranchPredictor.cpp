#include "BranchPredictor.h"

#include <algorithm>

BranchPrediction BranchPredictor::predict(
    std::size_t pc,
    const ControlSignals& signals
) {
    BranchPrediction prediction;

    if (!signals.isBranch && !signals.isJump) {
        prediction.predictedTaken = false;
        prediction.predictedPc = pc + 1;
        return prediction;
    }

    // The BTB supplies the target PC during fetch. Without a BTB hit, even a
    // taken-leaning counter cannot redirect because fetch does not recompute
    // the branch target here.
    const auto target = branchTargetBuffer.find(pc);
    int state = 1;
    const auto stateEntry = predictionState.find(pc);
    if (stateEntry != predictionState.end()) {
        state = stateEntry->second;
    }

    prediction.predictedTaken = target != branchTargetBuffer.end() && state >= 2;
    if (prediction.predictedTaken) {
        prediction.predictedPc = target->second;
    } else {
        prediction.predictedPc = pc + 1;
    }

    return prediction;
}

void BranchPredictor::update(std::size_t pc, std::size_t targetPc, bool actualTaken) {
    auto stateEntry = predictionState.find(pc);
    if (stateEntry == predictionState.end()) {
        stateEntry = predictionState.emplace(pc, 1).first;
    }
    int& state = stateEntry->second;

    // Move the saturating counter toward the observed behavior:
    // 0/1 predict not taken, 2/3 predict taken.
    if (actualTaken) {
        state = std::min(state + 1, 3);
        branchTargetBuffer[pc] = targetPc;
    } else {
        state = std::max(state - 1, 0);
    }
}

void BranchPredictor::reset() {
    branchTargetBuffer.clear();
    predictionState.clear();
}
