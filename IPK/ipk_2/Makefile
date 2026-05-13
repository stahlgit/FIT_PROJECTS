# === Variables ===
BUILD_DIR 	= build
SRC_DIR   	= src
TEST_DIR  	= tests
TARGET    	= ipk-rdt
TAR       	= xstahl01.tar
ZIP       	= xstahl01.zip
TEST_TARGET = test_rdt

# === Colors ===
RED    = \033[0;31m
GREEN  = \033[0;32m
YELLOW = \033[1;33m
BLUE   = \033[0;34m
NC     = \033[0m

# === Compiler settings ===
CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -g
LDFLAGS  =

# Auto-discover all subdirectories under src/ for includes
INCLUDE_DIRS = $(shell find $(SRC_DIR) -type d)
CXXFLAGS    += $(addprefix -I, $(INCLUDE_DIRS))

# === Source & object files ===
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_OBJ_FILES = $(filter-out $(BUILD_DIR)/main.o, $(OBJ_FILES))

# === Phony targets ===
.PHONY: all build_target clean tar zip test e2e netem speed NixDevShellName


# === Build rules ===
# all: default target - compile and link ipk-rdt
all: $(BUILD_DIR) $(TARGET)
	@printf "$(GREEN)Build complete: ./$(TARGET)$(NC)\n"

# build_target: alias for all
build_target: all

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
	@rm -f $(TEST_TARGET) test_rdt
	@printf "$(GREEN)Clean complete$(NC)\n"

# === Package (for submission) ===
tar: clean
	tar -cf $(TAR) Makefile $(SRC_DIR) $(TEST_DIR) README.md CHANGELOG.md LICENSE docs

zip: clean
	@rm -f $(ZIP)
	@zip -r $(ZIP) Makefile README.md CHANGELOG.md LICENSE $(SRC_DIR) $(TEST_DIR)

#=== Tests ===
TEST_SRCS = $(TEST_DIR)/test_packet.cpp \
            $(TEST_DIR)/test_send_window.cpp \
            $(TEST_DIR)/test_receive_buffer.cpp \
            $(TEST_DIR)/test_session.cpp \
            $(TEST_DIR)/test_dns_resolver.cpp \
            $(TEST_DIR)/test_udp_socket.cpp \
            $(TEST_DIR)/catch2_include/catch_amalgamated.cpp

$(TEST_TARGET): $(TEST_SRCS) $(TEST_OBJ_FILES)
	@printf "$(BLUE)Compiling and linking tests...$(NC)\n"
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

# === End-to-end integration tests ===
e2e: $(TARGET)
	@bash $(TEST_DIR)/e2e.sh

# === Network impairment tests (requires root / sudo) ===
netem: $(TARGET)
	@bash $(TEST_DIR)/netem.sh

# === Throughput / speed tests (requires root / sudo for impairment sections) ===
speed: $(TARGET)
	@bash $(TEST_DIR)/speed.sh