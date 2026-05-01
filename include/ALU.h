#ifndef ALU_H
#define ALU_H

// Arithmetic Logic Unit: performs the CPU's integer operations.
class ALU {
public:
    // Return left + right for ADD instructions.
    int add(int left, int right) const;

    // Return base + immediate for ADDI instructions.
    int addImmediate(int base, int immediate) const;

    // Return left - right for SUB instructions.
    int sub(int left, int right) const;

    // Return left & right for AND instructions.
    int bitwiseAnd(int left, int right) const;

    // Return left | right for OR instructions.
    int bitwiseOr(int left, int right) const;

    // Return true when both operands are equal for CMP instructions.
    bool equal(int left, int right) const;
};

#endif
