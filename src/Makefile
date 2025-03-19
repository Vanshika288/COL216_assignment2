# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directories
SRC_DIR = src
BIN_DIR = bin

# Executable names
NOFWD_EXEC = noforward
FWD_EXEC = forward

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/pipeline.cpp $(SRC_DIR)/control.cpp $(SRC_DIR)/alu.cpp $(SRC_DIR)/registers.cpp
OBJS = $(SRCS:.cpp=.o)

# Header files
HEADERS = $(SRC_DIR)/pipeline.hpp $(SRC_DIR)/control.hpp $(SRC_DIR)/alu.hpp $(SRC_DIR)/registers.hpp

# Build non-forwarding executable
$(NOFWD_EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NOFWD_EXEC) $(OBJS)

# Build forwarding executable with FORWARD flag enabled
$(FWD_EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -DFORWARDING_ENABLED -o $(FWD_EXEC) $(OBJS)

# Compile .cpp to .o
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build
clean:
	rm -f $(SRC_DIR)/*.o $(NOFWD_EXEC) $(FWD_EXEC)

# Run non-forwarding simulator
run-noforward: $(NOFWD_EXEC)
	./$(NOFWD_EXEC) ../inputfiles/strlen.txt 20

# Run forwarding simulator
run-forward: $(FWD_EXEC)
	./$(FWD_EXEC) ../inputfiles/strlen.txt 20

# Default target
all: $(NOFWD_EXEC) $(FWD_EXEC)

.PHONY: all clean run-noforward run-forward
