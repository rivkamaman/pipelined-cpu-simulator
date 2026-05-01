#include "ALU.h"

int ALU::add(int left, int right) const {
    // ADD datapath operation.
    return left + right;
}

int ALU::addImmediate(int base, int immediate) const {
    // ADDI datapath operation.
    return base + immediate;
}

int ALU::sub(int left, int right) const {
    // SUB datapath operation.
    return left - right;
}

int ALU::bitwiseAnd(int left, int right) const {
    // AND datapath operation.
    return left & right;
}

int ALU::bitwiseOr(int left, int right) const {
    // OR datapath operation.
    return left | right;
}

bool ALU::equal(int left, int right) const {
    // CMP updates the zero flag from this comparison.
    return left == right;
}
