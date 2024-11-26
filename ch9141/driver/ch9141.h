#pragma once

#include "../ifc/stm32/ch9141_ifc.h"

typedef ch9141_ErrorStatus_t (*ch9141_Receive_fp)(char *pDataRx, uint16_t size, uint16_t *rxLen);
typedef ch9141_ErrorStatus_t (*ch9141_Transmit_fp)(char const *pDataTx, uint16_t size);
typedef void (*ch9141_Pin_Sleep_fp)(ch9141_PinState_t newState);
typedef void (*ch9141_Pin_Mode_fp)(ch9141_PinState_t newState);

typedef enum ch9141_FuncState_e { CH9141_FUNC_STATE_DISABLE = 30, CH9141_FUNC_STATE_ENABLE } ch9141_FuncState_t;

typedef enum ch9141_Error_e {
    CH9141_ERR_NONE,
    CH9141_ERR_ARGUMENT,
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

typedef enum ch9141_SerialParity_e {
    CH9141_SERIAL_PARITY_NONE,
    CH9141_SERIAL_PARITY_ODD,
    CH9141_SERIAL_PARITY_EVEN
} ch9141_SerialParity_t;

typedef enum ch9141_State_e {
    CH9141_STATE_UNDEFINED,
    CH9141_STATE_IDLE,
    CH9141_STATE_LINK,
    CH9141_STATE_INIT,
    CH9141_STATE_SERIAL_GET,
    CH9141_STATE_SERIAL_SET,
    CH9141_STATE_DISCONNECT,
    CH9141_STATE_HELLO_GET,
    CH9141_STATE_HELLO_SET,
    CH9141_STATE_CHIPNAME_GET,
    CH9141_STATE_CHIPNAME_SET,
    CH9141_STATE_DEVICENAME_GET,
    CH9141_STATE_DEVICENAME_SET,
    CH9141_STATE_SLEEP_SWITCH,
    CH9141_STATE_SLEEP_GET,
    CH9141_STATE_SLEEP_SET,
    CH9141_STATE_POWER_GET,
    CH9141_STATE_POWER_SET,
    CH9141_STATE_PASSWORD_GET,
    CH9141_STATE_PASSWORD_SET,
    CH9141_STATE_MODE_GET,
    CH9141_STATE_MODE_SET,
    CH9141_STATE_STATUS_GET,
    CH9141_STATE_MAC_LOCAL_GET,
    CH9141_STATE_MAC_LOCAL_SET,
    CH9141_STATE_MAC_REMOTE_GET,
    CH9141_STATE_VCC_GET
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

typedef enum ch9141_BLEStatus_e {
    CH9141_BLESTAT_NOINIT,
    CH9141_BLESTAT_INIT_SCAN,
    CH9141_BLESTAT_ADV_CONNECTING,
    CH9141_BLESTAT_CONNECTED_ADVRDY,
    CH9141_BLESTAT_DISCONNECTING_CONNTIMEOUT,
    CH9141_BLESTAT_CONNECTED,
    CH9141_BLESTAT_UNDEFINED, // Custom - not defined by manufacturer
    CH9141_BLESTAT_ERROR
} ch9141_BLEStatus_t;

typedef struct ch9141_s
{
    struct
    {
        ch9141_Receive_fp receive;
        ch9141_Transmit_fp transmit;
        ch9141_Pin_Sleep_fp pinSleep;
        ch9141_Pin_Mode_fp pinMode;
    } interface;
    char rxBuf[100];
    char txBuf[100];
    uint16_t rxLen; // Indicates number of data available in reception buffer
    uint8_t responseLen;
    ch9141_State_t state;
    ch9141_Error_t error; // Driver error codes
    ch9141_AT_Error_t errorAT; // Device error codes, relevant if handle.error = CH9141_ERR_AT
} ch9141_t;

/**
 * @brief Links user defined platform functions to the target device
 * @param handle Pointer to the target device handle
 * @param fpReceive Pointer to the platform serial interface receive function
 * @param fpTransmit Pointer to the platform serial interface transmit function
 * @param fpPinSleep Pointer to the platform gpio pin sleep set/reset function (CH9141 PIN24)
 * @param fpPinMode Pointer to the platform gpio pin AT mode set/reset function (CH9141 PIN6)
 */
void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode);

/**
 * @brief Initializes the target device
 * @param handle Pointer to the target device handle
 * @param factoryRestore Restore factory settings or not
 */
void CH9141_Init(ch9141_t *handle, bool factoryRestore);

/**
 * @brief Gets serial interface parameters
 * @param handle Pointer to the target device handle
 * @return Serial interface parameters as a null-terminated string
 */
char *CH9141_SerialGet(ch9141_t *handle);

/**
 * @brief Sets serial interface parameters
 * @param handle Pointer to the target device handle
 * @param baudRate Target device baudrate (up 1Mbit/s)
 * @param dataBit Target device dataBit (8 or 9)
 * @param stopBit Target device stopBit (1 or 2)
 * @param parity Target device parity (none, odd or even)
 * @param timeout [ms]. Target device timeout in transparent transmission mode
 */
void CH9141_SerialSet(ch9141_t *handle, uint32_t baudRate, uint8_t dataBit, uint8_t stopBit,
                      ch9141_SerialParity_t parity, uint16_t timeout);

/**
 * @brief Disconnects the current connection
 * @param handle Pointer to the target device handle
 */
void CH9141_Disconnect(ch9141_t *handle);

/**
 * @brief Gets welcome message from device
 * @param handle Pointer to the target device handle
 * @return Welcome message as a null-terminated string
 */
char *CH9141_HelloGet(ch9141_t *handle);

/**
 * @brief Sets device welcome message
 * @param handle Pointer to the target device handle
 * @param helloSet Welcome message as a null-terminated string
 */
void CH9141_HelloSet(ch9141_t *handle, char const *helloSet);

/**
 * @brief Gets device name
 * @param handle Pointer to the target device handle
 * @return Device name as a null-terminated string
 */
char *CH9141_DeviceNameGet(ch9141_t *handle);

/**
 * @brief Sets device name
 * @param handle Pointer to the target device handle
 * @param nameSet Device name as a null-terminated string
 */
void CH9141_DeviceNameSet(ch9141_t *handle, char const *nameSet);

/**
 * @brief Gets chip name
 * @param handle Pointer to the target device handle
 * @return Chip name as a null-terminated string
 */
char *CH9141_ChipNameGet(ch9141_t *handle);

/**
 * @brief Sets chip name
 * @param handle Pointer to the target device handle
 * @param nameSet Chip name as a null-terminated string
 */
void CH9141_ChipNameSet(ch9141_t *handle, char const *nameSet);

/**
 * @brief Changes device low energy mode
 * @param handle Pointer to the target device handle
 * @param funcState Enable or disable low energy mode
 */
void CH9141_SleepSwitch(ch9141_t *handle, ch9141_FuncState_t funcState);

/**
 * @brief Gets device sleep mode
 * @param handle Pointer to the target device handle
 * @return Device sleep mode
 */
ch9141_SleepMode_t CH9141_SleepGet(ch9141_t *handle);

/**
 * @brief Sets device sleep mode
 * @param handle Pointer to the target device handle
 * @param sleepMode Device sleep mode
 */
void CH9141_SleepSet(ch9141_t *handle, ch9141_SleepMode_t sleepMode);

/**
 * @brief Gets device BLE transmission power
 * @param handle Pointer to the target device handle
 * @return Device BLE transmission power
 */
ch9141_Power_t CH9141_PowerGet(ch9141_t *handle);

/**
 * @brief Sets device BLE transmission power
 * @param handle Pointer to the target device handle
 * @param power Device BLE transmission power
 */
void CH9141_PowerSet(ch9141_t *handle, ch9141_Power_t power);

/**
 * @brief Gets device BLE working mode
 * @param handle Pointer to the target device handle
 * @return Device BLE working mode
 */
ch9141_Mode_t CH9141_ModeGet(ch9141_t *handle);

/**
 * @brief Sets device BLE working mode
 * @param handle Pointer to the target device handle
 * @param mode Device BLE working mode
 */
void CH9141_ModeSet(ch9141_t *handle, ch9141_Mode_t mode);

/**
 * @brief Gets device slave password
 * @param handle Pointer to the target device handle
 * @return Device slave password as a null-terminated string
 */
char *CH9141_PasswordGet(ch9141_t *handle);

/**
 * @brief Sets device slave password
 * @param handle Pointer to the target device handle
 * @param passwordSet Device slave password as a null-terminated string
 * @param funcState Enable or disable password check upon connection process
 */
void CH9141_PasswordSet(ch9141_t *handle, char const *passwordSet, ch9141_FuncState_t funcState);

/**
 * @brief Gets device BLE status
 * @param handle Pointer to the target device handle
 * @return Device BLE status
 */
ch9141_BLEStatus_t CH9141_StatusGet(ch9141_t *handle);

/**
 * @brief Gets device BLE MAC address
 * @param handle Pointer to the target device handle
 * @return Device BLE MAC address as a null-terminated string
 */
char *CH9141_MACLocalGet(ch9141_t *handle);

/**
 * @brief Sets device BLE MAC address
 * @param handle Pointer to the target device handle
 * @param mac Device BLE MAC address as a null-terminated string
 */
void CH9141_MACLocalSet(ch9141_t *handle, char const *mac);

/**
 * @brief Gets connected device BLE MAC address
 * @param handle Pointer to the target device handle
 * @return Connected device BLE MAC address as a null-terminated string
 */
char *CH9141_MACRemoteGet(ch9141_t *handle);

/**
 * @brief Gets supply voltage of the chip
 * @param handle Pointer to the target device handle
 * @return [mV]. Supply voltage of the chip
 */
uint16_t CH9141_VCCGet(ch9141_t *handle);