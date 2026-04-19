#include <stdio.h>
#include "solver.h"
#include "API.h"


// You do not need to edit this file.
// This program just runs your solver and passes the choices
// to the simulator.
int main(int argc, char* argv[]) {
    debug_log("Running...");
    Maze m;
    maze_init(&m);
    while (1) {
        Action nextMove = solver(&m);
        switch(nextMove){
            case FORWARD:
                API_moveForward();
                break;
            case LEFT:
                API_turnLeft();
                break;
            case RIGHT:
                API_turnRight();
                break;
            case IDLE:
                break;
            case UTURN:
                API_turnRight();
                API_turnRight();
                break;
        }
    }
}