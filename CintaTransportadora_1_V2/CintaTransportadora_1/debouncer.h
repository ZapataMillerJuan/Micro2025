#ifndef DEBOUNCER_H_
#define DEBOUNCER_H_

#include <stdio.h>

#define MAX_INPUTS_DEBOUNCED		4

#define FALLING_EDGE				0
#define RISING_EDGE					1

#define SETED						1
#define CLEARED						0

typedef enum{
	DOWN,
	UP,
	FALLING,
	RISING
}e_Estados;

typedef struct{
	uint8_t value;
	e_Estados state;
	uint8_t val;
	volatile uint8_t* pin;
	uint8_t  pos;
	void (*stateChanged)(e_Estados estado);
}s_Input;

s_Input debouncerBuff[MAX_INPUTS_DEBOUNCED];
uint8_t	inputIndex;

void initialize_debounce();

void task_debouncer();

uint8_t add_to_debounce(volatile uint8_t* PIN, uint8_t  POS, void (*STATECHANGED)(e_Estados estado));

void inputState(s_Input *stateInput);

uint8_t getState(uint8_t input);


#endif //DEBOUNCER_H_