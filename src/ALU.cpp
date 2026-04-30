#include "ALU.h"

int ALU::add(int left, int right) const {
    return left + right;
}

int ALU::sub(int left, int right) const {
    return left - right;
}

bool ALU::equal(int left, int right) const {
    return left == right;
}
