#ifndef IR_DEBOUNCE_H
#define IR_DEBOUNCE_H

#include <stdint.h>

typedef enum {
	IR_IDLE,
	IR_RISING,
	IR_UP,
	IR_FALLING,
	IR_DOWN
} IRState;

typedef struct {
	IRState state;
	uint8_t last_sample;
} IRDebounce;

// Inicializa la estructura
void IRDebounce_Init(IRDebounce *ir);

// Llama esta función cada 40ms pasando el valor leído del sensor (0 o 1)
void IRDebounce_Update(IRDebounce *ir, uint8_t sample);

// Devuelve el estado actual (IR_UP, IR_DOWN, etc.)
IRState IRDebounce_GetState(const IRDebounce *ir);

#endif