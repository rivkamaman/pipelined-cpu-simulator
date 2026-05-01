#include "Instruction.h"

Instruction::Instruction(
    Opcode opcode,
    int dst,
    int src1,
    int src2,
    int immediate
)
    // Store the fixed instruction fields exactly as supplied by the program.
    : opcode(opcode),
      dst(dst),
      src1(src1),
      src2(src2),
      immediate(immediate) {
}

int Instruction::getSrc1() const {
    return src1;
}

int Instruction::getSrc2() const {
    return src2;
}

int Instruction::getImmediate() const {
    return immediate;
}
