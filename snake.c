#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define MAX_SNAKE_LENGTH 1000

typedef struct {
    int x;
    int y;
} Point;

int main() {
    // Initialize ncurses
    initscr();
    WINDOW *window = stdscr;
    
    // Configure window options
    keypad(window, TRUE);      // Enable arrow keys
    nodelay(window, TRUE);      // Make wgetch() non-blocking
    curs_set(0);                // Hide cursor
    
    // Seed random number generator
    srand(time(0));
    
    // Game variables
    Point snake[MAX_SNAKE_LENGTH];
    int snakeLength = 3;
    snake[0].x = 10; snake[0].y = 10;
    snake[1].x = 9;  snake[1].y = 10;
    snake[2].x = 8;  snake[2].y = 10;
    
    int foodX = rand() % 40;
    int foodY = rand() % 20;
    int dirX = 1, dirY = 0;
    int pressed;
    int maxX, maxY;
    int grew = 0;
    
    // Get maximum screen dimensions
    getmaxyx(window, maxY, maxX);
    
    // Game loop
    while (1) {
        // Input handling
        pressed = wgetch(window);
        
        // Change direction based on input
        if (pressed == KEY_LEFT && dirX != 1) {
            dirX = -1;
            dirY = 0;
        }
        else if (pressed == KEY_RIGHT && dirX != -1) {
            dirX = 1;
            dirY = 0;
        }
        else if (pressed == KEY_UP && dirY != 1) {
            dirX = 0;
            dirY = -1;
        }
        else if (pressed == KEY_DOWN && dirY != -1) {
            dirX = 0;
            dirY = 1;
        }
        else if (pressed == 'q' || pressed == 'Q') {
            // Quit game
            break;
        }
        
        // Advance position - move snake head
        int newX = snake[0].x + dirX;
        int newY = snake[0].y + dirY;
        
        // Boundary wrapping
        if (newX < 0) newX = maxX - 1;
        if (newX >= maxX) newX = 0;
        if (newY < 0) newY = maxY - 1;
        if (newY >= maxY) newY = 0;
        
        // Check if snake ate itself
        for (int i = 0; i < snakeLength; i++) {
            if (newX == snake[i].x && newY == snake[i].y) {
                // Game Over
                endwin();
                printf("Game Over! Final Length: %d\n", snakeLength);
                return 0;
            }
        }
        
        // Shift snake body - add new head
        for (int i = snakeLength - 1; i > 0; i--) {
            snake[i].x = snake[i - 1].x;
            snake[i].y = snake[i - 1].y;
        }
        snake[0].x = newX;
        snake[0].y = newY;
        
        // Clear screen and draw
        erase();
        
        // Draw snake body
        for (int i = 0; i < snakeLength; i++) {
            mvaddstr(snake[i].y, snake[i].x, "*");
        }
        
        // Draw food
        mvaddstr(foodY, foodX, "+");
        
        // Draw score
        mvprintw(0, 0, "Length: %d", snakeLength);
        
        refresh();
        
        // Check food collision
        if (snake[0].x == foodX && snake[0].y == foodY) {
            // Grow snake
            if (snakeLength < MAX_SNAKE_LENGTH) {
                snakeLength++;
            }
            // Generate new food
            foodX = rand() % maxX;
            foodY = rand() % maxY;
        }
        
        // Control speed
        usleep(100000);  // 100ms delay
    }
    
    // Cleanup
    endwin();
    
    return 0;
}
