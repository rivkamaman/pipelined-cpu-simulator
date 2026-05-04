#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Assembler.h"
#include "ControlHazardUnit.h"
#include "CPU.h"
#include "ForwardingUnit.h"
#include "HazardUnit.h"
#include "StallUnit.h"

void testAssemblerMovParsing() {
    const Instruction instruction = Assembler::parseLine("MOV R2 42");

    assert(instruction.opcode == Opcode::MOV);
    assert(instruction.dst == 2);
    assert(instruction.getImmediate() == 42);
}

void testAssemblerAddParsing() {
    const Instruction instruction = Assembler::parseLine("ADD R2 R0 R1");

    assert(instruction.opcode == Opcode::ADD);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerAddiParsing() {
    const Instruction instruction = Assembler::parseLine("ADDI R2 R0 5");

    assert(instruction.opcode == Opcode::ADDI);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getImmediate() == 5);
}

void testAssemblerAndParsing() {
    const Instruction instruction = Assembler::parseLine("AND R3 R0 R1");

    assert(instruction.opcode == Opcode::AND);
    assert(instruction.dst == 3);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerOrParsing() {
    const Instruction instruction = Assembler::parseLine("OR R4 R0 R1");

    assert(instruction.opcode == Opcode::OR);
    assert(instruction.dst == 4);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
}

void testAssemblerBranchParsing() {
    const Instruction instruction = Assembler::parseLine("JZ 8");

    assert(instruction.opcode == Opcode::JZ);
    assert(instruction.getImmediate() == 8);
}

void testAssemblerNopParsing() {
    const Instruction instruction = Assembler::parseLine("NOP");

    assert(instruction.opcode == Opcode::NOP);
}

void testAssemblerLabelBranchParsing() {
    const std::vector<std::string> lines = {
        "MOV R0 7",
        "JZ done",
        "MOV R1 -1",
        "done:",
        "HALT"
    };

    const std::vector<Instruction> program = Assembler::assembleLines(lines);

    assert(program.size() == 4);
    assert(program[1].opcode == Opcode::JZ);
    assert(program[1].getImmediate() == 3);
}

void testAssemblerAssembleLines() {
    const std::vector<std::string> lines = {
        "MOV R0 5",
        "MOV R1 3",
        "ADD R2 R0 R1",
        "",
        "# comment-only line",
        "HALT"
    };

    const std::vector<Instruction> program = Assembler::assembleLines(lines);

    assert(program.size() == 4);
    assert(program[0].opcode == Opcode::MOV);
    assert(program[1].opcode == Opcode::MOV);
    assert(program[2].opcode == Opcode::ADD);
    assert(program[3].opcode == Opcode::HALT);
}

void testAssemblerInvalidOpcodeThrows() {
    bool threw = false;

    try {
        Assembler::parseLine("BAD R0 1");
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAssemblerProgramRunsOnCpu() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "MOV R1 3",
        "ADD R2 R0 R1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
}

void testAssemblerAddSubFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/add_sub.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
    assert(cpu.getRegisterValue(3) == 2);
}

void testAssemblerLoadStoreFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/load_store.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(0) == 42);
    assert(cpu.getRegisterValue(1) == 42);
}

void testAssemblerLabelsFileRunsOnCpu() {
    CPU cpu;

    cpu.loadProgram(Assembler::assembleFile("tests/labels.asm"));
    cpu.run();

    assert(cpu.getRegisterValue(2) == 42);
}

void testAssemblerUnknownLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"JMP missing", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAssemblerDuplicateLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"again:", "MOV R0 1", "again:", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAddSub() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 5},     // R0 = 5
        {Opcode::MOV, 1, 0, 0, 3},     // R1 = 3
        {Opcode::ADD, 2, 0, 1, 0},     // R2 = 8
        {Opcode::SUB, 3, 0, 1, 0},     // R3 = 2
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(0) == 5);
    assert(cpu.getRegisterValue(1) == 3);
    assert(cpu.getRegisterValue(2) == 8);
    assert(cpu.getRegisterValue(3) == 2);
}

void testJZBranchTaken() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 7},
        {Opcode::MOV, 1, 0, 0, 7},
        {Opcode::CMP, 0, 1, 0, 0},     // zeroFlag = true
        {Opcode::JZ, 0, 0, 0, 5},      // jump to HALT
        {Opcode::MOV, 2, 0, 0, 99},    // skipped
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getZeroFlag() == true);
    assert(cpu.getRegisterValue(2) == 0);
}

void testJNZBranchTaken() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 7},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::CMP, 0, 1, 0, 0},     // zeroFlag = false
        {Opcode::JNZ, 0, 0, 0, 5},     // jump to HALT
        {Opcode::MOV, 2, 0, 0, 99},    // skipped
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getZeroFlag() == false);
    assert(cpu.getRegisterValue(2) == 0);
}

void testLoadStore() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 42},    // R0 = 42
        {Opcode::STORE, 0, 0, 0, 10},  // memory[10] = R0
        {Opcode::LOAD, 1, 0, 0, 10},   // R1 = memory[10]
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(1) == 42);
}

void testAddi() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 10},
        {Opcode::ADDI, 1, 0, 0, 5},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(1) == 15);
}

void testAnd() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 6},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::AND, 2, 0, 1, 0},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(2) == 2);
}

void testOr() {
    CPU cpu;

    std::vector<Instruction> program = {
        {Opcode::MOV, 0, 0, 0, 6},
        {Opcode::MOV, 1, 0, 0, 3},
        {Opcode::OR, 2, 0, 1, 0},
        {Opcode::HALT, 0, 0, 0, 0}
    };

    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.getRegisterValue(2) == 7);
}

void testPipelinedWithNopsAvoidsHazard() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "NOP",
        "NOP",
        "NOP",
        "ADDI R1 R0 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 6);
}

void testPipelinedBranchTakenFlushesYoungerInstructions() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "NOP",
        "NOP",
        "NOP",
        "CMP R0 R0",
        "JZ target",
        "MOV R1 99",
        "target:",
        "MOV R1 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 1);
}

void testPipelinedCorrectlyPredictedNotTakenBranchDoesNotFlush() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 1",
        "MOV R1 2",
        "CMP R0 R1",
        "JZ target",
        "MOV R2 5",
        "target:",
        "MOV R3 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(2) == 5);
    assert(cpu.getRegisterValue(3) == 7);
    assert(cpu.getStatistics().getBranchPredictions() == 1);
    assert(cpu.getStatistics().getBranchMispredictions() == 0);
    assert(cpu.getStatistics().getFlushes() == 0);
    assert(cpu.getStatistics().getBranchesNotTaken() == 1);
}

void testPipelinedInitiallyMispredictedTakenBranchFlushesWrongPath() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 9",
        "MOV R1 9",
        "CMP R0 R1",
        "JZ target",
        "MOV R2 99",
        "MOV R4 99",
        "target:",
        "MOV R3 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(2) == 0);
    assert(cpu.getRegisterValue(4) == 0);
    assert(cpu.getRegisterValue(3) == 7);
    assert(cpu.getStatistics().getBranchPredictions() == 1);
    assert(cpu.getStatistics().getBranchMispredictions() == 1);
    assert(cpu.getStatistics().getFlushes() == 1);
    assert(cpu.getStatistics().getBranchesTaken() == 1);
}

void testPipelinedLoopBranchPredictorLearnsTakenBehavior() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "MOV R1 0",
        "loop:",
        "ADDI R0 R0 -1",
        "CMP R0 R1",
        "JNZ loop",
        "MOV R3 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(0) == 0);
    assert(cpu.getRegisterValue(3) == 7);
    assert(cpu.getStatistics().getBranchPredictions() == 5);
    assert(cpu.getStatistics().getBranchesTaken() == 4);
    assert(cpu.getStatistics().getBranchesNotTaken() == 1);
    assert(cpu.getStatistics().getBranchMispredictions()
        < cpu.getStatistics().getBranchPredictions());
}

void testPipelinedJumpFlushesWrongPathInstructions() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R1 1",
        "JMP target",
        "MOV R2 99",
        "MOV R3 99",
        "target:",
        "MOV R4 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 1);
    assert(cpu.getRegisterValue(2) == 0);
    assert(cpu.getRegisterValue(3) == 0);
    assert(cpu.getRegisterValue(4) == 7);
}

void testPipelinedLoadStoreWithNops() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 42",
        "NOP",
        "NOP",
        "NOP",
        "STORE R0 10",
        "NOP",
        "NOP",
        "NOP",
        "LOAD R1 10",
        "NOP",
        "NOP",
        "NOP",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 42);
}

void testPipelinedHaltDrainsPipeline() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 5",
        "NOP",
        "NOP",
        "NOP",
        "ADDI R1 R0 1",
        "NOP",
        "NOP",
        "NOP",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.isHalted());
    assert(cpu.getRegisterValue(1) == 6);
}

void testHazardUnitRules() {
    IDEX writer;
    writer.valid = true;
    writer.instruction = Instruction(Opcode::MOV, 1, 0, 0, 9);
    writer.signals = ControlUnit::decode(writer.instruction);

    IFID addi;
    addi.valid = true;
    addi.instruction = Instruction(Opcode::ADDI, 4, 1, 0, 7);

    assert(HazardUnit::hasDataHazard(writer, addi));

    IFID store;
    store.valid = true;
    store.instruction = Instruction(Opcode::STORE, 0, 1, 0, 10);

    assert(HazardUnit::hasDataHazard(writer, store));

    IFID cmp;
    cmp.valid = true;
    cmp.instruction = Instruction(Opcode::CMP, 0, 2, 1, 0);

    assert(HazardUnit::hasDataHazard(writer, cmp));

    EXMEM exmem;
    exmem.valid = true;
    exmem.instruction = Instruction(Opcode::LOAD, 2, 0, 0, 10);
    exmem.signals = ControlUnit::decode(exmem.instruction);

    MEMWB memwb;

    IFID ifid;
    ifid.valid = true;
    ifid.instruction = Instruction(Opcode::SUB, 3, 2, 4, 0);

    assert(HazardUnit::hasDataHazard(IDEX{}, exmem, memwb, ifid));

    IDEX nonWriter;
    nonWriter.valid = true;
    nonWriter.instruction = Instruction(Opcode::CMP, 0, 1, 2, 0);
    nonWriter.signals = ControlUnit::decode(nonWriter.instruction);

    assert(!HazardUnit::hasDataHazard(nonWriter, addi));
}

void testForwardingUnitRules() {
    IDEX idex;
    idex.valid = true;
    idex.instruction = Instruction(Opcode::SUB, 3, 1, 2, 0);
    idex.signals = ControlUnit::decode(idex.instruction);

    EXMEM exmem;
    exmem.valid = true;
    exmem.instruction = Instruction(Opcode::ADD, 1, 4, 5, 0);
    exmem.signals = ControlUnit::decode(exmem.instruction);
    exmem.aluResult = 99;

    MEMWB memwb;
    memwb.valid = true;
    memwb.instruction = Instruction(Opcode::ADDI, 2, 0, 0, 4);
    memwb.signals = ControlUnit::decode(memwb.instruction);
    memwb.writeBackData = 4;

    const ForwardingDecision decision = ForwardingUnit::resolve(idex, exmem, memwb);

    assert(decision.forwardA == FROM_EXMEM);
    assert(decision.forwardB == FROM_MEMWB);

    exmem.instruction = Instruction(Opcode::LOAD, 1, 0, 0, 10);
    exmem.signals = ControlUnit::decode(exmem.instruction);

    assert(ForwardingUnit::resolve(idex, exmem, MEMWB{}).forwardA == NO_FORWARD);
}

void testPipelinedForwardingResolvesAddToSubDependency() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 10",
        "MOV R1 3",
        "ADD R2 R0 R1",
        "SUB R3 R2 R1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(2) == 13);
    assert(cpu.getRegisterValue(3) == 10);
}

void testStallUnitRules() {
    IDEX load;
    load.valid = true;
    load.instruction = Instruction(Opcode::LOAD, 1, 0, 0, 10);
    load.signals = ControlUnit::decode(load.instruction);

    IFID addi;
    addi.valid = true;
    addi.instruction = Instruction(Opcode::ADDI, 2, 1, 0, 5);

    assert(StallUnit::shouldStallForLoadUse(load, addi));

    IFID unrelated;
    unrelated.valid = true;
    unrelated.instruction = Instruction(Opcode::ADD, 2, 3, 4, 0);

    assert(!StallUnit::shouldStallForLoadUse(load, unrelated));
}

void testPipelinedLoadUseStallResolvesImmediateDependency() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "MOV R0 42",
        "STORE R0 10",
        "LOAD R1 10",
        "ADDI R2 R1 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 42);
    assert(cpu.getRegisterValue(2) == 43);
}

void testControlHazardUnitRules() {
    IDEX jump;
    jump.valid = true;
    jump.instruction = Instruction(Opcode::JMP, 0, 0, 0, 7);
    jump.signals = ControlUnit::decode(jump.instruction);

    ControlHazardDecision decision = ControlHazardUnit::resolve(jump, false);

    assert(decision.flush);
    assert(decision.redirectPc);
    assert(decision.targetPc == 7);

    IDEX branch;
    branch.valid = true;
    branch.instruction = Instruction(Opcode::JZ, 0, 0, 0, 4);
    branch.signals = ControlUnit::decode(branch.instruction);

    decision = ControlHazardUnit::resolve(branch, true);

    assert(!decision.flush);
    assert(!decision.redirectPc);

    decision = ControlHazardUnit::resolve(branch, false);

    assert(!decision.flush);
    assert(!decision.redirectPc);
}

int main() {
    testAssemblerMovParsing();
    testAssemblerAddParsing();
    testAssemblerAddiParsing();
    testAssemblerAndParsing();
    testAssemblerOrParsing();
    testAssemblerBranchParsing();
    testAssemblerNopParsing();
    testAssemblerLabelBranchParsing();
    testAssemblerAssembleLines();
    testAssemblerInvalidOpcodeThrows();
    testAssemblerProgramRunsOnCpu();
    testAssemblerAddSubFileRunsOnCpu();
    testAssemblerLoadStoreFileRunsOnCpu();
    testAssemblerLabelsFileRunsOnCpu();
    testAssemblerUnknownLabelThrows();
    testAssemblerDuplicateLabelThrows();

    testAddSub();
    testJZBranchTaken();
    testJNZBranchTaken();
    testLoadStore();
    testAddi();
    testAnd();
    testOr();
    testPipelinedWithNopsAvoidsHazard();
    testPipelinedLoadStoreWithNops();
    testPipelinedBranchTakenFlushesYoungerInstructions();
    testPipelinedCorrectlyPredictedNotTakenBranchDoesNotFlush();
    testPipelinedInitiallyMispredictedTakenBranchFlushesWrongPath();
    testPipelinedLoopBranchPredictorLearnsTakenBehavior();
    testPipelinedJumpFlushesWrongPathInstructions();
    testPipelinedHaltDrainsPipeline();
    testHazardUnitRules();
    testForwardingUnitRules();
    testPipelinedForwardingResolvesAddToSubDependency();
    testStallUnitRules();
    testPipelinedLoadUseStallResolvesImmediateDependency();
    testControlHazardUnitRules();

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
