#include <allegro.h>
#include <string.h>
#include <stdio.h>

#define handle_error(s) \
	 	do { perror(s), exit(EXIT_FAILURE);} while(0)

BITMAP *bmp, *bmp_p;
PALETTE pal;

int main(int argc, char **argv){
	int x,y,c;
	int pink;
	float hue, sat, val;
	char name[128];
	char pink_name[128];
	
	if (allegro_init() < 0)
		handle_error("allegro_init");

	set_color_depth(32);
	
	if(argc < 2)
		return -1;
	
	memset(name, 0, sizeof(name));
	memset(pink_name, 0, sizeof(name));
	strcpy(name, argv[1]);
	strcpy(pink_name, name);
	strtok(pink_name, "\.tga");
	strcat(pink_name, "pink.bmp");
	printf("%s\n", name);
	printf("%s\n", pink_name);

	bmp = load_bitmap(name, NULL);
	if(bmp == NULL)
		handle_error("load_bitmap");
	
	printf("w: %d, h: %d\n", bmp->w, bmp->h);
	
	bmp_p = create_bitmap(bmp->w, bmp->h);
	if(bmp_p == NULL)
		handle_error("create_bitmap");

	pink = makecol(255, 0, 255);
	for(x = 0; x < bmp->w; x++) {
		for(y=0; y < bmp->h; y++){
			c = getpixel(bmp, x, y);
			rgb_to_hsv(getr(c), getg(c), getb(c),
				   &hue, &sat, &val);

			val = val*255;
			if (val >= 240) c = pink;
			putpixel(bmp_p, x, y, c);
		}
	}
	
	get_palette(pal);
	if (save_bitmap(pink_name, bmp_p, pal) != 0)
		handle_error("save_bitmap");
	destroy_bitmap(bmp_p);
	allegro_exit();
}

