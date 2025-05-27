#ifndef SERVO_H_
#define	SERVO_H_

#include <stdint.h>

#define MAX_SERVOS 3

/**
*
*	setear valores de -90 a 90 seleccionando el servo
*/
void servo_setAngle(uint8_t servo, int8_t angle);

/**
* 
*
*/
uint8_t servo_add(volatile uint8_t* PORT, uint8_t PIN);

/**
* configurar timer de 8bits con 128us de ovf. 156 ciclos son ~20ms (50Hz para el servo)
* esta función va dentro de la interrupción de ovf del timer
*/
void servo_interrupt();

#endif