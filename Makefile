# Name of the output executable
TARGET = http_server

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Source files
SRCS = http_server.cpp

# Default target: Build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up: Remove the compiled files
clean:
	rm -f $(TARGET)