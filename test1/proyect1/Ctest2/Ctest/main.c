/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include "util.h"
/* END Includes --------------------------------------------------------------*/


/* typedef -------------------------------------------------------------------*/
flags		flag0;
 _sRx		dataRx;
 _sTx		dataTx;

/* END typedef ---------------------------------------------------------------*/


/* define --------------------------------------------------------------------*/
#define LED					PB7
#define SW0					PB4
#define ECHO				PB0
#define TRIGGER				PB1
#define IS10MS				flag0.bits.bit0
#define TRUE				1
#define FALSE				0
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
void uart_transmit(uint8_t data);
//void writeToTx(char* cadena);
/* END Function prototypes ---------------------------------------------------*/


/* Global variables ----------------------------------------------------------*/
uint8_t count100ms = 100;
uint8_t dato;
uint8_t count10ms = 10;
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
			PORTB ^= (1<<LED);
		break;
		case FIRMWARE:
		break;
		case LEDSTATUS:
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
void initialize_ports(){
	DDRB = (0 << SW0) | (1 << TRIGGER) | (1 <<LED);
	DDRB &= ~(1<<ECHO);
	PORTB = (1 << SW0) | (1 << ECHO) | (1<<LED);
}

void initialize_timer4(){ //CONFIGURAR PARA QUE CUENTE CADA 500ns
	TCCR4A = 0;
	OCR4A = 19999;
	TIFR4 = TIFR4;
	TIMSK4 = (1 << OCIE4A);
	TCCR4B = (1 << ICNC4) | (1 << CS41);
}

void initialize_USART(uint8_t ubrr){
	UBRR0H = (ubrr >> 8);
	UBRR0L = ubrr;
	UCSR0A = 0xFE; //inicializo todas las banderas excepto el multiprocesor
	UCSR0B =   0x98; // (1<<RXCIE0) | (1<<RXEN0)|(1<<TXEN0); //Activo las banderas de interrupcion de recepcion y la habilitacion del rx y tx 
	UCSR0C =  0x06; // (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00); //se setea como asincrono, paridad desactivada, 1 stop bit, 8 data bits 
	
}
void every10ms(){
	if (!count100ms){								// If 100 ms have passed
		//on_reset_hcsr();
		//task_communication();
		count100ms = 10;
	}
	count100ms--;
	IS10MS = FALSE;
}
/* END Function prototypes user code ------------------------------------------*/

int main()
{
	/* Local variables -----------------------------------------------------------*/

	/* END Local variables -------------------------------------------------------*/



	
	/* User code Init ------------------------------------------------------------*/
	cli();
	initialize_ports();
	//initialize_timer1();
	
	initialize_USART(16); // 16 PARA atmega2560 para 115200, doble speed operation (U2Xn = 1)
	sei();
	/* END User code Init --------------------------------------------------------*/

	//writeToTx("PRENDIDO DESDE MICRO");
	while (1)
	{
		/* User Code loop ------------------------------------------------------------*/
		serialTask(&dataRx,&dataTx);
		
		/* END User Code loop --------------------------------------------------------*/
	}
	return 0;
}
/*ISR(USART0_RX_vect){
	dataRx.buff[dataRx.indexW++] = UDR0;
	dataRx.indexW &= dataRx.mask;
	PORTB &= ~(1<<LED);	
}*/
void uart_transmit(uint8_t data) {
	while (!(UCSR0A & (1 << UDRE0))); // Esperar a que el buffer esté vacío
	UDR0 = data;
	PORTB ^= (1<<LED);
}
// Interrupción cuando se recibe un dato por UART0
ISR(USART0_RX_vect) {
	uint8_t received = UDR0;  // Leer el dato recibido
	uart_transmit(received); // Hacer eco (enviar de vuelta)
	
}
ISR(TIMER4_COMPA_vect){
	OCR1A += 2000;
	count10ms--;
	if(!count10ms){
		IS10MS = TRUE;							// Flag that indicates that 10 ms have passed
		count10ms = 10;
	}
}
/*ISR(TIMER4_COMPB_vect){
	on_timer1_compb_hcsr();
}*/

/*ISR(TIMER4_CAPT_vect){
	on_timer1_capt_hcsr();
}*/