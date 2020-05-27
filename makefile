CC = gcc

SRC_DIR = ./bin2c
OBJECTS = $(SRC_DIR)/main.o 
BIN_PATH = ./bin/bin2c
CFLAGS = -Wall -std=c99

$(BIN_PATH):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ 

clean: 
	rm -rf $(objects)
