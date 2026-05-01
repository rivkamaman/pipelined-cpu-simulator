CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -MMD -MP -Iinclude

TARGET := cpu_simulator
TEST_TARGET := cpu_tests

BUILD_DIR := build
TEST_BUILD_DIR := build_tests

SRC := $(wildcard src/*.cpp)
TEST_SRC := $(filter-out src/main.cpp,$(SRC)) tests/tests.cpp

OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
TEST_OBJ := $(patsubst src/%.cpp,$(TEST_BUILD_DIR)/%.o,$(filter-out src/main.cpp,$(SRC)))
TEST_OBJ += $(TEST_BUILD_DIR)/tests.o

DEP := $(OBJ:.o=.d)
TEST_DEP := $(TEST_OBJ:.o=.d)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_BUILD_DIR)/%.o: src/%.cpp | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_BUILD_DIR)/tests.o: tests/tests.cpp | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) $(TARGET) $(TEST_TARGET)

-include $(DEP)
-include $(TEST_DEP)