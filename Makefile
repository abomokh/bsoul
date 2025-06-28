# Makefile for NIC Simulator Project

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# Target executable
TARGET = nic_sim.exe

# Source files
SOURCES = main.cpp NIC_sim.cpp L2.cpp L3.cpp L4.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = common.hpp packets.hpp NIC_sim.hpp L2.h L3.h L4.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
main.o: main.cpp NIC_sim.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

NIC_sim.o: NIC_sim.cpp NIC_sim.hpp common.hpp packets.hpp L2.h L3.h L4.h
	$(CXX) $(CXXFLAGS) -c NIC_sim.cpp

L2.o: L2.cpp L2.h packets.hpp common.hpp
	$(CXX) $(CXXFLAGS) -c L2.cpp

L3.o: L3.cpp L3.h L4.h packets.hpp common.hpp
	$(CXX) $(CXXFLAGS) -c L3.cpp

L4.o: L4.cpp L4.h packets.hpp common.hpp
	$(CXX) $(CXXFLAGS) -c L4.cpp

# Clean target
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run target (example usage)
run: $(TARGET)
	./$(TARGET) param_file.txt packet_file.txt

# Phony targets
.PHONY: all clean run 