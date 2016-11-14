IDIR = ../include
CC = gcc
CFLAGS = -g -I$(IDIR)
ODIR=obj

_DEPS = includes.h defines.h structs.h prototypes.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rsa_buffer_utils.o rsa_network_helper.o client.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: rsa_mem_client

rebuild: clean all

rsa_mem_client: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f rsa_mem_client $(OBJ)

