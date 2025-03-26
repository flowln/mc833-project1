CLIENT_SRCS := main.c
SERVER_SRCS := main.c

.PHONY: all
all: clean build

build-client:
	gcc $(addprefix src/client/, $(CLIENT_SRCS)) -I src/ -o bin/client

build-server:
	gcc $(addprefix src/server/, $(SERVER_SRCS)) -I src/ -o bin/server

.PHONY: build
build: build-client build-server

clean-client:
	-rm bin/client

clean-server:
	-rm bin/server

.PHONY: clean
clean: clean-client clean-server