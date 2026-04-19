#include "solver.h"
#include "irs.h"
#include "delay.h"
#include <stdbool.h>
#include <stdint.h>

const uint16_t IR_wall_threshold = 2048; // CALLIBRATE: threshold IR reading for a wall

Action solver(Maze* m) {
    return floodFill(m);
}

void maze_init(Maze* m) {
    /* Set initial mouse state */
    m->state = SEARCHING_CENTER;
    m->target_x_min = 7;
    m->target_x_max = 8;
    m->target_y_min = 7;
    m->target_y_max = 8;
    m->mouse_x = 0;
    m->mouse_y = 0;
    m->mouse_dir = NORTH;

    /* Setup queue head & tail */
    m->queue_head = 0;
    m->queue_tail = 0;

    /* Set initial state of cells */
    for (uint8_t y = 0; y < GRID_SIZE; ++y) {
        for (uint8_t x = 0; x < GRID_SIZE; ++x) {
            m->grid[y][x].x = x;
            m->grid[y][x].y = y;

            // Set all cells except goals as unvisited
            if (x < m->target_x_min || x > m->target_x_max || y < m->target_y_min || y > m->target_y_max)
                m->grid[y][x].flags = 0x10;
            else
                m->grid[y][x].flags = 0x00;

            // Add perimeter walls
            if (x == 0) { m->grid[y][x].flags |= WALL_WEST; }
            if (y == 0) { m->grid[y][x].flags |= WALL_SOUTH; }
            if (x == GRID_SIZE - 1) { m->grid[y][x].flags |= WALL_EAST; }
            if (y == GRID_SIZE - 1) { m->grid[y][x].flags |= WALL_NORTH; }
        }
    }
    
    /* Set manhattan distances of goal (s)*/
    for (uint8_t i = m->target_y_min; i <= m->target_y_max; ++i) {
        for (uint8_t j = m->target_x_min; j <= m->target_x_max; ++j) {
            m->grid[i][j].distance = 0;
        }
    }

    /* Initial Calculation of Manhattan Distances*/
    maze_recalculate_distance(m);
}

/*
* Updates wall flags for a coordinate x and y based on an absolute direction
*/
void maze_update_wall(Maze* m, uint8_t x, uint8_t y, uint8_t direction) {
    switch (direction) {
        case NORTH:
            m->grid[y][x].flags |= WALL_NORTH;
            if (maze_in_Bounds(x, y + 1)) {
                m->grid[y+1][x].flags |= WALL_SOUTH;
            }
            break;
        case EAST:
            m->grid[y][x].flags |= WALL_EAST;
            if (maze_in_Bounds(x + 1, y)) {
                m->grid[y][x + 1].flags |= WALL_WEST;
            }
            break;
        case SOUTH:
            m->grid[y][x].flags |= WALL_SOUTH;
            if (maze_in_Bounds(x, y - 1)) {
                m->grid[y - 1][x].flags |= WALL_NORTH;
            }
            break;
        case WEST:
            m->grid[y][x].flags |= WALL_WEST;
            if (maze_in_Bounds(x - 1, y)) {
                m->grid[y][x - 1].flags |= WALL_EAST;
            }
            break;        
    }
}

void maze_enqueue(Maze* m, Cell* c) {
    m->queue[(m->queue_tail)++] = c;
}

Cell* maze_dequeue(Maze* m) {
    return m->queue[(m->queue_head)++];
}

void maze_recalculate_distance(Maze* m) {
    /* Setup queue head & tail */
    m->queue_head = 0;
    m->queue_tail = 0;

    /* Set all cells except goals as unvisited and reset distances */
    for (uint8_t y = 0; y < GRID_SIZE; ++y) {
        for (uint8_t x = 0; x < GRID_SIZE; ++x) {
            if (x < m->target_x_min || x > m->target_x_max || y < m->target_y_min || y > m->target_y_max) {
                m->grid[y][x].flags |= UNVISITED;
                m->grid[y][x].distance = 255;
            }
            else {
                m->grid[y][x].distance = 0;
                m->grid[y][x].flags &= ~UNVISITED;
                maze_enqueue(m, &m->grid[y][x]); // add goal cells to queue
            }
        }
    }

    /* Process all cells in queue*/
    while (m->queue_head != m->queue_tail) {
        maze_process_cell(m, maze_dequeue(m));
    }

}

void maze_process_cell(Maze* m, Cell* c) {
    if (!(c->flags & WALL_NORTH) && (m->grid[(c->y) + 1][c->x].flags & UNVISITED)) {
        m->grid[(c->y) + 1][c->x].flags &= ~UNVISITED;
        m->grid[(c->y) + 1][c->x].distance = c->distance + 1;
        maze_enqueue(m, &m->grid[(c->y) + 1][c->x]);
    }
    if (!(c->flags & WALL_EAST) && (m->grid[c->y][(c->x) + 1].flags & UNVISITED)) {
        m->grid[c->y][(c->x) + 1].flags &= ~UNVISITED;
        m->grid[c->y][(c->x) + 1].distance = c->distance + 1;
        maze_enqueue(m, &m->grid[c->y][(c->x) + 1]);
    }
    if (!(c->flags & WALL_SOUTH) && (m->grid[(c->y) - 1][c->x].flags & UNVISITED)) {
        m->grid[(c->y) - 1][c->x].flags &= ~UNVISITED;
        m->grid[(c->y) - 1][c->x].distance = c->distance + 1;
        maze_enqueue(m, &m->grid[(c->y) - 1][c->x]);
    }
    if (!(c->flags & WALL_WEST) && (m->grid[c->y][(c->x) - 1].flags & UNVISITED)) {
        m->grid[c->y][(c->x) - 1].flags &= ~UNVISITED;
        m->grid[c->y][(c->x) - 1].distance = c->distance + 1;
        maze_enqueue(m, &m->grid[c->y][(c->x) - 1]);
    }
}

/* Returns true is coordinate is inside of 16x16 maze */
bool maze_in_Bounds(uint8_t x, uint8_t y) {
    if (x >= GRID_SIZE || y >= GRID_SIZE) { return false; }
    return true;
}

/* 
*   Takes an direction relative to the mouse's heading direction 
*   and returns the absolute direction
*/
uint8_t get_Abs_Dir(uint8_t mouse_dir, uint8_t rel_dir) {
    return (mouse_dir + rel_dir) % 4;
}

Action floodFill(Maze* m) {
    /* if currently in goal */
    if (m->mouse_x >= m->target_x_min && m->mouse_x <= m->target_x_max && m->mouse_y >= m->target_y_min && m->mouse_y <= m->target_y_max) {
        if (m->state == SEARCHING_CENTER) {
            m->target_x_min = 0;
            m->target_x_max = 0;
            m->target_y_min = 0;
            m->target_y_max = 0;

            // maybe small delay (?)
            m->state = RETURNING_START;
            maze_recalculate_distance(m);
        }
        else if (m->state == RETURNING_START) {
            /* Ensures mouse is facing north */
            if (m->mouse_dir == SOUTH) {
                m->mouse_dir = NORTH;
                return UTURN;
            }
            else if (m->mouse_dir == EAST) {
                m->mouse_dir = NORTH;
                return LEFT;
            }
            else if (m->mouse_dir == WEST) {
                m->mouse_dir = NORTH;
                return RIGHT;
            }
            m->state = WAITING;
            return IDLE;
        }
        else if (m->state == WAITING) {
            if ((readIR(IR_FRONT_LEFT) + readIR(IR_FRONT_RIGHT) / 2) > IR_wall_threshold) { // block IR sensor with hand to trigger SPEEDRUN ?
                m->target_x_min = 7; 
                m->target_x_max = 8;
                m->target_y_min = 7; 
                m->target_y_max = 8;
                m->state = SPEEDRUN;
            }
            maze_recalculate_distance(m);
            return IDLE;
        }
        else {
            return IDLE;
        }
    }

    delayMicroseconds(50000);

    /* Detect walls */
    uint8_t old_flags = m->grid[m->mouse_y][m->mouse_x].flags;
    if (readIR(IR_LEFT) > IR_wall_threshold) {
        maze_update_wall(m, m->mouse_x, m->mouse_y, get_Abs_Dir(m->mouse_dir, REL_LEFT));
    }
    if (readIR(IR_RIGHT) > IR_wall_threshold) {
        maze_update_wall(m, m->mouse_x, m->mouse_y, get_Abs_Dir(m->mouse_dir, REL_RIGHT));
    }
    if((readIR(IR_FRONT_LEFT) + readIR(IR_FRONT_RIGHT) / 2) > IR_wall_threshold) {
        maze_update_wall(m, m->mouse_x, m->mouse_y, get_Abs_Dir(m->mouse_dir, REL_FRONT));
    }

    /* Recalculate distances if a new wall is discovered */
    if (old_flags != m->grid[m->mouse_y][m->mouse_x].flags) {
        maze_recalculate_distance(m);
    }

    /* Calculates the best absolute direction based on the surrounding coordinates' distances */
    Direction bestDirection = m->mouse_dir;
    uint8_t lowestDistance = 255;
    if (!(m->grid[m->mouse_y][m->mouse_x].flags & WALL_NORTH) && m->grid[m->mouse_y + 1][m->mouse_x].distance < lowestDistance) {
        lowestDistance = m->grid[m->mouse_y + 1][m->mouse_x].distance;
        bestDirection = NORTH;
    }
    if (!(m->grid[m->mouse_y][m->mouse_x].flags & WALL_EAST) && m->grid[m->mouse_y][m->mouse_x + 1].distance < lowestDistance) {
        lowestDistance = m->grid[m->mouse_y][m->mouse_x + 1].distance;
        bestDirection = EAST;
    }
    if (!(m->grid[m->mouse_y][m->mouse_x].flags & WALL_SOUTH) && m->grid[m->mouse_y - 1][m->mouse_x].distance < lowestDistance) {
        lowestDistance = m->grid[m->mouse_y - 1][m->mouse_x].distance;
        bestDirection = SOUTH;
    }
    if (!(m->grid[m->mouse_y][m->mouse_x].flags & WALL_WEST) && m->grid[m->mouse_y][m->mouse_x - 1].distance < lowestDistance) {
        lowestDistance = m->grid[m->mouse_y][m->mouse_x - 1].distance;
        bestDirection = WEST;
    }

    /* Transforms best absolute direction into the best direction relative to mouse heading direction */
    RelativeDirection bestRelDirection;
    bestRelDirection = (bestDirection - (m->mouse_dir) + 4) % 4;

    switch (bestRelDirection) {
        case REL_FRONT:
            if (m->mouse_dir == NORTH) { m->mouse_y++; }
            else if (m->mouse_dir == EAST) { m->mouse_x++; }
            else if (m->mouse_dir == SOUTH) { m->mouse_y--; }
            else if (m->mouse_dir == WEST) { m->mouse_x--; }
            return FORWARD;
        case REL_RIGHT:
            m->mouse_dir = (m->mouse_dir + 1) % 4;
            return RIGHT;
        case REL_BACK:
            m->mouse_dir = (m->mouse_dir + 2) % 4;
            return UTURN;
        case REL_LEFT:
            m->mouse_dir = (m->mouse_dir + 3) % 4;
            return LEFT;
    }

    return IDLE;
}
