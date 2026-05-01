#include "ALU.h"

int ALU::add(int left, int right) const {
    // ADD datapath operation.
    return left + right;
}

int ALU::sub(int left, int right) const {
    // SUB datapath operation.
    return left - right;
}

bool ALU::equal(int left, int right) const {
    // CMP updates the zero flag from this comparison.
    return left == right;
}
