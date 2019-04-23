src =	$(wildcard src/*.c) \
		$(wildcard src/player/*.c) \
		$(wildcard src/view/*.c)

obj = $(src:.c=.o)

CFLAGS = -Wall -g
CPPFLAGS = -I./include
LDLIBS = -lfftw3f -lm 
LDFLAGS = -pthread -rt
GLIBS = `allegro-config --libs`

player: $(obj)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(GLIBS)

clean:
	rm -f *.o 
