CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra

SRC_DIR := src
BUILD_DIR:= build
INCLUDE_DIR := include
DOCS_DIR := docs/media

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
HDRS := $(shell find $(INCLUDE_DIR) -type f -name "*.hpp")
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET := ipk25chat-client
ARCHIVE := xstahl01.zip

# Test configuration
TEST_SRC_DIR := tests
TEST_TARGET := $(TARGET)-tests
TEST_SRCS := $(shell find $(TEST_SRC_DIR) -type f -name "*.cpp")
TEST_OBJS := $(patsubst $(TEST_SRC_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

PROD_OBJS := $(filter-out $(BUILD_DIR)/main.o, $(OBJS))
# Google Test linker flags
LDLIBS := -lgtest -lgtest_main -pthread


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@ 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS) | create_obj_dir
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@ 

.PHONY: create_obj_dir
create_obj_dir:
	mkdir -p $(dir $(OBJS))

#.PHONY: clean
#clean:
#	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: pack
pack:
	find $(SRC_DIR) $(INCLUDE_DIR) $(DOCS_DIR) -type f -not -path '*/\.*' -not -name '*.DS_Store' | zip -@ $(ARCHIVE) $(SRC_DIR) $(INCLUDE_DIR) $(DOCS_DIR) Makefile README.md CHANGELOG.md LICENSE


# Test build rules
$(BUILD_DIR)/tests/%.o: $(TEST_SRC_DIR)/%.cpp $(HDRS) | create_test_dir
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

.PHONY: create_test_dir
create_test_dir:
	mkdir -p $(BUILD_DIR)/tests

# Test executable
$(TEST_TARGET): $(PROD_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDLIBS)

# Test run command
.PHONY: test
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Update clean target
.PHONY: clean
clean:
	rm -rf $(TARGET) $(TEST_TARGET) $(BUILD_DIR) $(ARCHIVE)


.DEFAULT_GOAL := all

