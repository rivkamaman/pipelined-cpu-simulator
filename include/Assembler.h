#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Instruction.h"

// Converts simple assembly text into Instruction objects for the CPU.
class Assembler {
public:
    // Assemble every instruction line from a file.
    static std::vector<Instruction> assembleFile(const std::string& filename);

    // Assemble a list of source lines.
    static std::vector<Instruction> assembleLines(const std::vector<std::string>& lines);

    // Parse one source line into one instruction.
    static Instruction parseLine(const std::string& line);

    // Parse one source line using a label table for jump and branch targets.
    static Instruction parseLine(
        const std::string& line,
        const std::unordered_map<std::string, int>& labelToIndex
    );

    // Convert an opcode token such as "ADD" into an Opcode value.
    static Opcode parseOpcode(const std::string& token);

    // Convert a register token such as "R2" into register index 2.
    static int parseRegister(const std::string& token);

    // Return true when token is a signed or unsigned decimal integer.
    static bool isNumber(const std::string& token);
};

#endif
