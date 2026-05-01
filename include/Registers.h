#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstddef>
#include <cstdint>
#include <vector>

// Register file with bounds-checked integer registers.
class Registers {
public:
    // Allocate count registers initialized to zero.
    explicit Registers(std::size_t count = 32);

    // Read the value stored in a register.
    std::int32_t read(std::size_t index) const;

    // Write a value into a register.
    void write(std::size_t index, std::int32_t value);

    // Return the number of registers.
    std::size_t count() const;

private:
    // Backing storage for register values.
    std::vector<std::int32_t> values;
};

#endif
