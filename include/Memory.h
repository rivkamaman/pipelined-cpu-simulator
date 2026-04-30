#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <cstdint>
#include <vector>

class Memory {
public:
    explicit Memory(std::size_t size);

    std::int32_t read(std::size_t address) const;
    void write(std::size_t address, std::int32_t value);
    std::size_t size() const;

private:
    std::vector<std::int32_t> data;
};

#endif
