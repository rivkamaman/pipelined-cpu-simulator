#include "Instruction.h"

Instruction::Instruction(
    Opcode opcode,
    int dst,
    int src1,
    int src2,
    int immediate
)
    : opcode(opcode),
      dst(dst),
      src1(src1),
      src2(src2),
      immediate(immediate) {
}
