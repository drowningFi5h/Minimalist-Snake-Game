# TECHNICAL REPORT: NCURSES-BASED SNAKE GAME

**Course:** Operating Systems (CS303)  
**Name:** Tausif Ansari  
**ID:** 202351148  
**Reference:** Nir Latchman - YouTube Tutorial  
**Date:** November 16, 2025

---

## 1. OVERVIEW

I built a Snake game that runs directly in our Linux terminal using the ncurses library. The entire game works without any graphical interface—just text characters on a black terminal. Throughout this project, I learned a lot about how terminal systems work, how to handle keyboard input in real-time, and how to make things appear on screen without a GUI framework.

---

## 2. WHAT I LEARNED

### 2.1 Why This Matters
When you use modern apps, you're usually looking at a graphical interface with buttons and windows. But programs don't need all that complexity to work well. For interactive programs that need to respond quickly to user input, a terminal-based approach can be simpler and sometimes even more efficient. This project taught me how to build that kind of interactive system from scratch.

### 2.2 What is ncurses?
ncurses stands for "new curses." It's a C library that lets programmers talk to the terminal in a nice, organized way. Without it, making the snake appear at specific locations, handling arrow keys, or clearing the screen would be messy. The library handles all that complexity so I can focus on the game logic.

---

## 3. HOW THE CODE IS ORGANIZED

### 3.1 The Libraries I Used

```c
#include <curses.h>        // Terminal control library
#include <stdlib.h>        // Random numbers
#include <time.h>          // Getting current time
#include <unistd.h>        // Sleeping/delays
#include <string.h>        // String operations

#define MAX_SNAKE_LENGTH 1000  // Longest the snake can grow
```

I needed four key libraries. The `curses.h` is the star of the show—that's what lets me control the terminal. The `time.h` helps me seed the random number generator so the food doesn't appear in the same spot every game. The `unistd.h` lets me pause the game for brief moments to control how fast it runs.

### 3.2 Creating a Point Structure

```c
typedef struct {
    int x;
    int y;
} Point;
```

Instead of tracking snake positions with separate x and y arrays, I created a simple Point structure that bundles both coordinates together. This makes the code cleaner and easier to understand.

---

## 4. SETTING UP THE TERMINAL

### 4.1 The Initialization Code

```c
initscr();                          // Take control of terminal
WINDOW *window = stdscr;            // Get the main screen
keypad(window, TRUE);               // Recognize special keys
nodelay(window, TRUE);              // Don't wait for keyboard
curs_set(0);                        // Hide the cursor
```

When the program starts, I need to set up the terminal properly. Think of `initscr()` as me raising my hand and telling the operating system, "I'm taking over the terminal now, let me handle everything."

#### What `keypad(window, TRUE)` Does
Here's a problem that would happen without this line: when you press the left arrow key on a regular terminal, the system sends a sequence of characters like `\033[D` (an escape sequence). My program would receive multiple separate values instead of one clear "arrow left" signal. By setting `keypad(TRUE)`, ncurses says, "I'll handle these escape sequences for you and just tell the game that the left arrow was pressed." This makes the game feel much more responsive.

#### What `nodelay(window, TRUE)` Does
By default, if I ask the terminal "Did the user press a key?", it would freeze and wait until they actually press something. That would pause the entire game. With `nodelay(TRUE)`, the system immediately tells me whether a key was pressed or not, and if not, my game keeps running. This is crucial for smooth gameplay.

#### `curs_set(0)` - Hide the Cursor
The cursor is that blinking line you see in the terminal. For a game, I don't want the player seeing it, so I hide it with `curs_set(0)`.

### 4.2 Getting Screen Dimensions

```c
int maxX, maxY;
getmaxyx(window, maxY, maxX);
```

Before the game starts, I need to know how big the terminal window is. This function tells me the width and height so I can keep the snake from moving off the screen.

---

## 5. STARTING THE GAME STATE

```c
Point snake[MAX_SNAKE_LENGTH];      // Store all snake segments
int snakeLength = 3;                 // Start with 3 pieces
snake[0].x = 10; snake[0].y = 10;   // Head at (10, 10)
snake[1].x = 9;  snake[1].y = 10;   // Body piece 1
snake[2].x = 8;  snake[2].y = 10;   // Body piece 2

int foodX = rand() % 40;            // Food somewhere random
int foodY = rand() % 20;            
int dirX = 1, dirY = 0;             // Moving right at start
```

I set up the snake as a simple array of points. The first element (index 0) is always the head—the part that the player controls and that grows first when eating. The body follows behind.

The direction variables work like this: if `dirX = 1` and `dirY = 0`, the snake is moving right. If `dirX = 0` and `dirY = -1`, it's moving up. These are basic direction vectors that I use to update position each frame.

---

## 6. THE MAIN GAME LOOP

The heart of any game is its loop—the code that runs over and over, handling input, updating the world, and drawing everything. My loop does four things each cycle:

```
Get Player Input → Move Snake → Check Collisions → Draw on Screen
```

### 6.1 Handling Player Input

```c
pressed = wgetch(window);

if (pressed == KEY_LEFT && dirX != 1) {
    dirX = -1; dirY = 0;
}
else if (pressed == KEY_RIGHT && dirX != -1) {
    dirX = 1; dirY = 0;
}
else if (pressed == KEY_UP && dirY != 1) {
    dirX = 0; dirY = -1;
}
else if (pressed == KEY_DOWN && dirY != -1) {
    dirX = 0; dirY = 1;
}
else if (pressed == 'q' || pressed == 'Q') {
    break;
}
```

I check what key the player pressed. Here's something important: I don't just accept any direction change. The conditions like `dirX != 1` prevent the snake from doing an immediate 180-degree turn. If the snake is moving right, I ignore a left arrow press because that would make the snake crash into itself instantly. That wouldn't be fair to the player.

Also notice that `wgetch()` returns immediately because of the `nodelay(TRUE)` setting from earlier. If no key was pressed, it returns an error value and my game just continues normally.

### 6.2 Moving the Snake

```c
int newX = snake[0].x + dirX;
int newY = snake[0].y + dirY;

// Boundary wrapping
if (newX < 0) newX = maxX - 1;
if (newX >= maxX) newX = 0;
if (newY < 0) newY = maxY - 1;
if (newY >= maxY) newY = 0;
```

First, I calculate where the head should go by adding the direction to its current position. Then, I handle what happens at the edges. If the snake goes off the left side of the screen, it appears on the right side. This wrapping makes the game feel continuous—like the screen is actually a tunnel that loops around.

### 6.3 Checking for Self-Collision

```c
for (int i = 0; i < snakeLength; i++) {
    if (newX == snake[i].x && newY == snake[i].y) {
        // Game Over
        endwin();
        printf("Game Over! Final Length: %d\n", snakeLength);
        return 0;
    }
}
```

Before I move the snake's head to the new position, I check if it would crash into any part of its own body. If the new head position matches any body segment, the game ends. This loop has to check every segment, so it runs in O(n) time where n is the snake's length. As the snake grows, this gets slower, but for a game on a terminal, it's plenty fast.

### 6.4 The Snake's Movement Physics

```c
for (int i = snakeLength - 1; i > 0; i--) {
    snake[i].x = snake[i - 1].x;
    snake[i].y = snake[i - 1].y;
}
snake[0].x = newX;
snake[0].y = newY;
```

This is clever: I move from the tail toward the head, making each body segment take the position of the one in front of it. Then the head moves to the new position. The result looks like the entire snake shifted forward by one space, but really I just shuffled positions around. Here's an example:

**Before:** Head(10,10) → Body1(9,10) → Body2(8,10) → Tail(7,10)  
**After:** Head(11,10) → Body1(10,10) → Body2(9,10) → Tail(8,10)

It's like everyone shifted one step forward in line.

---

## 7. DRAWING ON SCREEN

```c
erase();                                    // Clear everything
for (int i = 0; i < snakeLength; i++) {
    mvaddstr(snake[i].y, snake[i].x, "*");  // Draw each segment
}
mvaddstr(foodY, foodX, "+");                // Draw food
mvprintw(0, 0, "Length: %d", snakeLength); // Draw score
refresh();                                  // Make it visible
```

Each frame, I clear the screen and redraw everything. Without the `erase()` call, the snake would leave a trail of asterisks everywhere it moved, which would be confusing.

The `mvaddstr()` function name actually breaks down nicely: "mv" means move the cursor, "add" means add text to the drawing buffer, "str" means it takes a string. So I'm moving to a position and drawing a character there. For the score at the top, I use `mvprintw()` which is like printf but for the screen.

Finally, `refresh()` is critical. All the drawing I do happens in a buffer in memory. `refresh()` actually sends all those changes to the terminal so the player can see them. Without this call, the screen wouldn't update at all.

---

## 8. EATING FOOD AND GROWING

```c
if (snake[0].x == foodX && snake[0].y == foodY) {
    if (snakeLength < MAX_SNAKE_LENGTH) {
        snakeLength++;
    }
    foodX = rand() % maxX;
    foodY = rand() % maxY;
}
```

When the snake's head lands on the same spot as the food, two things happen. First, I increase the snake's length by one. The clever part is that the snake doesn't immediately grow a new segment. Instead, because the body-shifting code now has one less segment to move (the new length is +1), the tail stays in place instead of moving forward. The next frame, the snake looks bigger. That's all there is to it.

Then I pick a new random spot for the food. There's a small chance the food could spawn right on the snake, but that's rare enough not to worry about for a simple game.

---

## 9. CONTROLLING GAME SPEED

```c
usleep(100000);  // 100 milliseconds
```

If the game loop ran as fast as the computer could manage, the snake would move so quickly you couldn't control it. By putting the program to sleep for 100,000 microseconds (which is 0.1 seconds), I'm creating a consistent frame rate of about 10 frames per second.

This number is easy to tweak: use 50,000 for a faster game, or 200,000 for something slower. It's one of the first things you'd change if you wanted to adjust difficulty.

---

## 10. SHUTTING DOWN PROPERLY

```c
endwin();  // Restore terminal to normal
return 0;  // Exit the program
```

This is critical. When the game ends, I call `endwin()` which reverses everything that `initscr()` did. It turns off raw mode, shows the cursor again, and gives control of the terminal back to the shell. If I forgot this line, the terminal would be left in an unusable state—the user's typed commands wouldn't show up, and the terminal would be hard to fix without closing the whole window.

That's why I call `endwin()` before exiting on game over, and also here at the end of main. You have to be careful to clean up after yourself.

---

## 11. WHAT THIS TAUGHT ME ABOUT OPERATING SYSTEMS

### 11.1 Terminal I/O and System Calls
Under the hood, what looks like magic is really the program calling operating system functions. When I call `wgetch()`, it's actually calling the `read()` system call to get keyboard input from the OS. When I call `mvaddstr()` and `refresh()`, that eventually calls `write()` to send text to the terminal. And `usleep()` calls a system function that tells the OS to pause this process.

### 11.2 Process and Terminal Control
The whole game runs as a single process. The terminal is a resource that the OS manages. When I call `initscr()`, I'm telling the OS, "I want to control this terminal exclusively." The OS lets me manipulate the terminal state (raw mode, cursor, etc.) and then waits for me to give it back with `endwin()`.

### 11.3 Memory and Data Structure Choices
I chose to use a fixed-size array instead of dynamic memory allocation. This was a trade-off: simpler code that's easier to debug, but with a maximum snake length. In a real application, I'd probably use dynamic memory (malloc/free) to allow unlimited growth, but for a learning project, this approach is cleaner.

### 11.4 Input Buffering and Terminal Modes
Terminal input normally works in "canonical" mode—the OS buffers up a whole line of text until you press Enter. By using ncurses and setting raw mode, I'm telling the OS to give me each key press individually the moment it happens. The `nodelay(TRUE)` setting means "don't wait for input"—just check if a key was pressed and tell me immediately whether it was or not.

### 11.5 Real-Time Responsiveness
Games need to be responsive. The frame loop runs on my schedule (controlled by `usleep()`), not on waiting for user input. This is different from a traditional interactive program that might be blocked waiting for a command. The game stays in control the whole time.

---

## 12. WAYS TO MAKE THIS BETTER

### 12.1 Features I Could Add
- Keep track of high scores and save them to a file
- Add different difficulty levels that get faster as you score more points
- Add obstacles or walls on the screen
- Have multiple food items worth different points
- Use colors for different parts of the snake
- Add sound effects when you eat or crash
- Create a pause feature

### 12.2 Bug Fixes
The current version works well, but could be improved:
- Food sometimes spawns on top of the snake. I could check the snake array before placing food
- The random food placement uses modulo, which creates a slight bias. A better approach would sample more evenly
- If you want a "game over on hitting walls" mode instead of wrapping, that would take a few line changes

### 12.3 Making It Faster
- Instead of clearing the whole screen every frame, I could just update the specific cells that changed
- Use double-buffering to eliminate flicker if running on slow terminals
- Cache computations instead of recalculating things

---

## 13. HOW TO BUILD AND RUN IT

### 13.1 Compiling the Code
Open a terminal in the project directory and run:

```bash
gcc -o snake snake.c -lncurses
```

The `-o snake` part names the output file. The `-lncurses` part tells the compiler to link in the ncurses library. If you want to see warnings while compiling, add `-Wall -Wextra`.

### 13.2 Running the Game
```bash
./snake
```

Use arrow keys to move, and press 'q' to quit.

### 13.3 Using the Makefile
If you have the Makefile I created, life is easier:

```bash
make           # Compiles the game
make run       # Compiles and runs it
make clean     # Removes the compiled game
```

---

## 14. TESTING THE GAME

I tested the game by playing it under various conditions:

| What I Tested | What Should Happen |
|---|---|
| Moving normally | Snake follows arrow key input smoothly |
| Pressing arrow keys rapidly | Game handles it without crashing |
| Turning 180 degrees | Invalid turns are ignored |
| Hitting screen edge | Snake wraps to opposite side |
| Eating food | Snake grows, score increases, new food appears |
| Hitting own body | Game ends and shows final length |
| Pressing 'q' | Game closes cleanly |

### 14.2 Edge Cases That Could Break Things
- Very small terminal window (nothing bad happens, just cramped gameplay)
- Mashing keys really fast (handled fine by the input buffer)
- Snake filling the entire screen (capped by MAX_SNAKE_LENGTH)
- No food ever spawns on top of snake (statistically rare)

---

## 15. WHAT I'LL TAKE AWAY FROM THIS

This project showed me that you don't need a complex graphics framework to make an interactive program. The terminal has been around for decades, and the tools to control it (like ncurses) are still powerful and elegant. Building this game taught me about:

- How the operating system handles terminal I/O at a fundamental level
- The importance of non-blocking input in interactive applications
- How to organize a real-time game loop
- Memory layout and array-based data structure management
- Clean shutdown procedures (you have to put things back the way you found them)

More than anything, this project made me appreciate how much thought goes into seemingly simple interactions. When you press a key, there's a whole chain of events: the OS intercepts it, sends it to my program, my program decides what to do, calculates new positions, checks for collisions, draws on the screen, and then waits for the next frame. All in about 100 milliseconds.

The snake game is one of the classic programming projects because it hits that sweet spot: simple enough to code in an afternoon, complex enough to learn from.

---

## 16. FURTHER READING

If you want to dive deeper into this stuff:
- **ncurses Docs:** The official docs at https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/ have tons of examples
- **Linux Man Pages:** Type `man ncurses` or `man wgetch` to get documentation right in your terminal
- **Game Loop Design:** Look up articles on real-time game loops—this is used in professional games too
- **Terminal Emulation:** If you're curious what's actually happening under the hood, research ANSI escape codes

---

**Written by:** Tausif Ansari (202351148)  
**Course:** CS303 - Operating Systems  
**Date:** November 16, 2025  
**Reference:** Nir Latchman YouTube Tutorial
