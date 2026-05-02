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

    bool hasCycles() const;
    double cpi() const;
    void print() const;

private:
    int cycles = 0;
    int instructions = 0;
    int stalls = 0;
    int flushes = 0;
    int forwardings = 0;
};

#endif
