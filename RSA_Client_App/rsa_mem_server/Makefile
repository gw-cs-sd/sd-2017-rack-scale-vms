rsa_mem_server = src
clean_rsa_mem_server = $(addprefix clean_,$(rsa_mem_server))

.PHONY: $(rsa_mem_server) $(clean_rsa_mem_server)

all: $(rsa_mem_server)
clean: $(clean_rsa_mem_server)

$(rsa_mem_server):
	cd $@ && $(MAKE)

$(clean_rsa_mem_server):
	cd $(patsubst clean_%,%,$@) && $(MAKE) clean
