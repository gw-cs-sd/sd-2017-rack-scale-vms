CC = gcc
CFLAGS = -g -O0 -Wall -Werror
LDFLAGS =
BUILD = ./build
SRC=$(wildcard *.c)

RSApi_DIR = ../RSApi
RSApi_LIB = $(RSApi_DIR)/build/librsa.a

.PHONY: all rebuild rsa_api build_all clean

all: rsa_api build_all

rebuild: clean all

rsa_api:
	cd $(RSApi_DIR) && $(MAKE)

build_all: $(patsubst %.c, $(BUILD)/%.o, $(SRC))

$(BUILD)/%.o : %.c
	$(CC) $(CFLAGS) -o $(@) $(^) $(RSApi_LIB) $(LDFLAGS)

clean:
	rm -f $(SRC:%.c=$(BUILD)/%.o)
	cd $(RSApi_DIR) && $(MAKE) clean
