# Compiler settings
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -Iinclude

# Source files and Output binary
SRCS     = src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp
TARGET   = memsim

# Default target (what runs when you type 'make')
all: $(TARGET)

# Rule to link the program
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(INCLUDES) -o $(TARGET)

# Rule to clean up build files (type 'make clean')
clean:
	rm -f $(TARGET)
	rm -f *.o

# Phony targets prevent conflicts with files of the same name
.PHONY: all clean