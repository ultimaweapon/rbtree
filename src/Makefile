.PHONY: all clean

OUTPUT = librbtree.a
CPPFLAGS = -O3 -Wall -I../include
SOURCES = rbtree.c

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	find . -iname '*.o' -delete

visualizer: CPPFLAGS += -DRBTREE_DEBUG
visualizer: visualizer.o $(SOURCES:.c=.o)

$(OUTPUT): $(SOURCES:.c=.o)
	$(AR) rcs $@ $+
