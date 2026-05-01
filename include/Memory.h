#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <cstdint>
#include <vector>

// Word-addressable data memory with bounds-checked reads and writes.
class Memory {
public:
    // Allocate memory with the given number of 32-bit words.
    explicit Memory(std::size_t size);

    // Read one word from address.
    std::int32_t read(std::size_t address) const;

    // Write one word to address.
    void write(std::size_t address, std::int32_t value);

    // Return the number of addressable words.
    std::size_t size() const;

private:
    // Backing storage for memory words.
    std::vector<std::int32_t> data;
};

#endif
