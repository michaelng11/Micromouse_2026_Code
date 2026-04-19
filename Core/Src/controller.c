/*
 * controller.c
 */

#include "main.h"
#include "controller.h"
#include "pid.h"
#include "delay.h"

/*
 * move(1) will move rat 1 cell forward
 */
void move(int8_t n) {
	int32_t oneCell = 600;
	setPIDGoalA(0);
	setPIDGoalD(oneCell * n);
	while (!PIDdone()) {
		updatePID();
		delayMicroseconds(1000);
	}
	resetPID();
}

/*
 * turn(1) turns your rat 90 degrees in your positive rotation direction and turn(-1) turns the other way
 */
void turn(int8_t n) {
	int32_t oneTurn = 500;

	setPIDGoalD(0);
	setPIDGoalA(oneTurn * n);
	while (!PIDdone()) {
		updatePID();
		delayMicroseconds(1000);
	}
	resetPID();
}
