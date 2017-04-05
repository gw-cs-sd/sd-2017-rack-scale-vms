
CC = gcc
CFLAGS = -c -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE -g -O0
LDFLAGS =

SOURCES = mem-event.c
OBJ = $(SOURCES:.c=.o)
OBJECTS = obj/$(OBJ)

EXECUTABLE = mem-event

# Using pkg-config to link LibVMI into this application
CFLAGS += `pkg-config --cflags`
LDFLAGS += `pkg-config --libs glib-2.0 libvmi`

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o obj/$@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
