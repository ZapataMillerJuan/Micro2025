/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "irDebounce.h"
#include "util.h"
/* END Includes --------------------------------------------------------------*/


/* typedef -------------------------------------------------------------------*/
flags		flag0;
 _sRx		dataRx;
 _sTx		dataTx;
_uWord		myWord;
/* END typedef ---------------------------------------------------------------*/


/* define --------------------------------------------------------------------*/
#define LED					PB7
#define SW0					PB4
#define Echo				PL0
#define Trigger				PB1
#define IS10MS				flag0.bits.bit0
#define IS40MS				flag0.bits.bit1
#define RAW_INPUT			flag0.bits.bit2
#define ECHOREADY			flag0.bits.bit3
#define TRUE				1
#define FALSE				0
#define RXBUFSIZE           256
#define TXBUFSIZE           256
/* END define ----------------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void initialize_ports();
void initialize_timer1();
void every10ms();
//void decodeData(_sDato *datosCom);
void initialize_USART(uint8_t ubrr);
//void task_communication(unsigned char datos);
void serialTask(_sRx *dataRx, _sTx *dataTx);
void decodeCommand(_sRx *dataRx, _sTx *dataTx);
void decodeHeader(_sRx *dataRx);
uint8_t putHeaderOnTx(_sTx  *dataTx, _eCmd ID, uint8_t frameLength);
uint8_t putByteOnTx(_sTx *dataTx, uint8_t byte);
//void writeToTx(char* cadena);
/* END Function prototypes ---------------------------------------------------*/


/* Global variables ----------------------------------------------------------*/
uint8_t count100ms = 10;
uint8_t count40ms = 4;
uint8_t dato;
uint8_t raw_input;
uint8_t count10ms = 10;
volatile uint8_t buffRx[RXBUFSIZE];

uint8_t buffTx[TXBUFSIZE];
volatile meas_state_t meas_state = MEAS_IDLE;
TimeHcsr timehc;
IRDebounce ir_sensor;
uint8_t currentState;
/* END Global variables ------------------------------------------------------*/


/* Constant in Flash ---------------------------------------------------------*/

/* END Constant in Flash -----------------------------------------------------*/


/* Function ISR --------------------------------------------------------------*/
//ISR(TIMER0_COMPA_vect)
//{
//}
/* END Function ISR ----------------------------------------------------------*/


/* Function prototypes user code ----------------------------------------------*/
/*void writeToTx(char* cadena){
	if(*cadena != 0x00){
		task_communication(*cadena);
		cadena++;
	}else{
		return;
	}
}
void task_communication(unsigned char datos){
	while (!(UCSR0A&(1<<UDRE0)));
		UDR0 = datos;
}*/
void serialTask(_sRx* dataRx, _sTx* dataTx){
	if(dataRx->isComannd){
		dataRx->isComannd=FALSE;
		decodeCommand(dataRx,dataTx);
	}
	if(dataRx->indexR!=dataRx->indexW){
		
		decodeHeader(dataRx);
	}
	if (dataTx->indexR!= dataTx->indexW) {
		if (UCSR0A & (1 << UDRE0)) { // Si el buffer de transmisión está vacío
			UDR0 = dataTx->buff[dataTx->indexR++]; // Enviar el dato
			dataTx->indexR &= dataTx->mask;
		}
	}
}

uint8_t putHeaderOnTx(_sTx  *dataTx, _eCmd ID, uint8_t frameLength){
	dataTx->chk = 0;
	dataTx->buff[dataTx->indexW++]='U';
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]='N';
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]='E';
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]='R';
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]=frameLength+1;
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]=':';
	dataTx->indexW &= dataTx->mask;
	dataTx->buff[dataTx->indexW++]=ID;
	dataTx->indexW &= dataTx->mask;
	dataTx->chk ^= (frameLength+1);
	dataTx->chk ^= ('U' ^'N' ^'E' ^'R' ^ID ^':') ;
	return  dataTx->chk;
}
uint8_t putByteOnTx(_sTx *dataTx, uint8_t byte)
{
	dataTx->buff[dataTx->indexW++]=byte;
	dataTx->indexW &= dataTx->mask;
	dataTx->chk ^= byte;
	return dataTx->chk;
}
void decodeCommand(_sRx *dataRx, _sTx *dataTx){
	switch(dataRx->buff[dataRx->indexData]){
		case ALIVE:
			putHeaderOnTx(dataTx, ALIVE, 2);
			putByteOnTx(dataTx, ACK );
			putByteOnTx(dataTx, dataTx->chk);
			//PORTB ^= (1<<LED);
		break;
		case FIRMWARE:
		break;
		case LEDSTATUS:
			myWord.ui8[0] = currentState;
			putHeaderOnTx(dataTx, LEDSTATUS, 2);
			putByteOnTx(dataTx, myWord.ui8[0] );
			putByteOnTx(dataTx, dataTx->chk);
		break;
		case BUTTONSTATUS:
		break;
		case ANALOGSENSORS:
		break;
		case SETBLACKCOLOR:
		break;
		case SETWHITECOLOR:
		break;
		case MOTORTEST:
		break;
		case SERVOANGLE:
		break;
		case CONFIGSERVO:
		break;
		case GETDISTANCE:
			myWord.ui16[0]	= timehc.distance;
			putHeaderOnTx(dataTx, GETDISTANCE, 3);
			putByteOnTx(dataTx, myWord.ui8[0]);
			putByteOnTx(dataTx, myWord.ui8[1]);
			putByteOnTx(dataTx, dataTx->chk);
		break;
		case GETSPEED:
		break;
		case STARTSTOP:
		break;
		case GETDATACAR:
		
		break;
		default:
		putHeaderOnTx(dataTx, (_eCmd)dataRx->buff[dataRx->indexData], 2);
		putByteOnTx(dataTx,UNKNOWN );
		putByteOnTx(dataTx, dataTx->chk);
		break;
		
	}
}
void decodeHeader(_sRx *dataRx){
	uint8_t auxIndex=dataRx->indexW;
	while(dataRx->indexR != auxIndex){
		switch(dataRx->header)
		{
			case HEADER_U:
			if(dataRx->buff[dataRx->indexR] == 'U'){
				dataRx->header = HEADER_N;
			}
			break;
			case HEADER_N:
			if(dataRx->buff[dataRx->indexR] == 'N'){
				dataRx->header = HEADER_E;
				}else{
				if(dataRx->buff[dataRx->indexR] != 'U'){
					dataRx->header = HEADER_U;
					dataRx->indexR--;
				}
			}
			break;
			case HEADER_E:
			if(dataRx->buff[dataRx->indexR] == 'E'){
				dataRx->header = HEADER_R;
				}else{
				dataRx->header = HEADER_U;
				dataRx->indexR--;
			}
			break;
			case HEADER_R:
			if(dataRx->buff[dataRx->indexR] == 'R'){
				dataRx->header = NBYTES;
				}else{
				dataRx->header = HEADER_U;
				dataRx->indexR--;
			}
			break;
			case NBYTES:
			dataRx->nBytes=dataRx->buff[dataRx->indexR];
			dataRx->header = TOKEN;
			break;
			case TOKEN:
			if(dataRx->buff[dataRx->indexR] == ':'){
				dataRx->header = PAYLOAD;
				dataRx->indexData = dataRx->indexR+1;
				dataRx->indexData &= dataRx->mask;
				dataRx->chk = 0;
				dataRx->chk ^= ('U' ^'N' ^'E' ^'R' ^dataRx->nBytes ^':') ;
				}else{
				dataRx->header = HEADER_U;
				dataRx->indexR--;
			}
			break;
			case PAYLOAD:
			dataRx->nBytes--;
			if(dataRx->nBytes>0){
				dataRx->chk ^= dataRx->buff[dataRx->indexR];
				}else{
				dataRx->header = HEADER_U;
				if(dataRx->buff[dataRx->indexR] == dataRx->chk)
				dataRx->isComannd = TRUE;
			}
			break;
			default:
			dataRx->header = HEADER_U;
			break;
		}
		dataRx->indexR++;
		dataRx->indexR &= dataRx->mask;
	}
}



void Measure(){
	
	switch(meas_state){
		
		case MEAS_IDLE:
		if(ECHOREADY){
			meas_state = MEAS_TRIGGER;
		}
		break;
		
		case MEAS_TRIGGER:
		//aca debo esperar los 10us y pasar a wait for echo
		if(ECHOREADY){
			
			PORTB |= (1<<Trigger);//lanzo el trigger
			OCR4A = TCNT4 + 19;//hago una interrupcion a 20 ciclos que son 10us
			ECHOREADY = FALSE;
			TIMSK4 |= (1<<OCIE4A);
			TCCR4B |= _BV(ICES4);
			
		}
		
		break;
		
		case MEAS_WAIT_FOR_ECHO:
		
		//apago el trigger y espero a que devuelva el echo en la interrupcion del input caputre
		if (PORTB & (1<<Trigger)){
			
			PORTB &= ~(1<<Trigger);
			TIMSK4 |= (1 << ICIE4);
			TIMSK4 &= ~(1<<OCIE4A);
			
		}
		break;
		
		case MEAS_DONE: //obtengo la distancia en cm
		if(timehc.startEcho <= timehc.stopEcho){
			timehc.distance = timehc.stopEcho - timehc.startEcho;
			}else{
			timehc.distance = timehc.stopEcho - timehc.startEcho + 0xffff;
		}
		
		//timehc.distance = timehc.distance/58;
		break;
		
	}
	return;
}
void initialize_ports(){
	DDRB = (0 << SW0) | (1 << Trigger) | (1 <<LED);
	DDRL &= ~(1<<PL2);
	DDRL &= ~(1<<Echo);
	PORTL = (1 << PL2);
	PORTL = (1 << Echo);
}

void initialize_timer4(){ //CONFIGURAR PARA QUE CUENTE CADA 500ns
	TCCR4A = 0;
	OCR4B = 19999;
	TIFR4 = TIFR4;
	TIMSK4 = (1 << OCIE4B);
	TCCR4B = (1 << ICNC4) | (1 << CS41) | (1 << ICES4);
	TCNT4 = 0;
}

void initialize_USART(uint8_t ubrr){
	UBRR0H = 0;
	UBRR0L = ubrr;
	UCSR0A = 0xFE; //inicializo todas las banderas excepto el multiprocesor
	UCSR0B =   0x98; // (1<<RXCIE0) | (1<<RXEN0)|(1<<TXEN0); //Activo las banderas de interrupcion de recepcion y la habilitacion del rx y tx 
	UCSR0C =  0x06; // (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00); //se setea como asincrono, paridad desactivada, 1 stop bit, 8 data bits 
	
}
void every10ms(){
	if (!count100ms){								// If 100 ms have passed
		ECHOREADY=1;
		meas_state=MEAS_IDLE;
		count100ms = 10;
		
	}
		count100ms--;
	if(!count40ms){
		//IR_Update(&ir_sensor, raw_input);
		//currentState = IR_GetState(&ir_sensor);
		count40ms = 4;
	}
		count40ms--;
		IS10MS = FALSE;
}
/* END Function prototypes user code ------------------------------------------*/

int main()
{
	/* Local variables -----------------------------------------------------------*/
	dataRx.buff = (uint8_t *)buffRx;
	dataRx.indexR = 0;
	dataRx.indexW = 0;
	dataRx.header = HEADER_U;
	dataRx.mask = RXBUFSIZE - 1;

	dataTx.buff = buffTx;
	dataTx.indexR = 0;
	dataTx.indexW = 0;
	dataTx.mask = TXBUFSIZE -1;
	ECHOREADY=TRUE;
	timehc.distance=100;
	/* END Local variables -------------------------------------------------------*/



	
	/* User code Init ------------------------------------------------------------*/
	cli();
	//IR_Init(&ir_sensor);
	initialize_ports();
	initialize_timer4();
	initialize_USART(16); // 16 PARA atmega2560 para 115200, doble speed operation (U2Xn = 1)
	sei();
	/* END User code Init --------------------------------------------------------*/

	//writeToTx("PRENDIDO DESDE MICRO");
	while (1)
	{
		/* User Code loop ------------------------------------------------------------*/
		serialTask(&dataRx,&dataTx);
		Measure();
		if(IS10MS){
			every10ms();
			raw_input = (PINL & (1 << PL2));
			
			if (raw_input) {
				// El pin está en nivel ALTO (HIGH)
					currentState = 0x01;
					PORTB = (1<<LED);
					currentState = 0x00;
				} else {
				// El pin está en nivel BAJO (LOW)
				PORTB = (0<<LED);
		}
		/* END User Code loop --------------------------------------------------------*/
	}
	return 0;
}
ISR(USART0_RX_vect){
	dataRx.buff[dataRx.indexW++] = UDR0;
	dataRx.indexW &= dataRx.mask;
}
ISR(TIMER4_COMPB_vect){
	OCR4B += 20000;
	IS10MS = TRUE;							// Flag that indicates that 10 ms have passed
}
ISR(TIMER4_COMPA_vect){
	meas_state = MEAS_WAIT_FOR_ECHO;
}

ISR(TIMER4_CAPT_vect){
	
	//ices1 es el input capture edge , me decta el falco desendente en caso de ser 0
	if(TCCR4B & _BV(ICES4)){
		
		timehc.startEcho = ICR4;
		TCCR4B &= ~_BV(ICES4);
		//rising
		}else{
		//falling
		timehc.stopEcho = ICR4;
		meas_state = MEAS_DONE;
		
	}
}