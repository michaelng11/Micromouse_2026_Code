/*
 * pid.c
 */

#include "main.h"
#include "motors.h"
#include "encoders.h"

/* Left and Right Encoder Counts */
int16_t left_counts = 0;
int16_t right_counts = 0;

/* Goal Variables */
static int16_t goalAngle = 0;
static int16_t goalDistance = 0;
static uint8_t cyclesAtGoal = 0;

/* PD Constants (Angular) */
static const float kPw = 0.01f;
static const float kDw = 1.0f;

/* PD Constants (Distance) */
static const float kPx = 0.01f;
static const float kDx = 1.0f;

/* PD Variables (Angular) */
static int16_t angleError = 0;
static int16_t prevAngleError = 0;

/* PD Variables (Distance) */
static float distanceError = 0.0f;
static float prevDistanceError = 0.0f;

/* Misc */
static const float MAXIMUM_ANGLE_CORRECTION = 0.5f;
static const float acceleration = 0.3f;

static float prevMotorSpeed = 0.0f;


/*
 * This function resets all the variables to their default state. Additionally, it resets motors and encoder counts.
 */
void resetPID() {
	cyclesAtGoal = 0;
	prevMotorSpeed = 0;

	goalAngle = 0;
	angleError = 0;
	prevAngleError = 0;

	goalDistance = 0;
	distanceError = 0;
	prevDistanceError = 0;

	resetMotors();
	resetEncoders();

}

/*
 * Updates PID
 */
void updatePID() {
	// Updates encoder counts
	left_counts = getLeftEncoderCounts();
	right_counts = getRightEncoderCounts();

	// Angle PD
	angleError = goalAngle - (left_counts - right_counts);
	int16_t angleDerivative = angleError - prevAngleError;
	float angleCorrection = (kPw * angleError) + (kDw * angleDerivative);
	prevAngleError = angleError;

	// Distance PD
	distanceError = goalDistance - ((left_counts + right_counts) / 2);
	float distanceDerivative = distanceError - prevDistanceError;
	float distanceCorrection = (kPx * distanceError) + (kDx * distanceDerivative);
	prevDistanceError = distanceError;

	// Caps the Angle Correction
	if (angleCorrection >= MAXIMUM_ANGLE_CORRECTION) {
		angleCorrection = MAXIMUM_ANGLE_CORRECTION;
	}
	else if (angleCorrection <= -MAXIMUM_ANGLE_CORRECTION) {
		angleCorrection = -MAXIMUM_ANGLE_CORRECTION;
	}

	// Caps acceleration / deceleration
	if (distanceCorrection - prevMotorSpeed > acceleration) {
		distanceCorrection = prevMotorSpeed + acceleration;
	}
	else if (distanceCorrection - prevMotorSpeed <-acceleration) {
		distanceCorrection = prevMotorSpeed - acceleration;
	}
	prevMotorSpeed = distanceCorrection;

	// Apply motor corrections
	setMotorLPWM(distanceCorrection + angleCorrection);
	setMotorRPWM(distanceCorrection - angleCorrection);

	// Check if at goal
	uint8_t distAtGoal = (distanceError < 15 && distanceError > -15);
	uint8_t angleAtGoal = (angleError < 10 && angleError > -10);

	if (distAtGoal && angleAtGoal) {
		cyclesAtGoal++;
		if (cyclesAtGoal > 100) {
			cyclesAtGoal = 100;
		}
	}
	else {
		cyclesAtGoal = 0;
	}

}

/* Sets the goal distance */
void setPIDGoalD(int16_t distance) {
	goalDistance = distance;
}

/* Sets the goal angle */
void setPIDGoalA(int16_t angle) {
	goalAngle = angle;
}

/* Returns 1 if a number of cycles have passed while sufficiently close to the goal as determined in updatePID() */
int8_t PIDdone(void) {
	if (cyclesAtGoal >= 100)
		return 1;
	else
		return 0;
}
