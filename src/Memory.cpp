#include "Memory.h"

#include <stdexcept>

Memory::Memory(std::size_t size)
    // Memory starts cleared to zero.
    : data(size, 0) {
}

std::int32_t Memory::read(std::size_t address) const {
    // Keep invalid program addresses from indexing outside the vector.
    if (address >= data.size()) {
        throw std::out_of_range("Memory read address is out of range");
    }

    return data[address];
}

void Memory::write(std::size_t address, std::int32_t value) {
    // Keep invalid program addresses from indexing outside the vector.
    if (address >= data.size()) {
        throw std::out_of_range("Memory write address is out of range");
    }

    data[address] = value;
}

std::size_t Memory::size() const {
    return data.size();
}
