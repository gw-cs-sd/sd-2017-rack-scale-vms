CC = gcc
CFLAGS = -g -Wall -Werror

RSApi_DIR = RSApi
RSApi_LIB = $(RSApi_DIR)/build/librsa.a

.PHONY: all
all: rsa_api rsa_mem_server rsa_test_client

rebuild: clean all

rsa_api:
	cd $(RSApi_DIR) && $(MAKE)

rsa_mem_server: rsa_api
	$(CC) $(CFLAGS) -o $@ $@.c $(RSApi_LIB)

rsa_test_client: rsa_api
	$(CC) $(CFLAGS) -o $@ $@.c $(RSApi_LIB)

.PHONY: clean
clean:
	rm -f rsa_mem_server rsa_test_client
	cd $(RSApi_DIR) && $(MAKE) clean

