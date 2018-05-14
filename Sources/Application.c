/*
 * Application.c
 *
 *  Created on: 11.05.2018
 *      Author: JumpStart
 */


#include "PE_Error.h"
#include "Application.h"
#include "FRTOS1.h"
#include "Bit1.h"
#include "Bit2.h"
#include "Bit3.h"
#include "Bit4.h"
#include "AD1.h"

uint8_t channel_Nr;
uint16_t current_Val;

uint8_t App_SelectChannel(uint8_t SensorID){
	if(SensorID <= SENS_COUNT){
		Bit1_PutVal( (SensorID & 0x1) );
		Bit2_PutVal( (SensorID & 0x2) );
		Bit3_PutVal( (SensorID & 0x4) );
		Bit4_PutVal( (SensorID & 0x8) );
		channel_Nr = SensorID;
		return ERR_OK;
	}else{
		return ERR_RANGE;
	}
}

uint8_t App_GetValue(uint8_t SensorID, uint16_t* result){
	if(App_SelectChannel(SensorID)==ERR_OK){
		if(AD1_Measure(TRUE)==ERR_OK){
			AD1_GetValue16(result);
		}else
			return ERR_FAILED;
	}else
		return ERR_RANGE;
}

void App_PrintChannelValue(uint8_t channel, uint16_t val, const CLS1_StdIOType *io){
	CLS1_SendStr("Channel: \t", io->stdOut);
			CLS1_SendNum8u(channel, io->stdOut);
			CLS1_SendStr("\t Value: \t", io->stdOut);
			CLS1_SendNum16u(val, io->stdOut);
			CLS1_SendStr("\n\r", io->stdOut);
}

uint8_t App_IterateChannels(const CLS1_StdIOType *io){
	uint16_t result;
	for(uint8_t i=0 ; i < SENS_COUNT ; i++){
		App_GetValue(i, &result);
		App_PrintChannelValue(i, result, io);
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	return ERR_OK;
}


/*!
 * \brief Prints the help text to the console
 * \param io StdIO handler
 * \return ERR_OK or failure code
 */
static uint8_t APP_PrintHelp(const CLS1_StdIOType *io) {
  CLS1_SendHelpStr("App", "Application commands\r\n", io->stdOut);
  CLS1_SendHelpStr("  help|status", "Print help or status information\r\n", io->stdOut);
  CLS1_SendHelpStr("  channel <num>", "Select mux channel\r\n", io->stdOut);
  CLS1_SendHelpStr("  iterate", "Iterate through channels and print sensor values\r\n", io->stdOut);
  return ERR_OK;
}

/*!
 * \brief Prints the status text to the console
 * \param io StdIO handler
 * \return ERR_OK or failure code
 */
static uint8_t APP_PrintStatus(const CLS1_StdIOType *io) {
  uint8_t buf[16];

  CLS1_SendStatusStr("App", "\r\n", io->stdOut);
  UTIL1_Num32sToStr(buf, sizeof(buf), channel_Nr);
  UTIL1_strcat(buf, sizeof(buf), "\r\n");
  CLS1_SendStatusStr("  Channel", buf, io->stdOut);
  return ERR_OK;
}


uint8_t APP_ParseCommand(const uint8_t *cmd, bool *handled, const CLS1_StdIOType *io) {
  uint32_t val;
  const unsigned char *p;

  if (UTIL1_strcmp((char*)cmd, CLS1_CMD_HELP)==0 || UTIL1_strcmp((char*)cmd, "App help")==0) {
    *handled = TRUE;
    return APP_PrintHelp(io);
  } else if (UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, "App status")==0) {
    *handled = TRUE;
    return APP_PrintStatus(io);
  } else if (UTIL1_strncmp(cmd, "App channel ", sizeof("App channel ")-1)==0) {
    p = cmd+sizeof("App channel ")-1;
    if (UTIL1_xatoi(&p, &val)==ERR_OK) {
      if(App_SelectChannel(val)==ERR_OK){
      }else{
    	  CLS1_SendStr("sum ting wong\n\r",io->stdErr);
      }
      *handled = TRUE;
    } else {
      return ERR_FAILED; /* wrong format of command? */
    }
  } else if (UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, "App value")==0) {
	    *handled = TRUE;
	    App_PrintChannelValue(channel_Nr, current_Val, io);
  } else if (UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, "App iterate")==0) {
      *handled = TRUE;
      App_IterateChannels(io);
    }
  return ERR_OK;
}

static void AppTask(void *pvParameters) {


	for(;;){
		if(App_GetValue(channel_Nr, &current_Val) != ERR_OK){
			//error
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}


void APP_Start(){
	if (FRTOS1_xTaskCreate(AppTask, "App", configMINIMAL_STACK_SIZE+100, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
	    for(;;){} /* error */
	  }

}





