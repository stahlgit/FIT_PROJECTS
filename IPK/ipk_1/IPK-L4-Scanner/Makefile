# === Variables ===
BUILD_DIR 	= build
SRC_DIR   	= src
TEST_DIR  	= tests
TARGET    	= ipk-L4-scan
TAR       	= xstahl01.tar
TEST_TARGET = test_scanner

# === Colors ===
RED    = \033[0;31m
GREEN  = \033[0;32m
YELLOW = \033[1;33m
BLUE   = \033[0;34m
NC     = \033[0m

# === Compiler settings ===
CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -g
LDFLAGS  = -lpcap

# Auto-discover all subdirectories under src/ for includes
INCLUDE_DIRS = $(shell find $(SRC_DIR) -type d)
CXXFLAGS    += $(addprefix -I, $(INCLUDE_DIRS))

# === Source & object files ===
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_OBJ_FILES = $(filter-out $(BUILD_DIR)/main.o, $(OBJ_FILES))

# === Phony targets ===
.PHONY: all clean tar test NixDevShellName


# === Build rules ===
all: $(BUILD_DIR) $(TARGET)
	@printf "$(GREEN)Build complete: ./$(TARGET)$(NC)\n"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ_FILES)
	@printf "$(BLUE)Linking $(TARGET)...$(NC)\n"
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(BLUE)Compiling $<...$(NC)\n"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Nix devShell name (required by guidelines) ===
NixDevShellName:
	@echo "c"

# === Clean ===
clean:
	@printf "$(YELLOW)Cleaning...$(NC)\n"
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@rm -f $(TEST_TARGET)
	@printf "$(GREEN)Clean complete$(NC)\n"

# === Package (for submission) ===
tar: clean
	tar -cf $(TAR) Makefile $(SRC_DIR) $(TEST_DIR) README.md CHANGELOG.md LICENSE docs

#=== Tests ===
$(TEST_TARGET): $(TEST_DIR)/test_scanner.cpp $(TEST_OBJ_FILES) $(TEST_DIR)/catch2_include/catch_amalgamated.cpp
	@printf "$(BLUE)Compiling and linking tests...$(NC)\n"
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test: all $(TEST_TARGET)
	./$(TEST_TARGET)

# === Quick DNS smoke test ===
dns_test: $(BUILD_DIR)/dns/dns_resolver.o $(BUILD_DIR)/dns_test.o
	@printf "$(BLUE)Linking dns_test...$(NC)\n"
	$(CXX) $^ -o dns_test $(LDFLAGS)

$(BUILD_DIR)/dns_test.o: tests/dns_test.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@