rsa_mem = src
clean_rsa_mem = $(addprefix clean_,$rsa_mem)

.PHONY: $(rsa_mem) $(clean_rsa_mem)

all: $(rsa_mem)
clean: $(clean_rsa_mem)

$(rsa_mem):
	cd $@ && $(MAKE)

$(clean_rsa_mem):
	cd $(patsubst clean_%,%,$@) && (MAKE) clean
