.PHONY: all clean test

OUTDIR = lib

all: $(OUTDIR)
	$(MAKE) -C src
	cp src/librbtree.a $(OUTDIR)/

clean:
	$(MAKE) -C tests clean
	$(MAKE) -C src clean
	rm -f $(OUTDIR)/*

test: all
	$(MAKE) -C tests

$(OUTDIR):
	mkdir $@
