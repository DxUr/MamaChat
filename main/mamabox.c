#if 0
#include "mamabox.h"
#include <wchar.h>
#include "net/net.h"

// HERE AND JUST HERE THE MAMABOX LOGIC
// THE WINDOW FORMAT
#define W 80
#define H 22

/*
    ╔════════════╗
    ║            ║
    ╠════════════╣
    ║            ║
    ╚════════════╝
*/


extern mode_e current_mode;
extern char alias[8];
extern int id;

char playground[W-2][H-5];

const int8_t ps[2] = {0, 1};
const int8_t ng[2] = {0, -1};

void set_cursor(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
}

int getch() {
    int ch;
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return ch;
}


int pos[MAX_CLIENTS_COUNT][2];

void main_loop() {
    memset(playground, ' ', sizeof(playground));

    for (unsigned int i = 0; i < MAX_CLIENTS_COUNT; i++)
    {
        if (!pos[i][0] ||
            pos[i][0] > sizeof(playground) / sizeof(playground[0]) ||
            !pos[i][1] ||
            pos[i][1] > sizeof(playground[0]) / sizeof(playground[0][0])) continue;
        playground[pos[i][0]-1][pos[i][1]-1] = '$';
    }
    int key = getch();
    if (65<=key&&key<=68){
        byte_t buffer[MAX_BUFFER_SIZE];
        MSG_HEADER* msg = buffer;
        msg->op = 0;
        msg->id = id;
        strncpy(msg->alias, alias, sizeof(msg->alias) - 1);
        msg->posx = pos[id][0] + ps[key == 66] + ps[key == 68];
        msg->posy = pos[id][1] + ng[key == 65] + ng[key == 67];
        Net_Handler.send_data(buffer);
        
    }
}


void draw_frame() {
    //CLS // clear terminal
    set_cursor(0, 0);
    printf("╔");
    for (unsigned int i = 0; i < W - 2; i++) printf("═");
    printf("╗\n");
    for (unsigned int i = 1; i < H; i++)
    {
        set_cursor(0, i);
        if (i==4) {
            printf("╠");
            for (unsigned int _i = 0; _i < W - 2; _i++) printf("═");
            printf("╣\n");
            continue;
        }
        printf("║");
        set_cursor(W-1, i);
        printf("║\n");
    }
    printf("╚");
    for (unsigned int i = 0; i < W - 2; i++) printf("═");
    printf("╝\n");

    set_cursor(1, 1);
    printf(current_mode==CLIENT?"Client:%s\n":"Server:%s\n", alias);
    printf("\033[C");
    
}

void draw_playground() {
    set_cursor(1, 5);
    for (int y = 0; y < sizeof(playground[0]) / sizeof(playground[0][0]); y++) {
        for (int x = 0; x < sizeof(playground) / sizeof(playground[0]); x++) {
            putchar(playground[x][y]);
        }
        printf("\n\033[C");
    }
}



void connected() {

    while (1)
    {
        CLS
        draw_playground();
        draw_frame();
        main_loop();
        fflush(stdout);
        usleep(1000000/TPS);
    }

}

void disconnected() {

}

void client_connected(buffer_t p_buff) {

}

void client_disconnected(buffer_t p_buff) {

}

void recv_data(buffer_t p_buff) {
    MSG_HEADER* msg = p_buff;
    printf("reciev data form some where op:%d\n", msg->op);
    getchar();
}
#endif