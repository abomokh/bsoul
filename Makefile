# Makefile for NIC Simulator Project
# Compiles to nic_sim.exe using CC for C++ files and CXXFLAGS

# Compiler
CC = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# Target executable
TARGET = nic_sim.exe

# Source files
SOURCES = main.cpp NIC_sim.cpp L2.cpp L3.cpp L4.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run tests
test0: $(TARGET)
	./$(TARGET) test0_param.in test0_packets.in

test1: $(TARGET)
	./$(TARGET) test1_param.in test1_packets.in

test2: $(TARGET)
	./$(TARGET) test2_param.in test2_packets.in

# Phony targets
.PHONY: all clean test0 test1 test2 