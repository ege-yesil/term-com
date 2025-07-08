CC = gcc
SRC = src
BIN = bin

SERVER := $(addprefix $(SRC)/, server.c util.c commands.c)
CLIENT := $(addprefix $(SRC)/, client.c util.c)

all: server client

server: $(BIN)/server
client: $(BIN)/client

$(BIN)/server: $(SERVER)
	@mkdir -p $(BIN)
	$(CC) $^ -o $@

$(BIN)/client: $(CLIENT)
	@mkdir -p $(BIN)
	gcc $^ -o $@

clean: $(BIN)
	@rm -rf $(BIN)
	@mkdir -p $(BIN)
