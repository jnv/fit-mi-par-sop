CC=mpiCC
CFLAGS=-Wall -Wno-long-long -pedantic -ansi -O3 -ggdb
LDFLAGS=
SOURCES:=$(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLES=par sekv

default: all

all: sekv par
	
sekv: $(OBJECTS)
	$(CC) $(filter-out src/par.o, $(OBJECTS)) $(LDFLAGS) -o sekv
	
par: $(OBJECTS)
	$(CC) $(filter-out src/sekv.o, $(OBJECTS)) $(LDFLAGS) -o par
	
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) $(EXECUTABLES)
