#include <m2560def.inc>

;igualdades
.equ	softversion=0x01
.equ	build=0x04
.equ	centuria=20
.equ	anno=17
.equ	mes=1
.equ	dia=29
.equ	LED = 7
.equ	SW0 = 6
.equ	num = 3997
.equ	tenms = 100
;definiciones - nombres simb?licos
.def	w=r16
.def	w1=r17
.def	saux=r18
.def	flag1=r19
.def	newButton=r21

;**** GPIOR0 como regsitros de banderas ****
.equ LASTSTATESW0	= 0
.equ TIME		= 1

;Segmento de EEPROM
;.eseg
;econfig:	.BYTE	1

;constantes
;const2:		.DB 1, 2, 3


;segmento de Datos SRAM
.dseg
statboot:	.BYTE	1
addrrx:		.BYTE	2
RXBUF:		.BYTE	24
LEDTIME:	.BYTE	2	
timeLedOn:	.BYTE	2


;segmento de C?digo
.cseg
.org	0x00
	jmp	start
/*;interrupciones	
.org	0x16
	jmp	serv_rx0
.org	0x26
	jmp	serv_cmp0

	*/
;constantes
consts:		.DB 0, 255, 0b01010101, -128, 0xaa,0
varlist:	.DD 0, 0xfadebabe, -2147483648, 1 << 30


;Servicio de interrupciones
serv_rx0:
	reti

serv_cmp0:
	reti



;**** Funciones ****

ini_ports:
	ldi r16, (1<<LED) | (0<<SW0)
	out ddrb,r16
	ldi r16, (1<<SW0)
	out PORTB, r16
	ret
delay1ms:
	ldi	r24, low(num) ; NUM = 2BYTES 
	ldi	r25, high(num)

delayloop: 
	sbiw	r25:r24, 1
	brne	delayloop
	nop
	nop
	ret
resetTimer:
	cbi	GPIOR0, TIME
	clr	r16
	sts	LEDTIME+0, r16
	sts	LEDTIME+1, r16
	ret
changeLed: 
	ldi	r21, (1<<LED)
	in	r20, PINB
	eor	r20, r21
	out	PORTB, r20

	sbic	GPIOR0, TIME
	call	resetTimer
	jmp	loop
;ini_serie0:
	;ret


;Like a main in C
start:
	cli ;desactiva las interrup  
	call	ini_ports
	ldi	r16, 0

	sts	LEDTIME+0, r16 ;Guardado en la SRAM
	sts	LEDTIME+1, r16

	ldi	r16, low(tenms);Se le pasa la parte baja de tenms a r16
	sts	timeLedOn+0, r16;Se guarda

	ldi	r16, high(tenms);Se pasa la parte alta
	sts	timeLedOn+1, r16; se guarda
	sbi	PORTB, LED

	;call	ini_serie0
loop:
	call	delay1ms
	lds	r26, LEDTIME+0;lee directamente de la SRAM
	lds	r27, LEDTIME+1
	adiw	r26, 1 ;se va sumando de a uno r26
	sts	LEDTIME+0, r26;Se guarda
	sts	LEDTIME+1, r27 

doLed:	
	//este segmento entra en change led si timeled = 1000 || 100 ms
	ldi	r16, low(tenms)
	ldi	r17, high(tenms)
	cp	r16, r26
	cpc	r17, r27
	breq changeLed
	sbi	GPIOR0, TIME
	ldi	r16, low(1000)
	ldi	r17, high(1000)
	cp	r16, r26	;
	cpc	r17, r27	;
	breq	changeLed	;
	cbi GPIOR0, TIME
	jmp	loop