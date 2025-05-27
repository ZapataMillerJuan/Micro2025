#include "servo.h"
#include "utilities.h"

#define ANGLE_TO_PWM_SLOPE			121     // Pendiente de conversi�n (78 unidades PWM por grado)
#define ANGLE_TO_PWM_OFFSET			21093    // Offset para 0�
#define ROUNDING_CORRECTION			5     // Correcci�n para redondeo (+5/10 = redondeo aritm�tico)


typedef struct sServo{
	volatile uint8_t* port;
	uint8_t pin;
	volatile uint8_t cycle;
	uint8_t dutyCycle;
}s_servo;

static s_servo buffServos[MAX_SERVOS];
static volatile uint8_t is20ms = 156;
static uint8_t servoCount = 0;
//static volatile uint8_t iterator = 0;
volatile int16_t pwm_value;

void servo_setAngle(uint8_t servo, int8_t angle) {
    if(servo >= servoCount || angle < -90 || angle > 90) {
        return;
    }
    pwm_value = (int16_t)angle * ANGLE_TO_PWM_SLOPE;
    pwm_value += ANGLE_TO_PWM_OFFSET;
	pwm_value /= 180;
    pwm_value += ROUNDING_CORRECTION;
    pwm_value /= 10;
	
    buffServos[servo].dutyCycle = (uint8_t)pwm_value;
}

uint8_t servo_add(volatile uint8_t* PORT, uint8_t PIN){
	if(servoCount >= MAX_SERVOS)
		return 0xFF;
	
	buffServos[servoCount].cycle = 12;
	buffServos[servoCount].dutyCycle = 12;
	buffServos[servoCount].port = PORT;
	buffServos[servoCount].pin = PIN;
	return servoCount++;
}

void servo_interrupt(){
	is20ms--;
	for(iterator = 0; iterator < MAX_SERVOS; iterator++){
		buffServos[iterator].cycle--;
		if(!buffServos[iterator].cycle)
			*buffServos[iterator].port &= ~(1 << buffServos[iterator].pin);
	}
	if(!is20ms){
		for(iterator = 0; iterator < MAX_SERVOS; iterator++){
			buffServos[iterator].cycle = buffServos[iterator].dutyCycle;
			*buffServos[iterator].port |= (1 << buffServos[iterator].pin);
		}
		is20ms = 156;
	}
}