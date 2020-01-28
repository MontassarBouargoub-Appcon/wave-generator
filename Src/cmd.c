/*! \mainpage  cmd.c Module ver 1.1
 *
 * #############################################################
 * \section intro_sec Description
 * This module implement AT commands communication
 *
 * #############################################################
 * \section howtouse_sec How to use this Module
 *
 *
 * #############################################################
 * \section releasenote_sec Release notes
 *
 * <b> Module ver 1.0 </b> Release by ab, date
 *
 * First writing
 *
 * <b> Module ver 1.1 </b> Release by ab, date
 *
 * Add USB virtual com port functionality
 *
 ******************************************************************************/

#include "cmd.h"
#include "string.h"
#include <stdio.h>

#ifdef CMD_USE_COMM_INTERFACE_USB

#include "usbd_cdc_if.h"

#endif

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    eCMD_INIT_IDLE = 0,
    eCMD_INIT_DONE,
    eCMD_INIT_ERR,
} CMD_Init_Type;

typedef enum
{
    eCMD_COMM_COMPLETE_ST = 0,
    eCMD_COMM_INPROGRESS_ST,
    eCMD_COMM_IDLE_ST,
    eCMD_COMM_ERROR_ST,
} CMD_Comm_State_Type;

typedef struct
{
    uint8_t Tx_Buf[CONFIG_COMM_TX_MAX_SIZE];    //!< Tx working Buffer
    uint8_t *pRx_Buf;
    uint16_t Rx_Buf_Size;
    volatile uint32_t *pTx_State;               //!< Communication Tx State
    volatile CMD_Comm_State_Type Rx_State;      //!< Communication Rx State
    volatile CMD_Error_TypeDef Err;
    CMD_Init_Type gState;
} CMD_Handler_TypeDef;


/* Private define ------------------------------------------------------------*/
#define CMD_INTERNAL_NULL_STR        "NULL"


#if defined(CMD_USE_COMM_INTERFACE_UART) && defined(CMD_USE_COMM_INTERFACE_USB)
#error : Choose Only one communication interface!
#endif

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
#ifdef CMD_USE_COMM_INTERFACE_UART
static void _CMD_Config_Comm_Module(void);
static void _CMD_Init_Comm_Module(void);
static void _CMD_DeInit_Comm_Module(void);
#endif

static void _CMD_TxPkt_IT(uint8_t *buf, uint16_t bufSize);
static void _CMD_RxPkt_IT(uint8_t *buf, uint16_t bufSize);

static CMD_Ret_Type _CMD_Exist(const char *pName, CMD_Node_TypeDef **ppNode);

/* Private variables ---------------------------------------------------------*/
static CMD_Handler_TypeDef hCMD;                //!< Communication Handler
static CMD_Node_TypeDef *pCMD_Node = NULL;
static uint8_t CMD_NodeTotal;

#ifdef CMD_USE_COMM_INTERFACE_UART
static CMD_Periph_TypeDef hCMD_Periph;         //!< Periphiral
static volatile CMD_Comm_State_Type Cmd_Tx_State;
static uint8_t Cmd_Rx_Buf[CONFIG_COMM_RX_MAX_SIZE];    //!< Rx working Buffer
#endif

/**
  * @brief  Init module
  * @param  none
  * @note   User should call this function in order to use this module
  * @retval none
  */
CMD_Ret_Type CMD_Init(CMD_Periph_TypeDef hPeriph)
{
    hCMD.Rx_State = eCMD_COMM_IDLE_ST;

#ifdef CMD_USE_COMM_INTERFACE_UART
    hCMD_Periph = hPeriph;
    hCMD.pRx_Buf = Cmd_Rx_Buf;
    hCMD.Rx_Buf_Size = CONFIG_COMM_RX_MAX_SIZE - 1;
    hCMD.pTx_State = (volatile uint32_t *) &Cmd_Tx_State;
    Cmd_Tx_State = eCMD_COMM_IDLE_ST;
    _CMD_DeInit_Comm_Module();
    _CMD_Config_Comm_Module();
    _CMD_Init_Comm_Module();
#endif

#ifdef CMD_USE_COMM_INTERFACE_USB
    CDC_Get_Ref_Rx_Buf(&hCMD.pRx_Buf, &hCMD.Rx_Buf_Size);
    hCMD.Rx_Buf_Size--;
    hCMD.pTx_State = CDC_Get_Ref_Tx_State();
#endif

    return eCMD_RET_OK;
}

/**
  * @brief  Init Nodes
  * @param  none
  * @retval none
  */
CMD_Ret_Type CMD_Init_Nodes(CMD_Node_TypeDef *pNode, uint8_t NodeTotal)
{
    pCMD_Node = pNode;
    CMD_NodeTotal = NodeTotal;

    if (pCMD_Node != NULL && CMD_NodeTotal != 0)
    {
        hCMD.gState = eCMD_INIT_DONE;
    }
    else
    {
        hCMD.gState = eCMD_INIT_ERR;
    }
    return eCMD_RET_OK;
}

/**
  * @brief  Send data
  * @param  none
  * @retval none
  */
CMD_Ret_Type CMD_Data_Send(const char *pData, uint16_t DataSize, Cmd_Send_Type Control)
{
    if ((hCMD.pTx_State != NULL) && (*hCMD.pTx_State != eCMD_COMM_INPROGRESS_ST))
    {
        if (Control == eCMD_COPY_AND_SEND)
        {
            if (DataSize < sizeof(hCMD.Tx_Buf))
            {
                memcpy(hCMD.Tx_Buf, pData, DataSize);
                _CMD_TxPkt_IT(hCMD.Tx_Buf, DataSize);
                return eCMD_RET_OK;
            }
        }
        else
        {
            _CMD_TxPkt_IT((uint8_t *) pData, DataSize);
            return eCMD_RET_OK;
        }
    }
    return eCMD_RET_FAIL;
}

/**
  * @brief  Get reference to tx buffer
  * @param  none
  * @retval none
  */
CMD_Ret_Type CMD_Get_Tx_Buf(char **ppBuf, uint16_t *pBufSize)
{
    *ppBuf = (char *) hCMD.Tx_Buf;
    *pBufSize = sizeof(hCMD.Tx_Buf) - 1;

    return eCMD_RET_OK;
}

/**
  * @brief  main module task
  * @param  arg: pointer to arguments
  * @retval none
  */
CMD_Ret_Type CMD_Run(void *arg)
{
    static enum
    {
        eINTERNAL_STATE_INIT = 0,
        eINTERNAL_STATE_WAIT,
        eINTERNAL_STATE_RES,
    } State = eINTERNAL_STATE_INIT;
    char *tpCh1;
    char *tpCh2;
    char *tpChOut = CMD_INTERNAL_NULL_STR;
    static CMD_Node_TypeDef *pNode = NULL;

    (void) arg;
    if (hCMD.gState == eCMD_INIT_DONE)
    {
        switch (State)
        {
            case eINTERNAL_STATE_INIT:
                memset(hCMD.pRx_Buf, 0x00, hCMD.Rx_Buf_Size + 1);
                _CMD_RxPkt_IT(hCMD.pRx_Buf, hCMD.Rx_Buf_Size);
                State = eINTERNAL_STATE_WAIT;
                break;
            case eINTERNAL_STATE_WAIT:
                if ((tpCh1 = strstr((char *) hCMD.pRx_Buf, CMD_PREFIX_STR)) != NULL)
                {
                    if ((tpCh2 = strstr(tpCh1, CMD_POSTFIX_STR)) != NULL)
                    {
                        tpCh1 += strlen(CMD_PREFIX_STR);
                        *tpCh2 = '\0';
                        if (_CMD_Exist(tpCh1, &pNode) == eCMD_RET_OK)
                        {
                            if (pNode->pFc_Exe != NULL)
                            {
                                tpCh1 += strlen(pNode->pName); // Increment pointer to argument
                                if (pNode->pFc_Exe(tpCh1, &tpChOut) == eCMD_RET_OK)
                                {
                                    snprintf((char *) hCMD.Tx_Buf, sizeof(hCMD.Tx_Buf), "%sARG:%s%s%s\r\n",
                                             CMD_PREFIX_STR,
                                             tpChOut,
                                             CMD_RES_OK,
                                             CMD_POSTFIX_STR);
                                }
                                else
                                {
                                    snprintf((char *) hCMD.Tx_Buf, sizeof(hCMD.Tx_Buf), "%sARG:%s%s%s\r\n",
                                             CMD_PREFIX_STR,
                                             tpChOut,
                                             CMD_RES_FAIL,
                                             CMD_POSTFIX_STR);
                                }
                            }
                        }
                        else
                        {
                            snprintf((char *) hCMD.Tx_Buf, sizeof(hCMD.Tx_Buf), "%s%s%s\r\n",
                                     CMD_PREFIX_STR,
                                     CMD_RES_UNKNOWN,
                                     CMD_POSTFIX_STR);
                        }
                        _CMD_TxPkt_IT(hCMD.Tx_Buf, (uint16_t) strlen((char *) hCMD.Tx_Buf));
                        State = eINTERNAL_STATE_RES;
                    }
                }
                break;
            case eINTERNAL_STATE_RES:
                if (*hCMD.pTx_State == eCMD_COMM_COMPLETE_ST)
                {
                    if (pNode != NULL)
                    {
                        if (pNode->pFunc_CB != NULL)
                        {
                            pNode->pFunc_CB();
                        }
                        pNode = NULL;
                    }
                    State = eINTERNAL_STATE_INIT;
                }
                break;
            default:
                break;
        }
    }
    return eCMD_RET_OK;
}

/**
  * @brief  Encapsulate function by command name
  * @param  buf: pointer to buffer
  * @param  bufSize: buffer size
  * @retval none
  */
static CMD_Ret_Type _CMD_Exist(const char *pName, CMD_Node_TypeDef **ppNode)
{
    uint8_t i;

    for (i = 0; i < CMD_NodeTotal; i++)
    {
        if (strstr(pName, pCMD_Node[i].pName) != NULL)
        {
            *ppNode = &pCMD_Node[i];
            return eCMD_RET_OK;
        }
    }
    return eCMD_RET_FAIL;
}

/**
  * @brief  Transmit pkt with Interrupt 
  * @param  buf: pointer to buffer
  * @param  bufSize: buffer size
  * @retval none
  */
static void _CMD_TxPkt_IT(uint8_t *buf, uint16_t bufSize)
{

#ifdef CMD_USE_COMM_INTERFACE_UART
    *hCMD.pTx_State = eCMD_COMM_INPROGRESS_ST;
    HAL_UART_Transmit_IT(hCMD_Periph.puart, buf, bufSize);
#endif

#ifdef CMD_USE_COMM_INTERFACE_USB
    hCMD.pTx_State = CDC_Get_Ref_Tx_State();
    CDC_Transmit_FS(buf, bufSize);
#endif
}

/**
  * @brief  Receive pkt with interrupt
  * @param  buf: pointer to buffer
  * @param  bufSize: buffer size
  * @retval none
  */
static void _CMD_RxPkt_IT(uint8_t *buf, uint16_t bufSize)
{
#ifdef CMD_USE_COMM_INTERFACE_UART
    hCMD.Rx_State = eCMD_COMM_INPROGRESS_ST;
    hCMD_Periph.puart->RxState = HAL_UART_STATE_READY;
    HAL_UART_Receive_IT(hCMD_Periph.puart, buf, bufSize);
#endif
}

#ifdef CMD_USE_COMM_INTERFACE_UART

/**
  * @brief  Configure Communication module
  * @param  none
  * @retval none
  */
static void _CMD_Config_Comm_Module(void)
{
    hCMD_Periph.puart->Init.BaudRate = CMD_UART_BAUD_RATE;
    hCMD_Periph.puart->Init.WordLength = UART_WORDLENGTH_8B;
    hCMD_Periph.puart->Init.StopBits = UART_STOPBITS_1;
    hCMD_Periph.puart->Init.Parity = UART_PARITY_NONE;
    hCMD_Periph.puart->Init.Mode = UART_MODE_TX_RX;
    hCMD_Periph.puart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hCMD_Periph.puart->Init.OverSampling = UART_OVERSAMPLING_16;
}

/**
  * @brief  Init Communication module
  * @param  none
  * @retval none
  */
static void _CMD_Init_Comm_Module(void)
{
    (void) HAL_UART_Init(hCMD_Periph.puart);
    __HAL_UART_DISABLE_IT(hCMD_Periph.puart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(hCMD_Periph.puart, UART_IT_TXE);

}

/**
  * @brief  DeInit Communication module
  * @param  none
  * @retval none
  */
static void _CMD_DeInit_Comm_Module(void)
{
    hCMD_Periph.puart->RxXferSize = 0;
    hCMD_Periph.puart->RxXferCount = 0;
    (void) HAL_UART_DeInit(hCMD_Periph.puart);
}

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_RxCplt_Callback_func(UART_HandleTypeDef *phuart)
{
    if (phuart->Instance == hCMD_Periph.puart->Instance)
    {
        hCMD.Rx_State = eCMD_COMM_COMPLETE_ST;
    }
}

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_TxCplt_Callback_func(UART_HandleTypeDef *phuart)
{
    if (phuart->Instance == hCMD_Periph.puart->Instance)
    {
        *hCMD.pTx_State = eCMD_COMM_COMPLETE_ST;
    }
}

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_Error_Callback_func(UART_HandleTypeDef *phuart)
{
    if (phuart->Instance == hCMD_Periph.puart->Instance)
    {
        hCMD.Err.Com++;
        _CMD_DeInit_Comm_Module();
        _CMD_Init_Comm_Module();
        _CMD_RxPkt_IT(hCMD.pRx_Buf, hCMD.Rx_Buf_Size);
    }
}
#elif defined(CMD_USE_COMM_INTERFACE_UART_DUMMY)

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_RxCplt_Callback_func(UART_HandleTypeDef *phuart)
{

}

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_TxCplt_Callback_func(UART_HandleTypeDef *phuart)
{

}

/**
  * @brief  Rx complete HAL call back function
  * @param  arg: pointer to arguments
  * @retval none
  */
void CMD_UART_Error_Callback_func(UART_HandleTypeDef *phuart)
{

}

#endif