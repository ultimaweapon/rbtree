.PHONY: all clean

OUTPUT = librbtree.a
CPPFLAGS = -Wall -I../include
SOURCES = rbtree.c

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	find . -iname '*.o' -delete

$(OUTPUT): $(SOURCES:.c=.o)
	$(AR) rcs $@ $+
