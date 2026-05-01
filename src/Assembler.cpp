#include "Assembler.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {

std::vector<std::string> tokenize(const std::string& line) {
    // Ignore everything after '#', then split the remaining source on whitespace.
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

        // std::stoi accepts prefixes, so reject tokens like "12abc".
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

bool hasLabelPrefix(const std::vector<std::string>& tokens) {
    // Labels are written as the first token followed by a colon, e.g. "loop:".
    return !tokens.empty() && !tokens[0].empty() && tokens[0].back() == ':';
}

std::string parseLabelName(const std::string& token) {
    const std::string label = token.substr(0, token.size() - 1);
    if (label.empty()) {
        throw std::invalid_argument("Invalid empty label");
    }

    if (Assembler::isNumber(label)) {
        throw std::invalid_argument("Label cannot be numeric: " + label);
    }

    return label;
}

std::vector<std::string> stripLabelPrefix(const std::vector<std::string>& tokens) {
    if (!hasLabelPrefix(tokens)) {
        return tokens;
    }

    parseLabelName(tokens[0]);
    return std::vector<std::string>(tokens.begin() + 1, tokens.end());
}

std::unordered_map<std::string, int> buildLabelMap(const std::vector<std::string>& lines) {
    std::unordered_map<std::string, int> labelToIndex;
    int instructionIndex = 0;

    // First pass: map each label to the instruction index it points at.
    for (const std::string& line : lines) {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        const std::vector<std::string> instructionTokens = stripLabelPrefix(tokens);

        if (hasLabelPrefix(tokens)) {
            const std::string label = parseLabelName(tokens[0]);
            if (labelToIndex.count(label) > 0) {
                throw std::invalid_argument("Duplicate label: " + label);
            }

            labelToIndex[label] = instructionIndex;
        }

        if (!instructionTokens.empty()) {
            ++instructionIndex;
        }
    }

    return labelToIndex;
}

int parseTarget(
    const std::string& token,
    const std::unordered_map<std::string, int>& labelToIndex
) {
    // Branches and jumps may target either a numeric instruction index or a label.
    if (Assembler::isNumber(token)) {
        return parseInteger(token, "target");
    }

    const auto label = labelToIndex.find(token);
    if (label == labelToIndex.end()) {
        throw std::invalid_argument("Unknown label: " + token);
    }

    return label->second;
}

Instruction parseTokens(
    const std::vector<std::string>& sourceTokens,
    const std::unordered_map<std::string, int>& labelToIndex
) {
    // Parse the instruction portion after any optional leading label.
    const std::vector<std::string> tokens = stripLabelPrefix(sourceTokens);
    if (tokens.empty()) {
        throw std::invalid_argument("Cannot parse a label-only assembly line");
    }

    const Opcode opcode = Assembler::parseOpcode(tokens[0]);

    switch (opcode) {
        case Opcode::MOV:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::MOV,
                Assembler::parseRegister(tokens[1]),
                0,
                0,
                parseInteger(tokens[2], "immediate")
            );
        case Opcode::ADD:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::ADD,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                Assembler::parseRegister(tokens[3]),
                0
            );
        case Opcode::ADDI:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::ADDI,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                0,
                parseInteger(tokens[3], "immediate")
            );
        case Opcode::SUB:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::SUB,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                Assembler::parseRegister(tokens[3]),
                0
            );
        case Opcode::AND:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::AND,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                Assembler::parseRegister(tokens[3]),
                0
            );
        case Opcode::OR:
            requireOperandCount(tokens, 4, tokens[0]);
            return Instruction(
                Opcode::OR,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                Assembler::parseRegister(tokens[3]),
                0
            );
        case Opcode::CMP:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::CMP,
                0,
                Assembler::parseRegister(tokens[1]),
                Assembler::parseRegister(tokens[2]),
                0
            );
        case Opcode::LOAD:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::LOAD,
                Assembler::parseRegister(tokens[1]),
                0,
                0,
                parseInteger(tokens[2], "address")
            );
        case Opcode::STORE:
            requireOperandCount(tokens, 3, tokens[0]);
            return Instruction(
                Opcode::STORE,
                0,
                Assembler::parseRegister(tokens[1]),
                0,
                parseInteger(tokens[2], "address")
            );
        case Opcode::JMP:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JMP, 0, 0, 0, parseTarget(tokens[1], labelToIndex));
        case Opcode::JZ:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JZ, 0, 0, 0, parseTarget(tokens[1], labelToIndex));
        case Opcode::JNZ:
            requireOperandCount(tokens, 2, tokens[0]);
            return Instruction(Opcode::JNZ, 0, 0, 0, parseTarget(tokens[1], labelToIndex));
        case Opcode::NOP:
            requireOperandCount(tokens, 1, tokens[0]);
            return Instruction(Opcode::NOP);
        case Opcode::HALT:
            requireOperandCount(tokens, 1, tokens[0]);
            return Instruction(Opcode::HALT);
    }

    throw std::invalid_argument("Unsupported opcode");
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
    const std::unordered_map<std::string, int> labelToIndex = buildLabelMap(lines);

    // Second pass: convert every real instruction into executable form.
    for (const std::string& line : lines) {
        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        if (stripLabelPrefix(tokens).empty()) {
            continue;
        }

        program.push_back(parseTokens(tokens, labelToIndex));
    }

    return program;
}

Instruction Assembler::parseLine(const std::string& line) {
    return parseLine(line, {});
}

Instruction Assembler::parseLine(
    const std::string& line,
    const std::unordered_map<std::string, int>& labelToIndex
) {
    const std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) {
        throw std::invalid_argument("Cannot parse an empty assembly line");
    }

    return parseTokens(tokens, labelToIndex);
}

Opcode Assembler::parseOpcode(const std::string& token) {
    if (token == "MOV") {
        return Opcode::MOV;
    }
    if (token == "ADD") {
        return Opcode::ADD;
    }
    if (token == "ADDI") {
        return Opcode::ADDI;
    }
    if (token == "SUB") {
        return Opcode::SUB;
    }
    if (token == "AND") {
        return Opcode::AND;
    }
    if (token == "OR") {
        return Opcode::OR;
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
    if (token == "NOP") {
        return Opcode::NOP;
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

bool Assembler::isNumber(const std::string& token) {
    if (token.empty()) {
        return false;
    }

    // Accept an optional sign before the decimal digits.
    std::size_t index = 0;
    if (token[0] == '-' || token[0] == '+') {
        if (token.size() == 1) {
            return false;
        }

        index = 1;
    }

    for (; index < token.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(token[index]))) {
            return false;
        }
    }

    return true;
}
