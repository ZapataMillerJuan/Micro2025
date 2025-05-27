#include <m2560def.inc>

;igualdades
.equ	LEDBUILTIN = PB7
.equ	SW0 = PB4

.equ	BUFSIZERX = 32
.equ	BUFSIZETX = 32
//ESTADOS
.equ	STATEIDLE	= 0xAAAAAA00
.equ	STATEUNO	= 0x80000000
.equ	STATEDOS	= 0xA0000000
.equ	STATETRES	= 0XA8000000
.equ	STATECUATRO = 0XAA000000
.equ	STATECINCO	= 0XFE808080
.equ	STATESEIS	= 0XFEA0A0A0
.equ	STATESIETE	= 0xFEA8A8A8
.equ	STATEOCHO	= 0xFE000000
.equ	STATENUEVE	= 0xFEFE0000
.equ	STATEDIEZ	= 0xFEFEFE00
;**** GPIOR0 como regsitros de banderas ****
.equ	LASTSTATESW0 = 0	;GPIOR0<0>: ultimo estado del pulsador
;GPIOR0<1>: 
;GPIOR0<2>:  
;GPIOR0<3>:  
;GPIOR0<4>:  
.equ	IS10MS = 5		;GPIOR0<5>: is 10ms 
;GPIOR0<6>:
.equ	ISNEWSTATESW0 = 7	;GPIOR0<7>: SW0 un nuevo estado
;****	

;segmento de Datos SRAM
.dseg
bufRX:		.BYTE	BUFSIZERX
iwRX:		.BYTE	1
irRX:		.BYTE	1
bufTX:		.BYTE	BUFSIZETX
iwTX:		.BYTE	1
irTX:		.BYTE	1
time10ms:	.BYTE	1
time100ms:	.BYTE	1
dbSW0:		.BYTE	1
maskState:	.BYTE	4
ledState:	.BYTE	4



;segmento de C?digo
.cseg
.org	0x00
	jmp	start
.org	0x1C
	jmp	TIM0_COMPA


.org	0x34
;**** CONSTANTS in FLASH ****
version:	.db "20250314_01b01",'\n','\0'
msgTest:	.db "TEST",'\n','\0'
msgStateIdle:	.db "STATE IDLE",'\n','\0'
msgStateUno:	.db "STATE UNO",'\n','\0','\0'

;**** INTERRUPTS ****
TIM0_COMPA:
	in	r2, SREG
	push	r2
	push	r16
	lds	r16, time10ms
	dec	r16
	sts	time10ms, r16
	brne	OUT_TIM0_COMPA
	sbi	GPIOR0, IS10MS
	ldi	r16, 5
	sts	time10ms, r16
OUT_TIM0_COMPA:
	pop	r16
	pop	r2
	out	SREG, r2
	reti	
;**** Funciones ****
doLed:

	in		r16,PORTB
	ldi		r17,(1<<LEDBUILTIN)
	eor		r16, r17	
	out		PORTB,r16

	//jmp		loop
	ret
ini_ports:
	ldi	r16, (1 << LEDBUILTIN) | (0 << SW0)
	out	DDRB, r16
	ldi	r16, (1 << SW0)
	out	PORTB, r16
	ret

; fclk/256 - Mode CTC - OCR0A: 125 -> 2ms
ini_TMR0:
	ldi	r16, 0
	out	TCNT0, r16
	ldi	r16, 0x02
	out	TCCR0A, r16
	ldi	r16, 0x07
	out	TIFR0, r16
	ldi	r16, 0x02
	sts	TIMSK0, r16
	ldi	r16, 125
	out	OCR0A, r16
	ldi	r16, 0b00000100
	out	TCCR0B, r16
	ret
do100ms:
	cbi		GPIOR0, IS10MS
	lds	r16, time100ms
	dec	r16
	sts	time100ms, r16
	breq	PC+2
	ret
	ldi	r16, 10
	sts	time100ms, r16
	lds	r12, maskState+0
	lds	r13, maskState+1
	lds	r14, maskState+2
	lds	r15, maskState+3
	lds	r8, ledState+0
	lds	r9, ledState+1
	lds	r10, ledState+2
	lds	r11, ledState+3
	and	r8, r12
	and	r9, r13
	and	r10, r14
	and	r11, r15
	or	r8, r9
	or	r10, r11
	or	r8, r10
	breq	doLed
	//breq	ledOff
	//call doLed
	//sbi	PORTB, LEDBUILTIN	;led ON
	rjmp	saveMask
	ret
ledOff:
	cbi	PORTB, LEDBUILTIN
saveMask:
	clc
	ror	r15
	ror	r14
	ror	r13
	ror	r12
	brcc	PC+3
	ldi	r16, 0x80
	mov	r15, r16
	sts	maskState+0, r12
	sts	maskState+1, r13
	sts	maskState+2, r14
	sts	maskState+3, r15
	ret
doRX:
	lds	r16, irRX
	ldi	r26, low(bufRX)
	ldi	r27, high(bufRX)
	clr	r17
	add	r26, r16
	adc	r27, r17
	inc	r16
	andi	r16, BUFSIZERX-1
	sts	irRX, r16
	ld	r16, X
	ret
;Like a main in C
start:
	cli
	ldi	r16, low(RAMEND)
	out	SPL, r16
	ldi	r16, HIGH(RAMEND)
	out	SPH, r16
	call	ini_ports
	call	ini_TMR0
	ldi	r16, 5
	sts	time10ms, r16
	ldi	r16, 10
	sts	time100ms, r16
	out	GPIOR0, r16
	sei
loop:
	sbic	GPIOR0, IS10MS
	call	do100ms
	
	//rjmp	testBufRX
	
	jmp loop
testBufRX:
	lds	r17, iwRX
	lds	r16, irRX
	cp	r16, r17
	breq	testBufTX
	call	doRX

testBufTX:
	lds	r17, iwTX
	lds	r16, irTX
	cp	r16, r17
	brne	PC+2
	jmp	loop