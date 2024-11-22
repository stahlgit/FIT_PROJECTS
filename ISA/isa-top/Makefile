BUILD_DIR = build
SRC_DIR = src
TESTS_DIR = tests
TARGET = isa-top
CXX = g++
CXXFLAGS = -std=c++17 -pthread
GTEST_LIB = -lgtest -lgtest_main
OBJ_FILES = $(SRC_DIR)/utils.cpp $(SRC_DIR)/stats.cpp
TAR = xstahl01.tar
TAR_FILES = CMakeLists.txt Makefile README.md $(SRC_DIR) $(TESTS_DIR) manual.pdf

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(BUILD_DIR)
	@echo "Building $(TARGET)..."
	cd $(BUILD_DIR) && cmake .. && cmake --build . 
	mv $(BUILD_DIR)/$(TARGET) ./
	sudo setcap cap_net_raw=eip ./$(TARGET)

tests:
	$(CXX) $(CXXFLAGS) -o test_main $(TESTS_DIR)/test_main.cpp $(OBJ_FILES) $(GTEST_LIB)
	$(CXX) $(CXXFLAGS) -o test_stats $(TESTS_DIR)/test_stats.cpp $(OBJ_FILES) $(GTEST_LIB)
	./test_main
	./test_stats
	rm -f test_main test_stats

clean:
	rm -rf $(BUILD_DIR) $(TARGET) test_main test_stats

pack:clean
	tar -cvf  $(TAR) $(TAR_FILES)