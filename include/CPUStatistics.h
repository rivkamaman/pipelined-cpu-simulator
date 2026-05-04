#ifndef CPU_STATISTICS_H
#define CPU_STATISTICS_H

class CPUStatistics {
public:
    void reset();

    void recordCycle();
    void recordInstruction();
    void recordStall();
    void recordFlush();
    void recordForwarding();
    void recordBranchPrediction();
    void recordBranchMisprediction();
    void recordBranchTaken();
    void recordBranchNotTaken();

    bool hasCycles() const;
    double cpi() const;
    double branchAccuracy() const;
    void print() const;

    int getFlushes() const;
    int getBranchPredictions() const;
    int getBranchMispredictions() const;
    int getBranchesTaken() const;
    int getBranchesNotTaken() const;

private:
    int cycles = 0;
    int instructions = 0;
    int stalls = 0;
    int flushes = 0;
    int forwardings = 0;
    int branchPredictions = 0;
    int branchMispredictions = 0;
    int branchesTaken = 0;
    int branchesNotTaken = 0;
};

#endif
