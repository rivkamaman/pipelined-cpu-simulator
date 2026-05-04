#include "BranchPredictor.h"

#include <algorithm>

BranchPrediction BranchPredictor::predict(
    std::size_t pc,
    const Instruction& instruction,
    const ControlSignals& signals
) {
    BranchPrediction prediction;

    if (!signals.isBranch) {
        prediction.predictedTaken = false;
        prediction.predictedPc = pc + 1;
        return prediction;
    }

    // Branch prediction guesses the next PC before EX knows the real branch
    // outcome. The 2-bit counter keeps one unusual outcome from flipping the
    // prediction immediately.
    const int state = table.count(pc) == 0 ? 0 : table.at(pc);
    prediction.predictedTaken = state >= 2;
    prediction.predictedPc = prediction.predictedTaken
        ? static_cast<std::size_t>(instruction.getImmediate())
        : pc + 1;
    return prediction;
}

void BranchPredictor::update(std::size_t pc, bool actualTaken) {
    int& state = table[pc];

    // Move the saturating counter toward the observed behavior:
    // 0/1 predict not taken, 2/3 predict taken.
    if (actualTaken) {
        state = std::min(state + 1, 3);
    } else {
        state = std::max(state - 1, 0);
    }
}

void BranchPredictor::reset() {
    table.clear();
}
