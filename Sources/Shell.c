/*
 * Shell.c
 *
 *  Created on: 27.07.2017
 *      Author: Remo
 */


#include "CLS1.h"
#include "AS1.h"
#include "Shell.h"
#include "FRTOS1.h"
#include "RTT1.h"
#include "EE241.h"
#include "Application.h"


static bool UART_KeyPressed(void) {
return AS1_GetCharsInRxBuf()!=0;
}

static void UART_SendChar(uint8_t ch) {
CLS1_SendCharFct(ch, AS1_SendChar);
}

static void UART_ReceiveChar(uint8_t *p) {
if (AS1_RecvChar(p)!=ERR_OK) {
  *p = '\0';
}
}

static CLS1_ConstStdIOType UART_stdio = {
.stdIn = UART_ReceiveChar,
.stdOut = UART_SendChar,
.stdErr = UART_SendChar,
.keyPressed = UART_KeyPressed,
};

static uint8_t UART_DefaultShellBuffer[CLS1_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */

typedef struct {
  CLS1_ConstStdIOType *stdio;
  unsigned char *buf;
  size_t bufSize;
} SHELL_IODesc;

static void SHELL_SendChar(uint8_t ch) {
#if SHELL_CONFIG_HAS_SHELL_RTT
  RTT1_SendChar(ch);
#endif
#if SHELL_CONFIG_HAS_EXTRA_UART
  UART_SendChar(ch);
#endif
#if SHELL_CONFIG_HAS_SHELL_CDC
  CDC1_SendChar(ch);
#endif
}

static void SHELL_ReadChar(uint8_t *p) {
  *p = '\0'; /* default, nothing available */
#if SHELL_CONFIG_HAS_SHELL_RTT
  if (RTT1_stdio.keyPressed()) {
    RTT1_stdio.stdIn(p);
    return;
  }
#endif
#if SHELL_CONFIG_HAS_EXTRA_UART
  if (UART_stdio.keyPressed()) {
    UART_stdio.stdIn(p);
    return;
  }
#endif
#if SHELL_CONFIG_HAS_SHELL_CDC
  if (CDC1_stdio.keyPressed()) {
    CDC1_stdio.stdIn(p);
    return;
  }
#endif
}

static bool SHELL_KeyPressed(void) {
#if SHELL_CONFIG_HAS_SHELL_RTT
  if (RTT1_stdio.keyPressed()) {
    return TRUE;
  }
#endif
#if SHELL_CONFIG_HAS_EXTRA_UART
  if (UART_stdio.keyPressed()) {
    return TRUE;
  }
#endif
#if SHELL_CONFIG_HAS_SHELL_CDC
  if (CDC1_stdio.keyPressed()) {
    return TRUE;
  }
#endif
  return FALSE;
}

CLS1_ConstStdIOType SHELL_stdio =
{
  (CLS1_StdIO_In_FctType)SHELL_ReadChar, /* stdin */
  (CLS1_StdIO_OutErr_FctType)SHELL_SendChar, /* stdout */
  (CLS1_StdIO_OutErr_FctType)SHELL_SendChar, /* stderr */
  SHELL_KeyPressed /* if input is not empty */
};

static uint8_t SHELL_DefaultShellBuffer[CLS1_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */

CLS1_ConstStdIOType *SHELL_GetStdio(void) {
  return &SHELL_stdio;
}

static const SHELL_IODesc ios[] =
{
    {&SHELL_stdio, SHELL_DefaultShellBuffer, sizeof(SHELL_DefaultShellBuffer)},
};

/* forward declaration */
static uint8_t SHELL_ParseCommand(const unsigned char *cmd, bool *handled, const CLS1_StdIOType *io);

static const CLS1_ParseCommandCallback CmdParserTable[] =
{
  CLS1_ParseCommand, /* Processor Expert Shell component, is first in list */
  SHELL_ParseCommand, /* our own module parser */
  APP_ParseCommand,
  EE241_ParseCommand,
  NULL /* Sentinel */
};

static uint32_t SHELL_val;

void SHELL_SendString(unsigned char *msg) {
#if PL_CONFIG_HAS_SHELL_QUEUE
  SQUEUE_SendString(msg);
#elif CLS1_DEFAULT_SERIAL
  CLS1_SendStr(msg, CLS1_GetStdio()->stdOut);
#else
#endif
}

/*!
 * \brief Prints the help text to the console
 * \param io StdIO handler
 * \return ERR_OK or failure code
 */
static uint8_t SHELL_PrintHelp(const CLS1_StdIOType *io) {
  CLS1_SendHelpStr("Shell", "Shell commands\r\n", io->stdOut);
  CLS1_SendHelpStr("  help|status", "Print help or status information\r\n", io->stdOut);
  CLS1_SendHelpStr("  val <num>", "Assign number value\r\n", io->stdOut);
  return ERR_OK;
}

/*!
 * \brief Prints the status text to the console
 * \param io StdIO handler
 * \return ERR_OK or failure code
 */
static uint8_t SHELL_PrintStatus(const CLS1_StdIOType *io) {
  uint8_t buf[16];

  CLS1_SendStatusStr("Shell", "\r\n", io->stdOut);
  UTIL1_Num32sToStr(buf, sizeof(buf), SHELL_val);
  UTIL1_strcat(buf, sizeof(buf), "\r\n");
  CLS1_SendStatusStr("  val", buf, io->stdOut);
  return ERR_OK;
}

static uint8_t SHELL_ParseCommand(const unsigned char *cmd, bool *handled, const CLS1_StdIOType *io) {
  uint32_t val;
  const unsigned char *p;

  if (UTIL1_strcmp((char*)cmd, CLS1_CMD_HELP)==0 || UTIL1_strcmp((char*)cmd, "Shell help")==0) {
    *handled = TRUE;
    return SHELL_PrintHelp(io);
  } else if (UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, "Shell status")==0) {
    *handled = TRUE;
    return SHELL_PrintStatus(io);
  } else if (UTIL1_strncmp(cmd, "Shell val ", sizeof("Shell val ")-1)==0) {
    p = cmd+sizeof("Shell val ")-1;
    if (UTIL1_xatoi(&p, &val)==ERR_OK) {
      SHELL_val = val;
      *handled = TRUE;
    } else {
      return ERR_FAILED; /* wrong format of command? */
    }
  }
  return ERR_OK;
}

void SHELL_ParseCmd(uint8_t *cmd) {
  (void)CLS1_ParseWithCommandTable(cmd, ios[0].stdio, CmdParserTable);
}

static void ShellTask(void *pvParameters) {
  int i;
  /* \todo Extend as needed */

  (void)pvParameters; /* not used */
  /* initialize buffers */
  for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0';
  }
  SHELL_SendString("Shell task started!\r\n");
#if CLS1_DEFAULT_SERIAL
  (void)CLS1_ParseWithCommandTable((unsigned char*)CLS1_CMD_HELP, ios[0].stdio, CmdParserTable);
#endif
  for(;;) {
    /* process all I/Os */
    for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
      (void)CLS1_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
    }
    /*
    unsigned char *msg;
    if((msg = SQUEUE_ReceiveMessage()) != NULL){
    	CLS1_SendStr(msg,SHELL_stdio.stdOut);
    	vPortFree(msg);
    }
    */
    vTaskDelay(pdMS_TO_TICKS(10));
  } /* for */
}

void SHELL_Init(void) {
  CLS1_SetStdio(SHELL_GetStdio()); /* set default standard I/O to RTT */
  if (FRTOS1_xTaskCreate(ShellTask, "Shell", configMINIMAL_STACK_SIZE+100, NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
}

void SHELL_Deinit(void) {
}
