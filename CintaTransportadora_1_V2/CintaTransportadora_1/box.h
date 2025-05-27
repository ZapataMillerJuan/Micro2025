#ifndef BOX_H_
#define BOX_H_

#include "utilities.h"
#include "delay.h"

#define MAX_BOX_BUFFER			16

#define IS_OLD_BOX				boxFlag.bits.bit0

typedef enum e_type{
	NO_TYPE = 0xFF,
	TYPE_A = 0x01,
	TYPE_B = 0x02,
	TYPE_C = 0x03,
	DISCARD = 0xDD
}eType;

typedef struct sBox{
	eType type;
}s_Box;

struct{
	struct HCSR_data{
		uint8_t prom_quant;
		uint8_t prom_iter;
		uint16_t auxHeight; 
	}HCSR;
	struct{
		uint16_t hA;
		uint16_t hB;
		uint16_t hC;
		uint16_t width;
		uint8_t margin;
	}BOX;
}sConfigBox;

#define COUNTINGFOR_FALL_IR0	s_Belt.flag.bits.bit0
#define COUNTINGFOR_RISE_IR1	s_Belt.flag.bits.bit1
#define IS_SPEED_MEASURING		s_Belt.flag.bits.bit2

typedef struct{
	uint8_t IR;
	uint8_t servo;
	uint8_t selectedType;
	uint8_t readIndex;
	uint8_t dropIt;
	s_delay servoHitTime;
}s_Actuators;

struct{
	uint16_t speed;
	flags flag;
}s_Belt;

flags boxFlag;
uint8_t indexBox;

void initialize_boxes();

void addBox(uint16_t altura);

uint8_t getBoxType(uint8_t ind);

void setBoxType(uint8_t ind, eType type);

#endif // BOX_H_