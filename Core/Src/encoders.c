/*
 * encoders.c
 */

#include "main.h"
#include "encoders.h"

/*
 * Returns right encoder count
 */
int16_t getRightEncoderCounts() {
	return -(int16_t) TIM2->CNT;
}

/*
 * Returns left encoder count
 */
int16_t getLeftEncoderCounts() {
	return -(int16_t) TIM1->CNT;
}


void resetEncoders() {
	TIM1->CNT = (int16_t) 0;
	TIM2->CNT = (int16_t) 0;
}
