.PHONY: all clean

OUTDIR = lib

all: $(OUTDIR)
	$(MAKE) -C src
	cp src/librbtree.a $(OUTDIR)/

clean:
	$(MAKE) -C src clean
	rm -f $(OUTDIR)/*

$(OUTDIR):
	mkdir $@
