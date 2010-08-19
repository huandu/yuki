SAMPLE_MAKEFILE = $(filter-out ./Makefile, $(wildcard */Makefile))
SAMPLE_DIR = $(patsubst %/Makefile, %/, $(SAMPLE_MAKEFILE))

.PHONY: all clean

all :
	@for i in "$(SAMPLE_DIR)"; do cd $$i && make && cd ..; done

clean :
	@for i in "$(SAMPLE_DIR)"; do cd $$i && make clean && cd ..; done
