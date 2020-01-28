#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "app_mng.h"
#include "drv_uart.h"
#include "cmd.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    uint32_t freq;
    uint32_t amp;
    uint32_t mkey;
    uint32_t cs;
} App_Settings_TypeDef;

/* Private define ------------------------------------------------------------*/
#define APP_SW_VERSION_STR              CONFIG_SW_VERSION_STR
#define APP_MAGIC_KEY_NUMBER            ((uint32_t)0x14ab9afe84ab9afa)
#define APP_SETTINGS_START_ADR          ADDR_FLASH_PAGE_63
#define APP_SETTINGS_END_ADR            ADDR_FLASH_PAGE_64

#define APP_SIN_SAMPLES                 100
#define PI                              3.1415926
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void _APP_InitModuls(void);
static void _APP_AddModulsTask(void);

static void App_Calculate_Sin_Val(void);

static CMD_Ret_Type _APP_Cmd_Exe_Set_Settings(char *pIn, char **ppOut);
static CMD_Ret_Type _APP_Cmd_Exe_Get_Settings(char *pIn, char **ppOut);

/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;
extern DAC_HandleTypeDef hdac;

static CMD_Node_TypeDef App_Cmd_Node_Buf[] = {
        {CMD_FUNC_SET_SETTINGS, _APP_Cmd_Exe_Set_Settings, NULL},
        {CMD_FUNC_GET_SETTINGS, _APP_Cmd_Exe_Get_Settings, NULL},
};

#define APP_CMD_NUMOF_NODES    (sizeof(App_Cmd_Node_Buf) / sizeof(App_Cmd_Node_Buf[0]))

static struct
{
    App_Settings_TypeDef Settings;
    char *TxBuf;
    uint16_t TxBuf_Size;
    char Args_Buf[2000];
} hApp_Ins;

uint32_t sin_val[APP_SIN_SAMPLES];

/* ---------------------------------------------------------------------------*/
#ifdef CONFIG_USE_IWDG
extern IWDG_HandleTypeDef hiwdg;
#endif

/**
* @brief  Hard Error Function
* @param  none
* @retval none
*/
void APP_Error_Func(void)
{

}

/**
* @brief  Init
* @param  none
* @note   User should call this function in order to use it
* @retval none
*/
void APP_Init(void)
{
    _APP_InitModuls();
    _APP_AddModulsTask();
}

/**
  * @brief  Init all system module
  * @param  none
  * @retval none
  */
static void _APP_InitModuls(void)
{
    UART_HandleTypeDef *tphuart;
    CMD_Periph_TypeDef hcmd;
    char *tCh;
    uint16_t tBufSize;

    DRV_UART_Init(&huart2, eDRV_UART_CMD_MODULE);
    // CMD Init
    (void) DRV_UART_GetHandler(eDRV_UART_CMD_MODULE, &tphuart);
    hcmd.puart = tphuart;
    CMD_Init(hcmd);
    CMD_Init_Nodes(App_Cmd_Node_Buf, APP_CMD_NUMOF_NODES);
    CMD_Get_Tx_Buf(&tCh, &tBufSize);
    hApp_Ins.TxBuf = tCh;
    hApp_Ins.TxBuf_Size = tBufSize;
    snprintf(hApp_Ins.TxBuf, hApp_Ins.TxBuf_Size, "Starting: %s\r\n", CONFIG_SW_VERSION);
    CMD_Data_Send(hApp_Ins.TxBuf, (uint16_t) strlen(hApp_Ins.TxBuf), eCMD_SEND_ONLY);

    hApp_Ins.Settings.amp = 1650;
    hApp_Ins.Settings.freq = 500;

    App_Calculate_Sin_Val();
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, sin_val, APP_SIN_SAMPLES, DAC_ALIGN_12B_R);
}

/**
* @brief  Add system module task
* @param  none
* @retval none
*/
static void _APP_AddModulsTask(void)
{

}

/**
  * @brief  run
  * @param  arg: pointer to arguments
  * @retval 0
*/
void APP_Run(void *arg)
{
#ifdef CONFIG_USE_IWDG
    HAL_IWDG_Refresh(&hiwdg);
#endif
    (void) CMD_Run(arg);
}

/**
  * @brief  calculate sin values
  * @param  void
  * @retval void
*/
static void App_Calculate_Sin_Val(void)
{
    for (int i = 0; i < APP_SIN_SAMPLES; i++)
    {
//        sin_val[i] = ((sin(i * 2 * PI / APP_SIN_SAMPLES) + 1) * (4096 / 2));
        sin_val[i] = (((sin(i * 2 * PI / APP_SIN_SAMPLES) + 1) * 2048 * hApp_Ins.Settings.amp) / 1650);
    }
    HAL_TIM_Base_DeInit(&htim2);
    htim2.Init.Period = 100000 / hApp_Ins.Settings.freq;
    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start(&htim2);
}

/**
* @brief  Cmd function Exe, Insert fault insertion Settings
* @param  none
* @retval none
*/
static CMD_Ret_Type _APP_Cmd_Exe_Set_Settings(char *pIn, char **ppOut)
{
    uint32_t tamp;
    uint32_t tfreq;
    uint32_t i;

    // <temp>&<hum>&<co2>&<mode>
    tamp = (uint32_t) atof(pIn);
    if ((pIn = strstr(pIn, "&")) != NULL)
    {
        tfreq = (uint32_t) atof(++pIn);
        if (tamp <= 1650 && (tfreq >= 500 && tfreq <= 1500))
        {
            hApp_Ins.Settings.amp = tamp;
            hApp_Ins.Settings.freq = tfreq;
            App_Calculate_Sin_Val();
            snprintf(hApp_Ins.Args_Buf, sizeof(hApp_Ins.Args_Buf) - 1,
                     "\r\namp=%ld[mv]\r\n"
                     "freq=%ld[Hz]\r\n"
                     "samples=%d\r\n"
                     "wave:\r\n",
                     hApp_Ins.Settings.amp,
                     hApp_Ins.Settings.freq,
                     APP_SIN_SAMPLES);
            for (i = 0; i < APP_SIN_SAMPLES; i++)
            {
                sprintf((hApp_Ins.Args_Buf + strlen(hApp_Ins.Args_Buf)), "%ld\r\n", sin_val[i]);
            }
            *ppOut = hApp_Ins.Args_Buf;
            return eCMD_RET_OK;
        }
    }
    return eCMD_RET_FAIL;
}

/**
* @brief  Cmd function Exe, get fault insertion Settings
* @param  none
* @retval none
*/
static CMD_Ret_Type _APP_Cmd_Exe_Get_Settings(char *pIn, char **ppOut)
{
    uint32_t i;

    (void) pIn;
    snprintf(hApp_Ins.Args_Buf, sizeof(hApp_Ins.Args_Buf) - 1,
             "\r\namp=%ld[mv]\r\n"
             "freq=%ld[Hz]\r\n"
             "samples=%d\r\n"
             "wave:\r\n",
             hApp_Ins.Settings.amp,
             hApp_Ins.Settings.freq,
             APP_SIN_SAMPLES);
    for (i = 0; i < APP_SIN_SAMPLES; i++)
    {
        sprintf((hApp_Ins.Args_Buf + strlen(hApp_Ins.Args_Buf)), "%ld\r\n", sin_val[i]);
    }

    *ppOut = hApp_Ins.Args_Buf;
    return eCMD_RET_OK;
}
