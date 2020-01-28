/*****************************************************************************
 * Created by alexb on 28/05/2016.
 * Module ver 1.2
 *
*****************************************************************************/
#ifndef DRV_UART_H
#define DRV_UART_H

#include "config.h"

/* Global define ------------------------------------------------------------*/
// Define or Un-Define in case of UART Debug module
//#define DRV_UART_USE_DEBUG_TX_MODULE
//#define DRV_UART_USE_DEBUG_RX_MODULE

/* Include all Modules that works with this driver --------------------------*/
#include "cmd.h"


#if defined(DRV_UART_USE_DEBUG_TX_MODULE) || defined(DRV_UART_USE_DEBUG_RX_MODULE)
#define DRV_UART_USE_DEBUG_MODULE

#include "debug.h"

#endif
/* Global typedef -----------------------------------------------------------*/
typedef void(*DRV_UART_pCallBackFunc)(UART_HandleTypeDef *);

//#warning : modify this structure according to number of UART in the system
typedef enum
{
    eDRV_UART_CMD_MODULE,  //!< UART For CMD Module
#ifdef DRV_UART_USE_DEBUG_MODULE
    eDRV_UART_DEBUG_MODULE,       //!< UART For DEBUG Module
#endif
    eDRV_UART_TOTAL
} DRV_UART_Type;

typedef enum
{
    eDRV_UART_RET_FAIL,
    eDRV_UART_RET_OK,
} DRV_UART_Ret_Type;

/* Global define ------------------------------------------------------------*/
/* Define Call back functions -----------------------------------------------*/
//#warning : User should add call back function of each module

// Call back UART RX complete
static const DRV_UART_pCallBackFunc DRV_UART_RxCplt_Callback_FuncArr[eDRV_UART_TOTAL] =
        {
                CMD_UART_RxCplt_Callback_func, //!< UART RX call back For CMD Module
#ifdef DRV_UART_USE_DEBUG_MODULE
                DEBUG_UART_Rx_Cplt_Callback   //!< UART RX Callback For DEBUG Module
#endif
        };

// Call back UART TX complete
static const DRV_UART_pCallBackFunc DRV_UART_TxCplt_Callback_FuncArr[eDRV_UART_TOTAL] =
        {
                CMD_UART_TxCplt_Callback_func, //!< UART TX call back For CMD Module
#ifdef DRV_UART_USE_DEBUG_MODULE
                DEBUG_UART_Tx_Cplt_Callback   //!< UART RX Callback For DEBUG Module
#endif
        };

// Call back UART Error
static const DRV_UART_pCallBackFunc DRV_UART_Error_Callback_FuncArr[eDRV_UART_TOTAL] =
        {
                CMD_UART_Error_Callback_func, //!< UART Error call back For CMD Module
#ifdef DRV_UART_USE_DEBUG_MODULE
                DEBUG_UART_Error_Callback    //!< UART Error Callback For DEBUG Module
#endif
        };

/* Global function prototypes -----------------------------------------------*/
DRV_UART_Ret_Type DRV_UART_Init(UART_HandleTypeDef *phUART, DRV_UART_Type UART_type);
DRV_UART_Ret_Type DRV_UART_GetHandler(DRV_UART_Type UART_type, UART_HandleTypeDef **pHuart);

#endif //DRV_UART_H
