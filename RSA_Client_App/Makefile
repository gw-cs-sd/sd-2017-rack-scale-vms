rsa_mem = rsa_mem_server rsa_mem_client

clean_rsa_mem = $(addprefix clean_,$(rsa_mem))

.PHONY: $(rsa_mem) $(clean_rsa_mem)

all: $(rsa_mem)
clean: $(clean_rsa_mem)

$(rsa_mem):
	cd $@ && $(MAKE)

$(clean_rsa_mem):
	cd $(patsubst clean_%,%,$@) && $(MAKE) clean
