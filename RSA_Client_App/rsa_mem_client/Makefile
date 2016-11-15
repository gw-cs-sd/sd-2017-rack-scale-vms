rsa_mem_client = src
clean_rsa_mem_client = $(addprefix clean_,$(rsa_mem_client))

.PHONY: $(rsa_mem_client) $(clean_rsa_mem_client)

all: $(rsa_mem_client)
clean: $(clean_rsa_mem_client)

$(rsa_mem_client):
	cd $@ && $(MAKE)

$(clean_rsa_mem_client):
	cd $(patsubst clean_%,%,$@) && $(MAKE) clean
