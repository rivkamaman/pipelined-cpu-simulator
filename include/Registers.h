#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstddef>
#include <cstdint>
#include <vector>

class Registers {
public:
    explicit Registers(std::size_t count = 32);

    std::int32_t read(std::size_t index) const;
    void write(std::size_t index, std::int32_t value);
    std::size_t count() const;

private:
    std::vector<std::int32_t> values;
};

#endif
