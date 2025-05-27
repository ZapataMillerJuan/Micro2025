#include "irDebounce.h"

void IR_Init(IRDebounce *ir) {
	ir->state = IR_UP;
	ir->last_sample = 1;
}

void IR_Update(IRDebounce *ir, uint8_t sample) {
	switch (ir->state) {
		case IR_RISING:
			if (sample == 1 && ir->last_sample == 1){
				ir->state = IR_UP;
				ir->stateConfirmed = 1;
			}else{
				ir->state = IR_DOWN;
			}
			break;

		case IR_UP:
			if (sample == 0){
				ir->state = IR_FALLING;
			}else{
				ir->state = IR_UP;
			}
			break;

		case IR_FALLING:
			if (sample == 0 && ir->last_sample == 0){
				ir->state = IR_DOWN;
				ir->stateConfirmed = 0;
				
			}else{
				ir->state = IR_UP;
			}
			break;

		case IR_DOWN:
			if (sample == 1){
				ir->state = IR_RISING;
				}else{
					ir->state = IR_DOWN;
				}
			break;
		default:
				ir->state = IR_UP;
			break;
	}

	ir->last_sample = sample;
}

uint8_t IR_GetState( IRDebounce *ir) {
	return ir->stateConfirmed;
}