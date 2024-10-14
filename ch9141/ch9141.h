#pragma once

#include "ch9141_Interface.h"

typedef ch9141_ErrorStatus_t (*ch9141_Receive_fp)(char *pDataRx, uint16_t size, uint16_t *rxLen);
typedef ch9141_ErrorStatus_t (*ch9141_Transmit_fp)(char const *pDataTx, uint16_t size);
typedef void (*ch9141_Pin_Sleep_fp)(ch9141_PinState_t newState);
typedef void (*ch9141_Pin_Mode_fp)(ch9141_PinState_t newState);
typedef void (*ch9141_Delay_fp)(uint32_t ms);

typedef enum ch9141_Error_e {
    CH9141_ERR_NONE,
    CH9141_ERR_ARGUMENT,
    CH9141_ERR_MEMORY,
    CH9141_ERR_SERIAL_RX,
    CH9141_ERR_SERIAL_TX,
    CH9141_ERR_AT,
    CH9141_ERR_RESPONSE
} ch9141_Error_t;

typedef enum ch9141_AT_Error_e {
    CH9141_AT_ERR_NONE, // Custom - not defined by manufacturer
    CH9141_AT_ERR_CACHE, // The current chip does not have a cache to respond, you can try again later
    CH9141_AT_ERR_PARAM, // Some parameters of the AT command that is sent do not meet the specifications
    CH9141_AT_ERR_CMD_SUP, // Commands are not supported in the current mode
    CH9141_AT_ERR_CMD_EXEC // The command cannot be executed temporarily
} ch9141_AT_Error_t;

typedef enum ch9141_Mode_e {
    CH9141_MODE_BROADCAST,
    CH9141_MODE_HOST,
    CH9141_MODE_DEVICE,
    CH9141_MODE_UNDEFINED
} ch9141_Mode_t;

typedef enum ch9141_SleepMode_e {
    CH9141_SLEEPMODE_NONE,
    CH9141_SLEEPMODE_LOW_ENERGY,
    CH9141_SLEEPMODE_POWER_DOWN,
    CH9141_SLEEPMODE_UNDEFINED
} ch9141_SleepMode_t;

typedef enum ch9141_BLEStatAdv_e {
    CH9141_BLESTA_ADV_NOINIT,
    CH9141_BLESTA_ADV_INIT,
    CH9141_BLESTA_ADV,
} ch9141_BLEStatAdv_t;

typedef enum ch9141_BLEStatHost_e {
    CH9141_BLESTA_HOST_NOINIT,
    CH9141_BLESTA_HOST_SCAN,
    CH9141_BLESTA_HOST_CONNECTING,
    CH9141_BLESTA_HOST_CONNECTED,
    CH9141_BLESTA_HOST_DISCONNECTING
} ch9141_BLEStatHost_t;

typedef enum ch9141_BLEStatSlave_e {
    CH9141_BLESTA_SLAVE_NOINIT,
    CH9141_BLESTA_SLAVE_INIT,
    CH9141_BLESTA_SLAVE_ADV,
    CH9141_BLESTA_SLAVE_ADVRDY,
    CH9141_BLESTA_SLAVE_CONTIMEOUT,
    CH9141_BLESTA_SLAVE_CONNECTED
} ch9141_BLEStatSlave_t;

typedef enum ch9141_State_e {
    CH9141_STATE_UNDEFINED,
    CH9141_STATE_IDLE,
    CH9141_STATE_LINK,
    CH9141_STATE_INIT,
    CH9141_STATE_RESET,
    CH9141_STATE_SLEEP_SWITCH,
    CH9141_STATE_SLEEP_GET,
    CH9141_STATE_SLEEP_SET,
    CH9141_STATE_PASSWORD_GET,
    CH9141_STATE_PASSWORD_SET,
    CH9141_STATE_HELLO_GET,
    CH9141_STATE_HELLO_SET,
    CH9141_STATE_MODE_GET,
    CH9141_STATE_MODE_SET,
    CH9141_STATE_STATUS_GET,
    CH9141_STATE_MAC_LOCAL_GET,
    CH9141_STATE_MAC_LOCAL_SET,
    CH9141_STATE_MAC_REMOTE_GET,
    CH9141_STATE_VCC_GET,
    CH9141_STATE_POWER_GET,
    CH9141_STATE_POWER_SET
} ch9141_State_t;

typedef enum ch9141_Power_e {
    CH9141_POWER_0DB,
    CH9141_POWER_1DB,
    CH9141_POWER_2DB,
    CH9141_POWER_3DB,
    CH9141_POWER_MIN3DB,
    CH9141_POWER_MIN8DB,
    CH9141_POWER_MIN14DB,
    CH9141_POWER_MIN20DB,
    CH9141_POWER_UNDEFINED
} ch9141_Power_t;

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
    char password[6 + 1];
    char macLocal[18];
    char macRemote[18];
    bool isConnected;
    uint16_t rxLen; // Indicates number of data available in reception buffer
    uint16_t vcc; // Supply voltage [mV]
    ch9141_Mode_t bleMode;
    union
    {
        ch9141_BLEStatAdv_t statusAdv;
        ch9141_BLEStatHost_t statusHost;
        ch9141_BLEStatSlave_t statusSlave;
    } bleStatus;
    ch9141_SleepMode_t sleepMode;
    ch9141_State_t state;
    ch9141_Power_t power;
    ch9141_Error_t error; // Driver error codes
    ch9141_AT_Error_t errorAT; // Device error codes, relevant if handle.error = CH9141_ERR_AT
} ch9141_t;

void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode);
void CH9141_Init(ch9141_t *handle);
void CH9141_Reset(ch9141_t *handle);
void CH9141_SleepSwitch(ch9141_t *handle, ch9141_FuncState_t newState);
ch9141_SleepMode_t CH9141_SleepGet(ch9141_t *handle);
void CH9141_SleepSet(ch9141_t *handle);
ch9141_Power_t CH9141_PowerGet(ch9141_t *handle);
void CH9141_PowerSet(ch9141_t *handle);
void CH9141_PasswordGet(ch9141_t *handle);
void CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState);
void CH9141_HelloGet(ch9141_t *handle, char *helloDest, uint8_t helloSize, char const *helloRef);
void CH9141_HelloSet(ch9141_t *handle, char const *newHello);
ch9141_Mode_t CH9141_ModeGet(ch9141_t *handle);
void CH9141_ModeSet(ch9141_t *handle);
bool CH9141_StatusGet(ch9141_t *handle);
void CH9141_MACLocalGet(ch9141_t *handle);
void CH9141_MACLocalSet(ch9141_t *handle);
void CH9141_MACRemoteGet(ch9141_t *handle);
uint16_t CH9141_VCCGet(ch9141_t *handle);