#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <allegro.h>

#include "controller.h"
#include "player/player.h"
#include "view/view.h"

#define NULL ((void *)0)

void init()
{
    allegro_init();
    install_mouse();
    install_sound(DIGI_AUTODETECT, 0, 0);
    enable_hardware_cursor();
}

int main(int argc, char **argv)
{
    pthread_t *player_thread,
        *view_thread,
        *controller_thread;

    if (argc != 2)
    {
        printf("usage ./player <song_file_path>\n");
        exit(EXIT_FAILURE);
    }
    init();

    player_init(argv[1]);
    view_init();
    controller_init();
    player_thread = player_start(NULL);
    view_thread = view_start(NULL);
    controller_thread = controller_start(NULL);

    pthread_join(*controller_thread, NULL);
    printf("controller join\n");
    pthread_join(*player_thread, NULL);
    printf("player join\n");
    pthread_join(*view_thread, NULL);
    printf("view join\n");

    allegro_exit();
}
