#include "Registers.h"

#include <stdexcept>

Registers::Registers(std::size_t count)
    : values(count, 0) {
}

std::int32_t Registers::read(std::size_t index) const {
    if (index >= values.size()) {
        throw std::out_of_range("Register read index is out of range");
    }

    return values[index];
}

void Registers::write(std::size_t index, std::int32_t value) {
    if (index >= values.size()) {
        throw std::out_of_range("Register write index is out of range");
    }

    values[index] = value;
}

std::size_t Registers::count() const {
    return values.size();
}
