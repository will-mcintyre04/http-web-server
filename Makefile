CC = gcc
EXEC = main
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# Find all .c files
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Uses Make's pattern substitution to generate a list of object files from source files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Flags to run during the gcc commands
# -g: generate debug information
# -Wall: enables all warnings
# -fPIC: generate machine code that can run at any memory address
FLAGS = -g -Wall -fPIC -I$(INCLUDE_DIR)

# Create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Create the build directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR) 

# The final executable
# Target is the final executable
# Depends on the object files
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
