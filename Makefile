MAKEFLAGS = -j$(nproc)

CXX=clang++
CXXFLAGS=-Wall -Wextra -Wpedantic --std=c++2c -I./include -I/usr/include/CLI

SRC_DIR=src
SUB_SRC_DIR=src/subcommand_actions
BUILD_DIR=build

# Generate object files from source files
OBJS_SRC=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp))
OBJS_SUB=$(patsubst $(SUB_SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(wildcard $(SUB_SRC_DIR)/*.cpp))
OBJS=$(OBJS_SRC) $(OBJS_SUB)

TARGET=$(BUILD_DIR)/catalyst

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lyaml-cpp -fuse-ld=mold

# Build object files from src directory sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build object files from subcommand_actions directory sources
$(BUILD_DIR)/%.o: $(SUB_SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
