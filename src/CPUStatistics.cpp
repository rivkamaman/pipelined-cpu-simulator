#include "CPUStatistics.h"

#include <iomanip>
#include <iostream>

void CPUStatistics::reset() {
    cycles = 0;
    instructions = 0;
    stalls = 0;
    flushes = 0;
    forwardings = 0;
}

void CPUStatistics::recordCycle() {
    ++cycles;
}

void CPUStatistics::recordInstruction() {
    ++instructions;
}

void CPUStatistics::recordStall() {
    ++stalls;
}

void CPUStatistics::recordFlush() {
    ++flushes;
}

void CPUStatistics::recordForwarding() {
    ++forwardings;
}

bool CPUStatistics::hasCycles() const {
    return cycles > 0;
}

double CPUStatistics::cpi() const {
    return instructions == 0
        ? 0.0
        : static_cast<double>(cycles) / instructions;
}

void CPUStatistics::print() const {
    std::cout << "\n=== CPU Statistics ===\n"
              << std::left << std::setw(14) << "Cycles:" << cycles << '\n'
              << std::left << std::setw(14) << "Instructions:" << instructions << '\n'
              << std::left << std::setw(14) << "CPI:" << std::fixed << std::setprecision(2) << cpi() << '\n'
              << std::left << std::setw(14) << "Stalls:" << stalls << '\n'
              << std::left << std::setw(14) << "Flushes:" << flushes << '\n'
              << std::left << std::setw(14) << "Forwardings:" << forwardings << "\n\n";
}
