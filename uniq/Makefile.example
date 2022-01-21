#
# Simple project Makefile
#
CC=gcc
CFLAGS=-g -Wall -Werror -O2
PROGRAM=uniq
SOURCES=uniq.c

#
#  You don't need to change anything below this point
#
OBJECTS=$(SOURCES:.c=.o)

all: $(PROGRAM)
$(PROGRAM): $(OBJECTS)
	$(CC) -g -o $(PROGRAM) $(OBJECTS)

clean:
	/bin/rm -f *.o core $(PROGRAM)
