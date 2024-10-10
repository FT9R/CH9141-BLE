#pragma once

#include "ch9141_Interface.h"

typedef ch9141_ErrorStatus_t (*ch9141_Receive_fp)(char *pDataRx, uint16_t size, uint16_t *rxLen, uint32_t timeout);
typedef ch9141_ErrorStatus_t (*ch9141_Transmit_fp)(char const *pDataTx, uint16_t size, uint32_t timeout);
typedef void (*ch9141_Pin_Sleep_fp)(ch9141_PinState_t newState);
typedef void (*ch9141_Pin_Mode_fp)(ch9141_PinState_t newState);
typedef void (*ch9141_Delay_fp)(uint32_t ms);

typedef enum ch9141_Error_e {
    CH9141_ERR_NONE,
    CH9141_ERR_ARGUMENT,
    CH9141_ERR_MEMORY,
    CH9141_ERR_SERIAL,
    CH9141_ERR_AT,
    CH9141_ERR_RESPONSE
} ch9141_Error_t;

typedef enum ch9141_AT_Error_e {
    CH9141_AT_ERR_CACHE = 1, // The current chip does not have a cache to respond, you can try again later
    CH9141_AT_ERR_PARAM, // Some parameters of the AT command that is sent do not meet the specifications
    CH9141_AT_ERR_CMD_SUP, // Commands are not supported in the current mode
    CH9141_AT_ERR_CMD_EXEC // The command cannot be executed temporarily
} ch9141_AT_Error_t;

typedef enum ch9141_Mode_e {
    CH9141_MODE_BROADCAST,
    CH9141_MODE_HOST,
    CH9141_MODE_DEVICE,
} ch9141_Mode_t;

typedef struct ch9141_s
{
    struct
    {
        ch9141_Receive_fp receive;
        ch9141_Transmit_fp transmit;
        ch9141_Pin_Sleep_fp pinSleep;
        ch9141_Pin_Mode_fp pinMode;
        ch9141_Delay_fp delay;
    } interface;
    char rxBuf[100];
    char txBuf[100];
    uint16_t rxLen; // Indicates number of data available in reception buffer
    char password[6 + 1];
    ch9141_Mode_t mode;
    ch9141_Error_t error; // Driver error codes
    ch9141_AT_Error_t errorAT; // Device error codes, relevant if handle.error = CH9141_ERR_AT
} ch9141_t;

void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode, ch9141_Delay_fp fpDelay);
void CH9141_Init(ch9141_t *handle);
void CH9141_Sleep(ch9141_t *handle, ch9141_FuncState_t newState);
void CH9141_PasswordGet(ch9141_t *handle);

/**
 * @brief
 * @param handle
 * @param newState
 * @return
 * @note Use example: `strcpy(ble1.password, "123456"); CH9141_PasswordSet(&handle, CH9141_FUNC_ENABLE);`
 */
void CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState);

void CH9141_HelloGet(ch9141_t *handle, char *helloDest, uint8_t helloSize, char const *helloRef);
void CH9141_HelloSet(ch9141_t *handle, char const *newHello);
void CH9141_ModeGet(ch9141_t *handle);
void CH9141_ModeSet(ch9141_t *handle);
