# Sorting Visualizer in C
#
# Targets:
#   make            build the sortviz binary
#   make run        build, record frames and render all GIFs
#   make clean      remove the binary and generated data

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror -O2
SRC     = src/main.c src/sorts.c src/recorder.c
BIN     = sortviz

all: $(BIN)

$(BIN): $(SRC) src/sorts.h src/recorder.h
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	mkdir -p data assets
	./$(BIN)
	python3 scripts/render_gif.py

clean:
	rm -f $(BIN)
	rm -rf data

.PHONY: all run clean
