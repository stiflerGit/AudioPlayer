GLIBS=`allegro-config --libs`

.PHONY:
all:	View
	gcc -g -o GUI View.o View_Config.o control_panel.o fspect_panel.o position_panel.o bar.o button.o  text.o controller.o $(GLIBS)

View: 	fspect_panel control_panel position_panel
	gcc -o View.o View.c -c $(GLIBS)
	gcc -o View_Config.o View_Config.c -c $(GLIBS)

fspect_panel: bar button
	gcc -o fspect_panel.o ./frequency_spectrum_panel/fspect_panel.c -c $(GLIBS)
	
control_panel: button controller
	gcc -o control_panel.o ./control_panel/control_panel.c -c $(GLIBS)
	
position_panel: bar text
	gcc -o position_panel.o ./position_panel/position_panel.c -c $(GLIBS)
	 
bar:
	gcc -o bar.o ./graphic_framework/bar.c -c $(GLIBS)
	
button:
	gcc -o button.o ./graphic_framework/button.c -c $(GLIBS)
	
text: 
	gcc -o text.o ./graphic_framework/text.c -c $(GLIBS)

controller:
	gcc -o controller.o ../Controller.c -c

clean:
	rm -f *.o GUI
