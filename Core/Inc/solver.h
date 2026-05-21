#ifndef SOLVER_H
#define SOLVER_H

#include <stdbool.h>
#include <stdint.h>

#define GRID_SIZE 16

/* Cell Flags */
#define WALL_NORTH 0x01 // 0000 0001
#define WALL_EAST 0x02 // 0000 0010
#define WALL_SOUTH 0x04 // 0000 0100
#define WALL_WEST 0x08 // 0000 1000
#define UNVISITED 0x10 // 0001 0000

/* Goal Cells */
#define TARGET_X_MIN 7
#define TARGET_X_MAX 8
#define TARGET_Y_MIN 7
#define TARGET_Y_MAX 8

/* Mouse State */
typedef enum MouseState {
    SEARCHING_CENTER, 
    RETURNING_START,
    WAITING,
    SPEEDRUN
} MouseState;
/* Absolute Direction */
typedef enum Direction {NORTH, EAST, SOUTH, WEST} Direction;
/* Relative Direction */
typedef enum RelativeDirection {REL_FRONT, REL_RIGHT, REL_BACK, REL_LEFT} RelativeDirection;
/* Action to take */
typedef enum Action {LEFT, FORWARD, RIGHT, IDLE, UTURN} Action;

typedef struct {
    uint8_t flags;
    uint8_t distance;
    uint8_t x;
    uint8_t y;
} Cell;

typedef struct {
    Cell grid[GRID_SIZE][GRID_SIZE];
    uint8_t mouse_x;
    uint8_t mouse_y;
    Direction mouse_dir;

    /* 
    * Queue 
    * uint8_t will overflow leading to a circular array
    */
    Cell* queue[256];
    uint8_t queue_head;
    uint8_t queue_tail;

    MouseState state;
    uint8_t target_x_min;
    uint8_t target_x_max;
    uint8_t target_y_min;
    uint8_t target_y_max;

} Maze;

void maze_init(Maze* m);
void maze_update_wall(Maze* m, uint8_t x, uint8_t y, uint8_t direction);
void maze_enqueue(Maze* m, Cell* c);
Cell* maze_dequeue(Maze* m); 
void maze_recalculate_distance(Maze* m);
void maze_process_cell(Maze* m, Cell* c);

bool maze_in_Bounds(uint8_t x, uint8_t y);
uint8_t get_Abs_Dir(uint8_t mouse_dir, uint8_t rel_dir);


Action solver(Maze* m);
Action floodFill(Maze* m);

#endif
