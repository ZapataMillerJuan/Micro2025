/*
 * motores.h
 *
 *  Created on: Jun 11, 2025
 *      Author: juanz
 */

#ifndef MOTORES_H_
#define MOTORES_H_

#include "stdint.h"

typedef enum{
	FRONT,
	BACK,
	FREE,
	BRAKE
}_eState;

typedef struct{
	uint16_t speed;
	_eState estado;
	uint16_t maxSpeed;
}_sEng;

void en_InitENG(_sEng *engines,uint16_t maxSpeed);
void en_HandlerENG(_sEng *engines,int32_t newspeed,uint8_t freno);


#endif /* MOTORES_H_ */
