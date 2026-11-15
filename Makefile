CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDLIBS = -lm
TARGET = streamtree
SOURCES = src/main.c src/stream_gen.c src/stats.c src/hoeffding_tree.c src/prequential.c src/drift.c
HEADERS = include/stream_gen.h include/stats.h include/hoeffding_tree.h include/prequential.h include/drift.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean
