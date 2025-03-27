# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# Source files
SRC = main.cpp registers.cpp alu.cpp control.cpp
SRC_NOFWD = pipeline.cpp
SRC_FWD = pipeline.cpp

# Object files
OBJ = $(SRC:.cpp=.o)
OBJ_NOFWD = $(SRC_NOFWD:.cpp=.o)
OBJ_FWD = $(SRC_FWD:.cpp=.o)

# Targets
all: noforward forward

# Build noforward executable
noforward: $(OBJ) $(OBJ_NOFWD)
	$(CXX) $(CXXFLAGS) -o noforward $(OBJ) $(OBJ_NOFWD)

# Build forward executable
forward: $(OBJ) $(OBJ_FWD)
	$(CXX) $(CXXFLAGS) -o forward $(OBJ) $(OBJ_FWD)

# Pattern rule to compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean generated files
clean:
	rm -f *.o noforward forward
