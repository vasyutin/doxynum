# Doxynum makefile written by Sergey Vasyutin

CC=g++
CFLAGS=-c -O3 -I ../../src
LDFLAGS=
SOURCES=../../src/main.cpp ../../src/refdb.cpp ../../src/v8regexp.c
OBJECTS=../../src/main.o ../../src/refdb.o ../../src/v8regexp.o
EXECUTABLE=bin/doxynum

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p bin
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ../../src/*.o $(EXECUTABLE) bin

install:
	cp $(EXECUTABLE) /usr/bin
