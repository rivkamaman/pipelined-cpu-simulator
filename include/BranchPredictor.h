#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <cstddef>
#include <unordered_map>

#include "ControlUnit.h"
#include "Instruction.h"

struct BranchPrediction {
    bool predictedTaken = false;
    std::size_t predictedPc = 0;
};

class BranchPredictor {
public:
    BranchPrediction predict(
        std::size_t pc,
        const Instruction& instruction,
        const ControlSignals& signals
    );

    void update(std::size_t pc, bool actualTaken);

    void reset();

private:
    std::unordered_map<std::size_t, int> table;
};

#endif
