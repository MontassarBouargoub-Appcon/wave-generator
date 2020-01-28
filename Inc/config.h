//
// Created by Alex on 08-Sep-17.
//

#ifndef CONFIG_H
#define CONFIG_H

#include "stdint.h"
#include "main.h"

/* Global define ------------------------------------------------------------*/
#define APP_DEBUG

// Definition for Sw is booloader or application
//#define CONFIG_SW_IS_BOOTLOADER
//#define CONFIG_USE_IWDG

// Definition for Hw / Sw version
#define CONFIG_HW_VERSION_STR                  "1.0.0"

#ifdef CONFIG_SW_IS_BOOTLOADER
#define CONFIG_SW_VERSION_H_STR                '3'
#define CONFIG_SW_VERSION_M_STR                '3'
#define CONFIG_SW_VERSION_L_STR                '1'
#else
#define CONFIG_SW_VERSION                      "wave-generator-0.0.1"
#endif

#define CONFIG_SW_VER_FOR_APP_NOT_FOUND_STR    "Not_Found"

#ifndef CONFIG_SW_IS_BOOTLOADER
//#define CONFIG_APPLICATON_IS_PRODUCTION
#endif

// Definition for Debug module
#define CONFIG_DEBUG_RX_BUF_SIZE               ((uint16_t)500)
#define CONFIG_DEBUG_TX_BUF_SIZE               ((uint16_t)500)

// Definition for Communication module
#define CONFIG_COMM_TX_MAX_SIZE                ((uint16_t)1200)
#define CONFIG_COMM_RX_MAX_SIZE                ((uint16_t)1200)

// Definition for Communication AES/Hashing
#define CONFIG_COMM_USE_AES
//#define CONFIG_COMM_USE_HASH

#endif //CONFIG_H
