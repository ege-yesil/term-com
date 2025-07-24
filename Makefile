CC = gcc
SRC = src
BIN = bin

SERVER := src/server/server.c src/server/serverModes.c src/util.c src/commands.c
CLIENT := src/client/client.c src/util.c src/commands.c

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
