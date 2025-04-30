SRC_DIR = src
INCLUDE_DIR = src/include
BUILD_DIR = build

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Executable name
TARGET = ipk-l2l3-scan

# Docker network
DOCKER_NETWORK = test_network

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

docker_run:
	docker-compose up --abort-on-container-exit

.PHONY: all clean docker_clean