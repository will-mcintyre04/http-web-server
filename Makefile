CC = gcc
EXEC = main
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# Find all .c files
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Uses Make's pattern substitution to generate a list of object files from source files
# Does not create these files though
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Flags to run during the gcc commands
# -g: generate debug information
# -Wall: enables all warnings
# -fPIC: generate machine code that can run at any memory address
# -I: tells the compiler to look in a directory for header files
FLAGS = -g -Wall -fPIC -I$(INCLUDE_DIR)

# All target that depends on the creation of bin/build folders and 
all: $(BIN_DIR) $(OBJ_DIR) $(BIN_DIR)/$(EXEC)

# Create the bin and build directories if they don't exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# The final executable
# Target is the final executable
# Depends on the object files (needs them to be newer than the final executable)
# Then links the object files together to produce executable
$(BIN_DIR)/$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) -o $(BIN_DIR)/$(EXEC)

# Compile .c files into .o files inside the build directory
# The targets are object files in the build directory
# Each object file depends on its corresponding source file
# $< refers to the source file and $@ is the target object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(FLAGS) $< -o $@

# Run the executable 
# The run target depends on the final executable
run: $(BIN_DIR)/$(EXEC)
	./$(BIN_DIR)/$(EXEC)

# Clean up: remove .o files and the executable inside the bin directory
clean:
	-rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/$(EXEC)
