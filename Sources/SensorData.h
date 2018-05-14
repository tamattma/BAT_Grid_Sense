/*
 * SensorData.h
 *
 *  Created on: 12.05.2018
 *      Author: JumpStart
 */

#ifndef SOURCES_SENSORDATA_H_
#define SOURCES_SENSORDATA_H_

#include <stdint.h>

uint8_t saveValToFrame(uint8_t muxID, uint8_t sensorID, uint16_t val);

uint8_t SensorData_Init(void);

#endif /* SOURCES_SENSORDATA_H_ */
