/*****************************************************************************
 * Created by Alex on 5/24/2018.
 * Module ver 1.1
 * NOTE: This module needs 
 *
*****************************************************************************/

#ifndef _CMD_H
#define _CMD_H

#include "stm32f4xx_hal.h"
#include "config.h"

/* Global define ------------------------------------------------------------*/
//#define CMD_USE_COMM_INTERFACE_USB
#define CMD_USE_COMM_INTERFACE_UART

#define CMD_UART_BAUD_RATE       115200

#define CMD_PREFIX_STR           "CMD+"
#define CMD_POSTFIX_STR          "+END"

#define CMD_RES_OK               "+OK"
#define CMD_RES_FAIL             "+FAIL"
#define CMD_RES_UNKNOWN          "UNKNOWN"

#define CMD_FUNC_SET_SETTINGS  "SET="
#define CMD_FUNC_GET_SETTINGS  "GET"

/* Global typedef -----------------------------------------------------------*/
typedef enum
{
    eCMD_RET_FAIL = 0,
    eCMD_RET_OK,
} CMD_Ret_Type;

typedef enum
{
    eCMD_SEND_ONLY = 0,
    eCMD_COPY_AND_SEND
} Cmd_Send_Type;

typedef struct
{
#ifdef CMD_USE_COMM_INTERFACE_USB
    void *pNone;
#endif
#ifdef CMD_USE_COMM_INTERFACE_UART
    UART_HandleTypeDef *puart;
#endif
} CMD_Periph_TypeDef;

typedef struct
{
    uint16_t Com;
    uint16_t Cmd;
} CMD_Error_TypeDef;

typedef struct
{
    const char *pName;
    CMD_Ret_Type (*pFc_Exe)(char *pIn, char **ppOut);
    void (*pFunc_CB)(void);
} CMD_Node_TypeDef;

/* Global Call back functions -----------------------------------------------*/
#ifdef CMD_USE_COMM_INTERFACE_UART
void CMD_UART_RxCplt_Callback_func(UART_HandleTypeDef *phuart);
void CMD_UART_TxCplt_Callback_func(UART_HandleTypeDef *phuart);
void CMD_UART_Error_Callback_func(UART_HandleTypeDef *phuart);
#elif defined(CMD_USE_COMM_INTERFACE_UART_DUMMY)
void CMD_UART_RxCplt_Callback_func(UART_HandleTypeDef *phuart);
void CMD_UART_TxCplt_Callback_func(UART_HandleTypeDef *phuart);
void CMD_UART_Error_Callback_func(UART_HandleTypeDef *phuart);
#endif

/* Global function prototypes -----------------------------------------------*/

CMD_Ret_Type CMD_Init(CMD_Periph_TypeDef hPeriph);
CMD_Ret_Type CMD_Init_Nodes(CMD_Node_TypeDef *pNode, uint8_t NodeTotal);
CMD_Ret_Type CMD_Get_Tx_Buf(char **ppBuf, uint16_t *pBufSize);
CMD_Ret_Type CMD_Data_Send(const char *pData, uint16_t DataSize, Cmd_Send_Type Control);
CMD_Ret_Type CMD_Run(void *arg);

#endif //_CMD_H
