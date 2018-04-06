CFLAGS = -Wall -g
LDLIBS = -lfftw3f -lm
GLIBS = `allegro-config --libs`

GOBJS = bar.o button.o text.o

.PHONY:
all:	View	Model
	gcc -o audioplayer main.c view.o model.o $(GLIBS)

View:	Graphic_fw
	gcc -o view.o -c View/View.c $(GOBJS) $(GLIBS)

View_Config: 
	gcc -o view_config -c View/View_Config.c

Graphic_fw:
	gcc -o bar.o -c View/graphic_framework/bar.c  $(GLIBS)
	gcc -o text.o -c View/graphic_framework/text.c $(GLIBS)
	gcc -o button.o -c View/graphic_framework/button.c $(GLIBS) 

Model:
	gcc -o model.o -c Model/Player.c $(CLIBS) $(GLIBS)

Controller:

clean:
	rm -f *.o 
