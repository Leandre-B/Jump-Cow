#include <cstddef>
#include <cstdlib>
#include <sched.h>
#include <stdio.h>
#include <stdbool.h>
#include <thread>
#include <time.h> // sleep
#include <unistd.h>
#include <termios.h> // raw terminal mod
#include <pthread.h>
#include <chrono>
#include <ctime>
#include <queue>


struct termios oldt;
void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void enable_raw_mode()
{
    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}


const double GRAVITY = 0.1;
int GROUND = 11;

struct Cow{
    char sprite[2][2][4] = {
        {{' ', '_', '_', '0'},
        {'|', '\\', '|', '\\'}},
        {{' ', '_', '_', '0'},
        {'/', '|', '/', '|'}}
    };
    double x = 10, y = GROUND;
    int height = 2, width = 4;
    double speed = 0.4;
};

struct Tree{
    char sprite[3][3] = {
        {' ', '_', ' ',},
        {'|', ' ', '|',},
        {' ', '|', ' ',}
    };
    double x = 50, y = GROUND -1;
    int height = 3, width = 3;
    int delay = 60;
    int wait = 0;
    double speed = 0.7;
    bool active = false;
};

struct Bird{
    char sprite[1][3] = {
        {'<', '-', '-',},
    };
    double x = 50, y = GROUND -5;
    int height = 1, width = 3;
    int delay = 60;
    int wait = 0;
    double speed = 0.7;
    bool active = false;
};

struct Cloud{
    char sprite[1][3] = {
        {'<', '-', '-',},
    };
    double x = 50, y = GROUND -5;
    int height = 1, width = 3;
    int delay = 60;
    int wait = 0;
    double speed = 0.7;
    bool active = false;
};

void printDino(const char dino[2][4], int x, int y){
    for(int i=0; i<2; ++i){
        for(int j=0; j<4; ++j){
            printf("\x1b[%d;%dH", y+i, x+j);
            printf("%c", dino[i][j]);
        }
        printf("\n");
    }
}

void printAbre(const char dino[3][3], int x, int y){
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            printf("\x1b[%d;%dH", y+i, x+j);
            printf("%c", dino[i][j]);
        }
        printf("\n");
    }
}

void printBird(const char bird[3][3], int x, int y){
    for(int i=0; i<3; ++i){
        printf("\x1b[%d;%dH", y, x+i);
        printf("%c", bird[0][i]);
    }
}

void printGui(const char gui[][60]){
    for(int i=0; i<17; ++i){
        for(int j=0; j<60; ++j){
            printf("\x1b[%d;%dH", i+2, j+2);
            printf("%c", gui[i][j]);
        }
        printf("\n");
    }
}

std::queue<char> keys;
void* listenKeyboard(void* arg){
    char c;
    while(true){
        keys.push(getchar());
    }
}

bool collide(const Cow & cow, const Tree & tree){
    for(int i=cow.x; i<cow.x+cow.width; ++i){
        for(int j=cow.y; j<cow.y+cow.height; ++j){
            if( i > tree.x && i < tree.x + tree.width &&
                j > tree.y && j < tree.y + tree.height)
                return true;
        }
    }
    return false;
}

bool collide(const Cow & cow, const Bird & bird){
    for(int i=cow.x; i<cow.x+cow.width; ++i){
        for(int j=cow.y; j<cow.y+cow.height; ++j){
            if( i > bird.x && i < bird.x + bird.width &&
                j >= bird.y && j < bird.y + bird.height)
                return true;
        }
    }
    return false;
}

int main(){

    enable_raw_mode();

    pthread_t keyboardListener;
    pthread_create(&keyboardListener, NULL, listenKeyboard, NULL);


    char gui[][60] = {
    "      Score :                                              ",
    "  _______________________________________________________  ",
    " /                                                       \\ ",
    "|                                                         |",
    "|                                                         |",
    "|                                                         |",
    "|                                                         |",
    "|                                                         |",
    "|                                                         |",
    "|.........................................................|",
    "|.........................................................|",
    "|.........................................................|",
    " \\_______________________________________________________/ ",
    "              ____    _____                                ",
    "             /       |     |   |   _   |                   ",
    "            |        |     |    | | | |                    ",
    "             \\___    |_____|     |   |                     ",


    };

    Tree tree;
    Cow cow;
    Bird bird;

    bool loose = false;
    int score = 0;
    srand((time(nullptr)));
    while(true){
        auto t_start = std::chrono::high_resolution_clock::now();
       
        //clear
        printf("\x1b[2J");
        printf("\x1b[H");

        if(!keys.empty()){
            char key = keys.front();
            printf("%d", key);
            keys.pop();
            if(key == 122 and cow.y <= GROUND and GROUND >10)
                --GROUND;
            else if(key == 115 and GROUND <12)
                ++GROUND;
            else if(key == 32 and cow.y == GROUND)
                cow.speed = -cow.speed;
            else if(key == 114 && loose){
                loose = false;
                cow.x = 10; cow.y = GROUND;
                if(cow.speed < 0)
                    cow.speed = -cow.speed;
                tree.x = 50; tree.y = GROUND -1;
                tree.delay = 60; tree.wait = 0; tree.speed = 0.5;
                tree.active = false;

                bird.x = 50; bird.y = GROUND -4;
                bird.delay = 60; bird.wait = 0; bird.speed = 0.5;
                bird.active = false;

                score = 0;
                
                //clear q
                std::queue<char> empty;
                keys.swap(empty);
            }
        }

        if(!loose){
            cow.y += cow.speed;
            if(cow.y < 5)
                cow.speed = -cow.speed;
            if(cow.y > GROUND)
                cow.y = GROUND;


            if(!tree.active && !bird.active && (tree.wait<=0 or bird.wait<=0)){
                if(rand()%2 == 0)
                    tree.active = true;
                else
                    bird.active = true;

                tree.speed += 0.01;
                bird.speed += 0.01;
            }
            if(tree.x <= 3){
                tree.wait = tree.delay;
                tree.x = 53;
                tree.y = rand()%3 + 9;
                tree.delay -= 3;
                tree.active = false;
            }
            if(tree.active){
                tree.x -= tree.speed;
            }

            if(!tree.active)
                --tree.wait;

            if(bird.x <= 3){
                bird.wait = bird.delay;
                bird.x      = 53;
                bird.y = rand()%5 + 7;
                bird.delay -= 3;
                bird.active = false;
            }
            if(bird.active){
                bird.x -= bird.speed;
            }

            if(!bird.active)
                --bird.wait;
        
            if(collide(cow, tree) or collide(cow, bird))
                loose = true;
        }

        printGui(gui);
        printf("\x1b[2;17H%d", score);
        if((score/10)%2==0 or cow.y!=GROUND)
            printDino(cow.sprite[0], cow.x, cow.y);
        else
            printDino(cow.sprite[1], cow.x, cow.y);

        if(tree.active)
            printAbre(tree.sprite, tree.x, tree.y);
        if(bird.active)
            printBird(bird.sprite, bird.x, bird.y);
        if(loose){
            printf("\x1b[8;14H You loose ! Press r for retry");
        }
        fflush(stdout);
        score+=!loose;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    printf("Perdu !");
    while(true){

    };

    pthread_join(keyboardListener, NULL);
    restore_terminal();
    return 0;
}