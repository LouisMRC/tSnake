#include <ncurses.h>
#include <deque>
#include <array>
#include <stdlib.h>
#include <time.h>
#include <string>

#define GAME_SIZE 20



void gameLoop(WINDOW *gameWin);

int nextX(int posX, int direction);
int nextY(int posY, int direction);

int dirReverse(int direction);

int getCellSize(int gameWinSize, int gameSize);

bool include(std::deque<std::array<int, 2>> deck, std::array<int, 2> element);

std::array<int, 2> applePos(std::deque<std::array<int, 2>> &snake);

void redraw(WINDOW *gameWin, WINDOW *scoreWin, int score, int cellSize, std::array<int, 2> apple, std::deque<std::array<int, 2>> &snake);


int main()
{
    initscr();
    noecho();
    curs_set(0);
    timeout(10);
    keypad(stdscr, 1);
    WINDOW *gameWin = newwin(0, 0, 0, 0);
    gameLoop(gameWin);
    delwin(gameWin);
    endwin();
    return 0;
}

void gameLoop(WINDOW *gameWin)
{
    srand (time(NULL));
    int scrSizeX, scrSizeY, gameWinSize;
    getmaxyx(stdscr, scrSizeY, scrSizeX);
    int cellSize = getCellSize((scrSizeY < scrSizeX/2 ? scrSizeY : scrSizeX/2), GAME_SIZE);
    gameWinSize = GAME_SIZE * cellSize;

    wresize(gameWin, gameWinSize+2, gameWinSize*2+2);
    mvwin(gameWin, scrSizeY/2-gameWinSize/2, scrSizeX/2-gameWinSize*2/2);


    std::deque<std::array<int, 2>> snake = {{GAME_SIZE/2, GAME_SIZE/2}, {GAME_SIZE/2-1, GAME_SIZE/2}};
    std::array<int, 2> apple = applePos(snake);
    int direction = 0;
    int newDirection = 0;
    int delay = 150, timeCounter = 0;
    int score = 0;
    WINDOW *scoreWin = newwin(1, 3, 0, scrSizeX-4);
    double speed = 1.0;
    int gameState = 0;//0: game start(press enter to start) 1: playing 2: pause(press 'p') 3: game over(press space to restart)
    int key = 0;
    redraw(gameWin, scoreWin, score, cellSize, apple, snake);
    do
    {
        //check if the screen has been resized then update screensize, recalculate cellsize and redraw:
        int newSizeX, newSizeY;
        getmaxyx(stdscr, newSizeY, newSizeX);
        if(!(scrSizeX == newSizeX && scrSizeY == newSizeY))
        {
            scrSizeX = newSizeX;
            scrSizeY = newSizeY;

            cellSize = getCellSize((scrSizeY < scrSizeX/2 ? scrSizeY : scrSizeX/2), GAME_SIZE);
            gameWinSize = GAME_SIZE * cellSize;
            wresize(gameWin, gameWinSize+2, gameWinSize*2+2);
            mvwin(gameWin, scrSizeY/2-gameWinSize/2, scrSizeX/2-gameWinSize*2/2);

            mvwin(scoreWin, 0, scrSizeX-4);

            redraw(gameWin, scoreWin, score, cellSize, apple, snake);
        }

        //update:
        if(timeCounter>delay/speed && gameState == 1)
        {
            if(newDirection != dirReverse(direction))direction = newDirection;
            timeCounter = 0;
            std::array<int, 2> nextPos = {nextX(snake[0][0], direction), nextY(snake[0][1], direction)};
            if(nextPos == apple)
            {
		beep();
                score++;
                speed+=0.05;
                apple = applePos(snake);
            }
            else if(nextPos[0] >= GAME_SIZE || nextPos[0] < 0 || nextPos[1] >= GAME_SIZE || nextPos[1] < 0 || include(snake, nextPos))gameState = 3;
            else snake.pop_back();
            
            snake.push_front(nextPos);
            redraw(gameWin, scoreWin, score, cellSize, apple, snake);
        }

        //handle input:
        key = (int)getch();
        if(key == 10 && gameState == 0)gameState = 1;
        else if(key == KEY_RIGHT)newDirection = 0;
        else if(key == KEY_UP)newDirection = 90;
        else if(key == KEY_LEFT)newDirection = 180;
        else if(key == KEY_DOWN)newDirection = 270;
        else if(key == 'p')
        {
            if(gameState == 1)gameState = 2;
            else if(gameState == 2)gameState = 1;
        }
        if(gameState == 1)timeCounter+=10;
    } 
    while (key != 'q' && key != ' ');
    delwin(scoreWin);
    if(key == ' ')gameLoop(gameWin);
}

int nextX(int posX, int direction)
{
    if(direction == 0)return posX+1;
    else if(direction == 180)return posX-1;
    return posX;
}

int nextY(int posY, int direction)
{
    if(direction == 90)return posY-1;
    else if(direction == 270)return posY+1;
    return posY;
}

int dirReverse(int direction)
{
    return (direction+180)%360;
}

int getCellSize(int gameWinSize, int gameSize)
{
    return (gameWinSize-2)/gameSize;
}

//clear the stdscreen and redraw(snake, apple, score)
void redraw(WINDOW *gameWin, WINDOW *scoreWin, int score, int cellSize, std::array<int, 2> apple, std::deque<std::array<int, 2>> &snake)
{
    clear();
    wclear(gameWin);
    wclear(scoreWin);
    wattron(gameWin, A_REVERSE);

    //draw the snake
    for(std::array<int, 2> pos: snake)
        for(int i = 0; i < cellSize*2; i++)
            for(int j = 0; j < cellSize; j++)
                mvwprintw(gameWin, 1+pos[1]*cellSize+j, 1+pos[0]*cellSize*2+i, " ");

    //draw the apple
        for(int i = 0; i < cellSize*2; i++)
            for(int j = 0; j < cellSize; j++)
                mvwprintw(gameWin, 1+apple[1]*cellSize+j, 1+apple[0]*cellSize*2+i, " ");

    //display score
    wprintw(scoreWin, "%d", score);

    wattroff(gameWin, A_REVERSE);
    wborder(gameWin, '|', '|', '-', '-', '+', '+', '+', '+');
    refresh();
    wrefresh(scoreWin);
    wrefresh(gameWin);
}


bool include(std::deque<std::array<int, 2>> deck, std::array<int, 2> element)
{
    for(std::array<int, 2> dequeElement: deck)if(dequeElement == element)return 1;
    return 0;
}
std::array<int, 2> applePos(std::deque<std::array<int, 2>> &snake)
{
    std::array<int, 2> apple;
    do apple = {rand()%(GAME_SIZE-1), rand()%(GAME_SIZE-1)}; while(include(snake, apple));
    return apple;
}
