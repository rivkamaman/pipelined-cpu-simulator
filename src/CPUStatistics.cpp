#include "CPUStatistics.h"

#include <iomanip>
#include <iostream>

void CPUStatistics::reset() {
    cycles = 0;
    instructions = 0;
    stalls = 0;
    flushes = 0;
    forwardings = 0;
    branchPredictions = 0;
    branchMispredictions = 0;
    branchesTaken = 0;
    branchesNotTaken = 0;
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

void CPUStatistics::recordBranchPrediction() {
    ++branchPredictions;
}

void CPUStatistics::recordBranchMisprediction() {
    ++branchMispredictions;
}

void CPUStatistics::recordBranchTaken() {
    ++branchesTaken;
}

void CPUStatistics::recordBranchNotTaken() {
    ++branchesNotTaken;
}

bool CPUStatistics::hasCycles() const {
    return cycles > 0;
}

double CPUStatistics::cpi() const {
    return instructions == 0
        ? 0.0
        : static_cast<double>(cycles) / instructions;
}

double CPUStatistics::branchAccuracy() const {
    return branchPredictions == 0
        ? 0.0
        : 100.0 * (branchPredictions - branchMispredictions) / branchPredictions;
}

int CPUStatistics::getFlushes() const {
    return flushes;
}

int CPUStatistics::getBranchPredictions() const {
    return branchPredictions;
}

int CPUStatistics::getBranchMispredictions() const {
    return branchMispredictions;
}

int CPUStatistics::getBranchesTaken() const {
    return branchesTaken;
}

int CPUStatistics::getBranchesNotTaken() const {
    return branchesNotTaken;
}

void CPUStatistics::print() const {
    std::cout << "\n=== CPU Statistics ===\n"
              << std::left << std::setw(24) << "Cycles:" << cycles << '\n'
              << std::left << std::setw(24) << "Instructions:" << instructions << '\n'
              << std::left << std::setw(24) << "CPI:" << std::fixed << std::setprecision(2) << cpi() << '\n'
              << std::left << std::setw(24) << "Stalls:" << stalls << '\n'
              << std::left << std::setw(24) << "Flushes:" << flushes << '\n'
              << std::left << std::setw(24) << "Forwardings:" << forwardings << '\n'
              << std::left << std::setw(24) << "Branch Predictions:" << branchPredictions << '\n'
              << std::left << std::setw(24) << "Branch Mispredictions:" << branchMispredictions << '\n'
              << std::left << std::setw(24) << "Branch Accuracy:"
              << std::fixed << std::setprecision(2) << branchAccuracy() << "%\n"
              << std::left << std::setw(24) << "Branches Taken:" << branchesTaken << '\n'
              << std::left << std::setw(24) << "Branches Not Taken:" << branchesNotTaken << "\n\n";
}
