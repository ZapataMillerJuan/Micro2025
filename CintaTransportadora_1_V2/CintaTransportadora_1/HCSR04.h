#ifndef HCSR04_H_
#define HCSR04_H_

#include <stdint.h>

typedef enum
{
	HCSR04_STATE_READY,
	HCSR04_STATE_NO_INIT,
	HCSR04_STATE_BUSY,
}_eHCSR04State;

/*!
@brief HCSR04_AddNew Funci�n de inicializaci�n

Esta Funci�n agrega un nuevo HCSR04

@param [in] WritePin: Puntero a una funci�n que permite escribir un pin.
@param [in] ticks: Indica la cantidad de instrucciones por micro segundo que puede ejecutar el MCU.

@retVal La funci�n devuelve un n�mero que identifica al m�dulo HCSR04
*/
unsigned int HCSR04_AddNew(void (*WritePin)(uint8_t value), uint32_t ticks);
				 
/************************************************************************/
/* Devuelve la ultima distancia medida                                  */
/************************************************************************/
uint16_t HCSR04_Read(unsigned int handleHCSR04);

/************************************************************************/
/* Establece trigger en ON							                    */
/************************************************************************/
_eHCSR04State HCSR04_Start(unsigned int handleHCSR04);

/************************************************************************/
/* Devuelve el estado actual del HCSR seleccionado                      */
/************************************************************************/
_eHCSR04State HCSR04_State(unsigned int handleHCSR04);

/************************************************************************/
/* Establece el trigger en OFF                                          */
/************************************************************************/
void HCSR04_TriggerReady(unsigned int handleHCSR04);

/************************************************************************/
/* Llamar cuando se deba esperar el rise del HCSR seleccionado          */
/************************************************************************/
void HCSR04_RiseEdgeTime(unsigned int handleHCSR04, uint16_t usTimeRise);

/************************************************************************/
/* Llamar cuando se deba esperar el rise del HCSR seleccionado          */
/************************************************************************/
void HCSR04_FallEdgeTime(unsigned int handleHCSR04, uint16_t usTimeFall);

/************************************************************************/
/* Ejecutar constantemente                                              */
/************************************************************************/
void task_HCSR();

/************************************************************************/
/* Seleccionar a que funci�n llamar cuando se tenga una distancia medida*/
/************************************************************************/
void HCSR04_AttachOnReadyMeasure(unsigned int handleHCSR04, void (*OnReadyMeasure)(uint16_t distance));

#endif /* HCSR04_H_ */