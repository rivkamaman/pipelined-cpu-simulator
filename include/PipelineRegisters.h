#ifndef PIPELINE_REGISTERS_H
#define PIPELINE_REGISTERS_H

#include <cstddef>

#include "ControlUnit.h"
#include "Instruction.h"

struct IFID {
    bool valid = false;
    std::size_t pc = 0;
    Instruction instruction;
    bool predictedTaken = false;
    std::size_t predictedPc = 0;
};

struct IDEX {
    bool valid = false;
    std::size_t pc = 0;
    Instruction instruction;
    ControlSignals signals;
    bool predictedTaken = false;
    std::size_t predictedPc = 0;
};

struct EXMEM {
    bool valid = false;
    std::size_t pc = 0;
    Instruction instruction;
    ControlSignals signals;
    int aluResult = 0;
    int storeData = 0;
};

struct MEMWB {
    bool valid = false;
    std::size_t pc = 0;
    Instruction instruction;
    ControlSignals signals;
    int writeBackData = 0;
};

#endif
