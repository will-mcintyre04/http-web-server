CC = gcc
CFLAGS = -Iinclude
EXEC = main
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# Find all .c files and generate a list of .o files in the build directory
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

FLAGS = -g -Wall -lm -ldl -fPIC -rdynamic -I$(INCLUDE_DIR)

# Create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# The final executable
$(BIN_DIR)/$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) -o $(BIN_DIR)/$(EXEC)

# Compile .c files into .o files inside the build directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(FLAGS) $< -o $@

# Run the executable
run: $(BIN_DIR)/$(EXEC)
	./$(BIN_DIR)/$(EXEC)

# Clean up: remove .o files and the executable inside the bin directory
clean:
	-rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/$(EXEC)
