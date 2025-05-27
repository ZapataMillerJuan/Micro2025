#ifndef COMMUNICATION_ATMEGA328_H
#define COMMUNICATION_ATMEGA328_H

#include <stdlib.h>

#define RINGBUFFLENGTH		256
#define POSID               2
#define POSDATA             5

/**
 * @brief Enumeraci�n de la MEF para decodificar el protocolo
 * 
 */
typedef enum{
    START,
    HEADER_1,
    HEADER_2,
    HEADER_3,
    NBYTES,
    TOKEN,
    PAYLOAD
}_eProtocolo;

/**
 * @brief Enumeraci�n de la lista de comandos
 * 
 */
typedef enum{
    ACK=            0x0D,
    GETALIVE=       0xF0,
    FIRMWARE=       0xF1,
	IR=				0xA0,
	DISTANCE=		0xA3,
	DEBUGER=		0xDE
}_eID;

typedef struct{
	uint8_t timeOut;					//!< TiemOut para reiniciar la m�quina si se interrumpe la comunicaci�n
	uint8_t indexStart;					//!< Indice para saber en que parte del buffer circular arranca el ID
	uint8_t cheksumRx;					//!< Cheksumm RX
	uint8_t indexWriteRx;				//!< Indice de escritura del buffer circular de recepci�n
	uint8_t indexReadRx;				//!< Indice de lectura del buffer circular de recepci�n
	uint8_t indexWriteTx;				//!< Indice de escritura del buffer circular de transmisi�n
	uint8_t indexReadTx;				//!< Indice de lectura del buffer circular de transmisi�n
	uint8_t bufferRx[RINGBUFFLENGTH];   //!< Buffer circular de recepci�n
	uint8_t bufferTx[RINGBUFFLENGTH];   //!< Buffer circular de transmisi�n
	
}_sDato;

/************************************************************************/
/* Ejecutar una sola vez, inicializa la configuarci�n del USART			*/
/************************************************************************/
void initialize_USART(_sDato* datosCom, unsigned int ubrr,  void (*decodeAux)(_sDato *datosCom));

/************************************************************************/
/* Ejecutar constantemente												*/
/************************************************************************/
void task_communication(_sDato* datosCom);

void decodeProtocol(_sDato *datosCom);

void (*decode)(_sDato *datosCom);

#endif