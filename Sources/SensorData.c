/*
 * SensorData.c
 *
 *  Created on: 12.05.2018
 *      Author: JumpStart
 */


#include "SensorData.h"
#include "Application.h"

typedef struct Sensor{
	uint8_t ID;
	uint16_t Value;
}Sens_t;

typedef struct Frame{
	Sens_t Frame[SENS_COUNT];
	bool xAxis;		//TRUE if X Axis, FALSE if Y Axis
}Frame_t;

static Frame_t mux1;
static Frame_t mux2;

uint8_t saveValToFrame(uint8_t muxID, uint8_t sensorID, uint16_t val){
	if(muxID == 1){
		mux1.Frame[sensorID].Value = val;
	}else if(muxID == 2){
		mux2.Frame[sensorID].Value = val;
	}
}

uint8_t SensorData_Init(void){
	int i;

	mux1.xAxis = TRUE;
	for(i = 0 ; i < SENS_COUNT ; i++){
		mux1.Frame[i].ID = i;
		mux1.Frame[i].Value = 0;
	}

	mux2.xAxis = FALSE;
	for(i = 0 ; i < SENS_COUNT ; i++){
		mux2.Frame[i].ID = i;
		mux2.Frame[i].Value = 0;
	}
}
