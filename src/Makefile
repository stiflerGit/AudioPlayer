IDIR =./include
CC=gcc
CFLAGS=-Wall -g -I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm -lfftw3f
GLIBS = `allegro-config --libs`

# dependencies
_DEPS = controller.h graphic.h player.h ptask.h view_config.h view.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# object files
_OBJ = controller.o graphic.o player.o ptask.o view_config.o view.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# rules for object files
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(INCDIR)/*~ 