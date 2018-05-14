/*
 * Application.h
 *
 *  Created on: 11.05.2018
 *      Author: JumpStart
 */

#ifndef SOURCES_APPLICATION_H_
#define SOURCES_APPLICATION_H_

#include "CLS1.h"

#define SENS_COUNT (16)

uint8_t APP_ParseCommand(const uint8_t *cmd, bool *handled, CLS1_ConstStdIOType *io);

void APP_Start(void);

#endif /* SOURCES_APPLICATION_H_ */
