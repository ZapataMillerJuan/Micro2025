#include "irDebounce.h"

void IRDebounce_Init(IRDebounce *ir) {
	ir->state = IR_IDLE;
	ir->last_sample = 1;
}

void IRDebounce_Update(IRDebounce *ir, uint8_t sample) {
	switch (ir->state) {
		case IR_IDLE:
		if (sample == 0)
		ir->state = IR_FALLING;
		else
		ir->state = IR_RISING;
		break;

		case IR_RISING:
		if (sample == 1 && ir->last_sample == 1)
		ir->state = IR_UP;
		break;

		case IR_UP:
		if (sample == 0)
		ir->state = IR_FALLING;
		break;

		case IR_FALLING:
		if (sample == 0 && ir->last_sample == 0)
		ir->state = IR_DOWN;
		break;

		case IR_DOWN:
		if (sample == 1)
		ir->state = IR_RISING;
		break;
	}

	ir->last_sample = sample;
}

IRState IRDebounce_GetState(const IRDebounce *ir) {
	return ir->state;
}