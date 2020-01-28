/*! \mainpage  drv_uart.c Module ver 1.2
 * Created by alexb on 28/05/2016.
 *
 * #############################################################
 * \section intro_sec Description
 * This is a peripheral UART driver that link between specific UART and module.
 * User must define all parameters in interface.
 *
 * #############################################################
 * \section howtouse_sec How to use this Module
 *
 * 1. Include all Modules that have to work with this driver
 * 2. Modify enum structure in interface
 * 3. Modify Array of callback functions
 *
 * #############################################################
 * \section releasenote_sec Release notes
 *
 * <b> Module ver 1.0 </b> Release by alexb, date 28/05/2016
 *
 *   1. Head Version
 *
 * <b> Module ver 1.1 </b> Release by alexb, date 14/11/2016
 *
 *   1. Modify @ref DRV_UART_GetHandler function
 *   2. Added break loop in Callback functions for speedup
 *
 *   <b> Module ver 1.2 </b> Release by alexb, date 20/11/2016
 *
 *   1. Add Debug support
 ******************************************************************************/

#include "drv_uart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Call back function, called from HAL Drivers Interrupt
#define DRV_UART_RxCpltCallback_func        HAL_UART_RxCpltCallback
#define DRV_UART_TxCpltCallback_func        HAL_UART_TxCpltCallback
#define DRV_UART_ErrorCallback_func         HAL_UART_ErrorCallback
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef *DRV_UART_HandleArr[eDRV_UART_TOTAL];
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Init module
  * @param  none
  * @note   User should call this function in order to use this module
  * @retval Can be value of @ref DRV_UART_Ret_Type Type
  */
DRV_UART_Ret_Type DRV_UART_Init(UART_HandleTypeDef *phUART, DRV_UART_Type UART_type)
{
    if (UART_type < eDRV_UART_TOTAL)
    {
        DRV_UART_HandleArr[UART_type] = phUART;
        return eDRV_UART_RET_OK;
    }
    return eDRV_UART_RET_FAIL;
}

/**
  * @brief  Get UART handler struct
  * @param  UART_type: UART type (enum) declared in interface
  * @param  pHuart: doulble pointer to uart handler
  * @retval Can be value of @ref DRV_UART_Ret_Type Type
  */
DRV_UART_Ret_Type DRV_UART_GetHandler(DRV_UART_Type UART_type, UART_HandleTypeDef **pHuart)
{
    if (UART_type < eDRV_UART_TOTAL)
    {
        *pHuart = DRV_UART_HandleArr[UART_type];
        return eDRV_UART_RET_OK;
    }
    return eDRV_UART_RET_FAIL;
}

/**
  * @brief  Rx complete HAL Call back function
  * @param  hUART: pinter to UART handler
  * @retval none
  */
void DRV_UART_RxCpltCallback_func(UART_HandleTypeDef *hUART)
{
    uint8_t i;

    for (i = 0; i < eDRV_UART_TOTAL; i++)
    {
        if (DRV_UART_HandleArr[i]->Instance == hUART->Instance)
        {
#ifdef DRV_UART_USE_DEBUG_RX_MODULE
            (void) DEBUG_Add_Rx_Listener(hUART);
#endif
            DRV_UART_RxCplt_Callback_FuncArr[i](DRV_UART_HandleArr[i]);
            break;
        }
    }
}

/**
  * @brief  Tx complete HAL Call back function
  * @param  hUART: pinter to UART handler
  * @retval none
  */
void DRV_UART_TxCpltCallback_func(UART_HandleTypeDef *hUART)
{
    uint8_t i;

    for (i = 0; i < eDRV_UART_TOTAL; i++)
    {
        if (DRV_UART_HandleArr[i]->Instance == hUART->Instance)
        {
#ifdef DRV_UART_USE_DEBUG_TX_MODULE
            (void) DEBUG_Add_Tx_Listener(hUART);
#endif
            DRV_UART_TxCplt_Callback_FuncArr[i](DRV_UART_HandleArr[i]);
            break;
        }
    }
}

/**
  * @brief  Error HAL Call back function
  * @param  hUART: pinter to UART handler
  * @retval none
  */
void DRV_UART_ErrorCallback_func(UART_HandleTypeDef *hUART)
{
    uint8_t i;

    for (i = 0; i < eDRV_UART_TOTAL; i++)
    {
        if (DRV_UART_HandleArr[i]->Instance == hUART->Instance)
        {
            DRV_UART_Error_Callback_FuncArr[i](DRV_UART_HandleArr[i]);
            break;
        }
    }
}