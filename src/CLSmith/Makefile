# Makefile for the CLSmith. Only handles files found int src/CLSmith/. Before
# calling this, you must first call make in src/ and then delete
# RandomProgramGenerator.o (for now at least).

CC=g++
CFLAGS=-c -Wall -I../ -std=c++0x -g
LFLAGS=../*.o -std=c++0x
SOURCES=CLOutputMgr.cpp CLProgramGenerator.cpp Globals.cpp CLRandomProgramGenerator.cpp
OBJS=$(SOURCES:.cpp=.o)
BIN=CLSmith

all: generator launcher

generator: $(SOURCES) $(BIN)

launcher:
	gcc -I/homes/$(USER)/OpenCL/ cl_launcher.c -o launcher -lOpenCL

$(BIN): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
