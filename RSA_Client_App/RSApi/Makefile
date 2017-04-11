CC = gcc
CFLAGS = -g -I./ -Wall -Werror

ODIR = build
RSApi_DEPS = includes.h defines.h structs.h prototypes.h
_RSApi_OBJ = rsa_buffer_utils.o rsa_network_helper.o rsa_api.o
RSApi_OBJ = $(patsubst %,$(ODIR)/%,$(_RSApi_OBJ))
RSApi_LIB = $(ODIR)/librsa.a

$(ODIR)/%.o: %.c $(RSApi_DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all
all: rsa_api

rebuild: clean all

rsa_api: $(RSApi_OBJ)
	ar rcs $(RSApi_LIB) $(RSApi_OBJ)

.PHONY: clean
clean:
	rm -f $(RSApi_OBJ)
	rm -f $(RSApi_LIB)

