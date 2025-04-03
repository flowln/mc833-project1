COMMON_SRCS := data.c
CLIENT_SRCS := main.c
SERVER_SRCS := main.c

.PHONY: all
all: clean build

build-client:
	gcc $(addprefix src/client/, $(CLIENT_SRCS)) $(addprefix src/common/, $(COMMON_SRCS)) -I src/ -std=c99 -lc -lsqlite3 -g -O0 -o bin/client

build-server:
	gcc $(addprefix src/server/, $(SERVER_SRCS)) $(addprefix src/common/, $(COMMON_SRCS)) -I src/ -std=c99 -lc -lsqlite3 -g -O0 -o bin/server

.PHONY: build
build: build-client build-server

clean-client:
	-rm bin/client

clean-server:
	-rm bin/server

.PHONY: clean
clean: clean-client clean-server
