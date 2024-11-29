CC=gcc
CFLAGS=-Wall -Wextra
BIN_DIR=bin
SRC_DIR=src
INCLUDE_DIR=includes
SRC=$(SRC_DIR)/main.c
LIB=
OBJ=$(addprefix $(OBJ_DIR), $(LIB:.c=.o))
EXE=$(BIN_DIR)/$(shell basename $(SRC:.c=))

ifeq ($(DEBUG),1)
    CFLAGS += -g -DDEBUG
else
    CFLAGS += -O2
endif

.PHONY: clean run

all: clean $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) $(SRC) $(shell find $(BIN_DIR)/*.o) -o $@

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $(BIN_DIR)/$(shell basename $@)

clean:
	rm -rf $(EXE) $(BIN_DIR)
	mkdir $(BIN_DIR)
