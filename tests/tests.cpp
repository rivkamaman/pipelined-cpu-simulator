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

void testAssemblerAddParsing() {
    const Instruction instruction = Assembler::parseLine("ADD R2 R0 R1");

    assert(instruction.opcode == Opcode::ADD);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getSrc2() == 1);
    assert(instruction.toString() == "ADD R2,R0,R1");
}

void testAssemblerAddiParsing() {
    const Instruction instruction = Assembler::parseLine("ADDI R2 R0 5");

    assert(instruction.opcode == Opcode::ADDI);
    assert(instruction.dst == 2);
    assert(instruction.getSrc1() == 0);
    assert(instruction.getImmediate() == 5);
    assert(instruction.toString() == "ADDI R2,R0,5");
}

void testAssemblerMemoryParsing() {
    const Instruction load = Assembler::parseLine("LW R1,12(R2)");
    assert(load.opcode == Opcode::LW);
    assert(load.dst == 1);
    assert(load.getSrc1() == 2);
    assert(load.getImmediate() == 12);
    assert(load.toString() == "LW R1,12(R2)");

    const Instruction store = Assembler::parseLine("SW R3,8(R4)");
    assert(store.opcode == Opcode::SW);
    assert(store.getSrc1() == 3);
    assert(store.getSrc2() == 4);
    assert(store.getImmediate() == 8);
    assert(store.toString() == "SW R3,8(R4)");
}

void testAssemblerBranchAndJumpParsing() {
    const Instruction beq = Assembler::parseLine("BEQ R2,R3,8");
    assert(beq.opcode == Opcode::BEQ);
    assert(beq.getSrc1() == 2);
    assert(beq.getSrc2() == 3);
    assert(beq.getImmediate() == 8);
    assert(beq.toString() == "BEQ R2,R3,8");

    const std::vector<std::string> lines = {
        "BNE R4,R5,loop",
        "J done",
        "loop:",
        "NOP",
        "done:",
        "HALT"
    };
    const std::vector<Instruction> program = Assembler::assembleLines(lines);

    assert(program[0].opcode == Opcode::BNE);
    assert(program[0].getImmediate() == 2);
    assert(program[1].opcode == Opcode::J);
    assert(program[1].getImmediate() == 3);
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

void testAssemblerUnknownLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"J missing", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testAssemblerDuplicateLabelThrows() {
    bool threw = false;

    try {
        Assembler::assembleLines({"again:", "ADDI R0 R0 1", "again:", "HALT"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void testInstructionSemantics() {
    const Instruction add(Opcode::ADD, 1, 2, 3, 0);
    assert(add.writesRegister());
    assert(add.readsSrc1());
    assert(add.readsSrc2());
    assert(add.getFormat() == InstructionFormat::RType);

    const Instruction addi(Opcode::ADDI, 1, 2, 0, 4);
    assert(addi.writesRegister());
    assert(addi.readsSrc1());
    assert(!addi.readsSrc2());
    assert(addi.getFormat() == InstructionFormat::IType);

    const Instruction load(Opcode::LW, 1, 2, 0, 4);
    assert(load.writesRegister());
    assert(load.readsSrc1());
    assert(!load.readsSrc2());
    assert(load.isMemoryRead());

    const Instruction store(Opcode::SW, 0, 1, 2, 4);
    assert(!store.writesRegister());
    assert(store.readsSrc1());
    assert(store.readsSrc2());
    assert(store.isMemoryWrite());

    const Instruction branch(Opcode::BEQ, 0, 1, 2, 4);
    assert(branch.isBranch());
    assert(branch.getFormat() == InstructionFormat::IType);

    const Instruction jump(Opcode::J, 0, 0, 0, 4);
    assert(jump.isJump());
    assert(jump.getFormat() == InstructionFormat::JType);
}

void testSequentialAluProgram() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 5",
        "ADDI R2 R0 3",
        "ADD R3 R1 R2",
        "SUB R4 R1 R2",
        "AND R5 R1 R2",
        "OR R6 R1 R2",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.run();

    assert(cpu.getRegisterValue(3) == 8);
    assert(cpu.getRegisterValue(4) == 2);
    assert(cpu.getRegisterValue(5) == 1);
    assert(cpu.getRegisterValue(6) == 7);
}

void testSequentialLoadStoreUsesBaseRegister() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 42",
        "ADDI R2 R0 10",
        "SW R1,5(R2)",
        "LW R3,5(R2)",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.run();

    assert(cpu.getRegisterValue(3) == 42);
}

void testSequentialBranchesAndJump() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 7",
        "ADDI R2 R0 7",
        "BEQ R1,R2,equal",
        "ADDI R3 R0 -1",
        "J done",
        "equal:",
        "ADDI R3 R0 9",
        "done:",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.run();

    assert(cpu.getRegisterValue(3) == 9);
}

void testPipelinedWithNopsAvoidsHazard() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 5",
        "NOP",
        "NOP",
        "NOP",
        "ADDI R2 R1 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(2) == 6);
}

void testPipelinedForwardingResolvesAddToSubDependency() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 10",
        "ADDI R2 R0 3",
        "ADD R3 R1 R2",
        "SUB R4 R3 R2",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(3) == 13);
    assert(cpu.getRegisterValue(4) == 10);
}

void testPipelinedStoreUsesForwardedValueAndBase() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 42",
        "ADDI R2 R0 10",
        "SW R1,5(R2)",
        "LW R3,5(R2)",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(3) == 42);
}

void testPipelinedLoadUseStallResolvesImmediateDependency() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 42",
        "SW R1,10(R0)",
        "LW R2,10(R0)",
        "ADDI R3 R2 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(2) == 42);
    assert(cpu.getRegisterValue(3) == 43);
    assert(cpu.getStatistics().getFlushes() == 0);
}

void testPipelinedBEQUsesForwardedOperands() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 7",
        "ADDI R2 R0 7",
        "BEQ R1,R2,target",
        "ADDI R3 R0 99",
        "target:",
        "ADDI R4 R0 1",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(3) == 0);
    assert(cpu.getRegisterValue(4) == 1);
    assert(cpu.getStatistics().getBranchPredictions() == 1);
    assert(cpu.getStatistics().getBranchMispredictions() == 1);
}

void testPipelinedBEQNotTakenDoesNotFlush() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 4",
        "ADDI R2 R0 5",
        "BEQ R1,R2,target",
        "ADDI R3 R0 3",
        "target:",
        "ADDI R4 R0 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(3) == 3);
    assert(cpu.getRegisterValue(4) == 7);
    assert(cpu.getStatistics().getCorrectBranchPredictions() == 1);
    assert(cpu.getStatistics().getBranchMispredictions() == 0);
    assert(cpu.getStatistics().getFlushes() == 0);
}

void testPipelinedLoopBranchPredictorLearnsTakenBehavior() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 5",
        "ADDI R2 R0 0",
        "loop:",
        "ADDI R1 R1 -1",
        "BNE R1,R2,loop",
        "ADDI R3 R0 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 0);
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
        "ADDI R1 R0 1",
        "J target",
        "ADDI R2 R0 99",
        "ADDI R3 R0 99",
        "target:",
        "ADDI R4 R0 7",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.getRegisterValue(1) == 1);
    assert(cpu.getRegisterValue(2) == 0);
    assert(cpu.getRegisterValue(3) == 0);
    assert(cpu.getRegisterValue(4) == 7);
    assert(cpu.getStatistics().getBranchPredictions() == 1);
    assert(cpu.getStatistics().getBranchMispredictions() == 1);
    assert(cpu.getStatistics().getFlushes() == 1);
}

void testPipelinedHaltDrainsPipeline() {
    CPU cpu;

    const std::vector<std::string> lines = {
        "ADDI R1 R0 5",
        "NOP",
        "NOP",
        "NOP",
        "ADDI R2 R1 1",
        "NOP",
        "NOP",
        "NOP",
        "HALT"
    };

    cpu.loadProgram(Assembler::assembleLines(lines));
    cpu.runPipelined();

    assert(cpu.isHalted());
    assert(cpu.getRegisterValue(2) == 6);
}

void testHazardUnitRules() {
    IDEX writer;
    writer.valid = true;
    writer.instruction = Instruction(Opcode::ADDI, 1, 0, 0, 9);
    writer.signals = ControlUnit::decode(writer.instruction);

    IFID addi;
    addi.valid = true;
    addi.instruction = Instruction(Opcode::ADDI, 4, 1, 0, 7);

    assert(HazardUnit::hasDataHazard(writer, addi));

    IFID store;
    store.valid = true;
    store.instruction = Instruction(Opcode::SW, 0, 1, 2, 10);

    assert(HazardUnit::hasDataHazard(writer, store));

    IDEX branchWriter;
    branchWriter.valid = true;
    branchWriter.instruction = Instruction(Opcode::ADDI, 2, 0, 0, 4);
    branchWriter.signals = ControlUnit::decode(branchWriter.instruction);

    IFID branch;
    branch.valid = true;
    branch.instruction = Instruction(Opcode::BEQ, 0, 2, 1, 4);

    assert(HazardUnit::hasDataHazard(branchWriter, branch));
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

    exmem.instruction = Instruction(Opcode::LW, 1, 0, 0, 10);
    exmem.signals = ControlUnit::decode(exmem.instruction);

    assert(ForwardingUnit::resolve(idex, exmem, MEMWB{}).forwardA == NO_FORWARD);
}

void testStallUnitRules() {
    IDEX load;
    load.valid = true;
    load.instruction = Instruction(Opcode::LW, 1, 0, 0, 10);
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

void testControlHazardUnitRules() {
    IDEX jump;
    jump.valid = true;
    jump.instruction = Instruction(Opcode::J, 0, 0, 0, 7);
    jump.signals = ControlUnit::decode(jump.instruction);

    const ControlHazardDecision decision = ControlHazardUnit::resolve(jump);

    assert(decision.flush);
    assert(decision.redirectPc);
    assert(decision.targetPc == 7);
}

int main() {
    testAssemblerAddParsing();
    testAssemblerAddiParsing();
    testAssemblerMemoryParsing();
    testAssemblerBranchAndJumpParsing();
    testAssemblerInvalidOpcodeThrows();
    testAssemblerUnknownLabelThrows();
    testAssemblerDuplicateLabelThrows();
    testInstructionSemantics();

    testSequentialAluProgram();
    testSequentialLoadStoreUsesBaseRegister();
    testSequentialBranchesAndJump();

    testPipelinedWithNopsAvoidsHazard();
    testPipelinedForwardingResolvesAddToSubDependency();
    testPipelinedStoreUsesForwardedValueAndBase();
    testPipelinedLoadUseStallResolvesImmediateDependency();
    testPipelinedBEQUsesForwardedOperands();
    testPipelinedBEQNotTakenDoesNotFlush();
    testPipelinedLoopBranchPredictorLearnsTakenBehavior();
    testPipelinedJumpFlushesWrongPathInstructions();
    testPipelinedHaltDrainsPipeline();

    testHazardUnitRules();
    testForwardingUnitRules();
    testStallUnitRules();
    testControlHazardUnitRules();

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
