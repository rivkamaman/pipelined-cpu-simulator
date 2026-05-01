#include "Assembler.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::vector<std::string> tokenize(const std::string& line) {
    const std::size_t commentStart = line.find('#');
    const std::string source = line.substr(0, commentStart);

    std::istringstream input(source);
    std::vector<std::string> tokens;
    std::string token;

    while (input >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

int parseInteger(const std::string& token, const std::string& fieldName) {
    try {
        std::size_t parsedChars = 0;
        const int value = std::stoi(token, &parsedChars);

        if (parsedChars != token.size()) {
            throw std::invalid_argument("Trailing characters");
        }

        return value;
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid " + fieldName + ": " + token);
    }
}

void requireOperandCount(
    const std::vector<std::string>& tokens,
    std::size_t expected,
    const std::string& opcode
) {
    if (tokens.size() != expected) {
        throw std::invalid_argument("Invalid operand count for " + opcode);
    }
}

} // namespace

std::vector<Instruction> Assembler::assembleFile(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error("Could not open assembly file: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    return assembleLines(lines);
}

std::vector<Instruction> Assembler::assembleLines(const std::vector<std::string>& lines) {
    std::vector<Instruction> program;

    for (const std::string& line : lines) {
        if (tokenize(line).empty()) {
            continue;
        }

        program.push_back(parseLine(line));
    }

    return program;
}

Instruction Assembler::parseLine(const std::string& line) {
    const std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) {
        throw std::invalid_argument("Cannot parse an empty assembly line");
    }

    const Opcode opcode = parseOpcode(tokens[0]);

    switch (opcode) {
        case Opcode::MOV:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::MOV,
                parseRegister(tokens[1]),
                0,
                0,
                parseInteger(tokens[2], "immediate")
            );
        case Opcode::ADD:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::ADD,
                parseRegister(tokens[1]),
                parseRegister(tokens[2]),
                parseRegister(tokens[3]),
                0
            );
        case Opcode::SUB:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::SUB,
                parseRegister(tokens[1]),
                parseRegister(tokens[2]),
                parseRegister(tokens[3]),
                0
            );
        case Opcode::CMP:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::CMP,
                0,
                parseRegister(tokens[1]),
                parseRegister(tokens[2]),
                0
            );
        case Opcode::LOAD:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::LOAD,
                parseRegister(tokens[1]),
                0,
                0,
                parseInteger(tokens[2], "address")
            );
        case Opcode::STORE:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::STORE,
                0,
                parseRegister(tokens[1]),
                0,
                parseInteger(tokens[2], "address")
            );
        case Opcode::JMP:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JMP, 0, 0, 0, parseInteger(tokens[1], "target"));
        case Opcode::JZ:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JZ, 0, 0, 0, parseInteger(tokens[1], "target"));
        case Opcode::JNZ:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JNZ, 0, 0, 0, parseInteger(tokens[1], "target"));
        case Opcode::HALT:
            requireOperandCount(tokens, 1, tokens[0]);
            return Instruction(Opcode::HALT);
    }

    throw std::invalid_argument("Unsupported opcode");
}

Opcode Assembler::parseOpcode(const std::string& token) {
    if (token == "MOV") {
        return Opcode::MOV;
    }
    if (token == "ADD") {
        return Opcode::ADD;
    }
    if (token == "SUB") {
        return Opcode::SUB;
    }
    if (token == "LOAD") {
        return Opcode::LOAD;
    }
    if (token == "STORE") {
        return Opcode::STORE;
    }
    if (token == "CMP") {
        return Opcode::CMP;
    }
    if (token == "JMP") {
        return Opcode::JMP;
    }
    if (token == "JZ") {
        return Opcode::JZ;
    }
    if (token == "JNZ") {
        return Opcode::JNZ;
    }
    if (token == "HALT") {
        return Opcode::HALT;
    }

    throw std::invalid_argument("Unknown opcode: " + token);
}

int Assembler::parseRegister(const std::string& token) {
    if (token.size() < 2 || token[0] != 'R') {
        throw std::invalid_argument("Invalid register format: " + token);
    }

    for (std::size_t index = 1; index < token.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(token[index]))) {
            throw std::invalid_argument("Invalid register format: " + token);
        }
    }

    return parseInteger(token.substr(1), "register");
}
