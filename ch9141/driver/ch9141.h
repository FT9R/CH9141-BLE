#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Custom data types */
typedef enum ch9141_ErrorStatus_e { CH9141_ERROR_STATUS_SUCCESS = 10, CH9141_ERROR_STATUS_ERROR } ch9141_ErrorStatus_t;

typedef enum ch9141_PinState_e {
    CH9141_PIN_STATE_UNDEFINED,
    CH9141_PIN_STATE_RESET = 20,
    CH9141_PIN_STATE_SET
} ch9141_PinState_t;

typedef enum ch9141_FuncState_e { CH9141_FUNC_STATE_DISABLE = 30, CH9141_FUNC_STATE_ENABLE } ch9141_FuncState_t;

typedef enum ch9141_Error_e {
    CH9141_ERR_NONE,
    CH9141_ERR_ARGUMENT,
    CH9141_ERR_SERIAL_RX,
    CH9141_ERR_SERIAL_TX,
    CH9141_ERR_AT,
    CH9141_ERR_RESPONSE,
    CH9141_ERR_INTERFACE
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
    CH9141_SLEEPMODE_NONE, // Icc = 7.5mA
    CH9141_SLEEPMODE_LOW_ENERGY, // Icc = 300uA, master cannot send serial data
    CH9141_SLEEPMODE_POWER_DOWN, // Icc = 6uA
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
    CH9141_STATE_INIT,
    CH9141_STATE_SERIAL_GET,
    CH9141_STATE_SERIAL_SET,
    CH9141_STATE_CONNECT,
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
    CH9141_STATE_VCC_GET,
    CH9141_STATE_ADC_GET,
    CH9141_STATE_GPIO_GET,
    CH9141_STATE_GPIO_SET,
    CH9141_STATE_GPIO_INIT_GET,
    CH9141_STATE_GPIO_INIT_SET,
    CH9141_STATE_GPIO_EN_GET,
    CH9141_STATE_GPIO_EN_SET
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

/* Platform functions pointers */
/**
 * @brief The one of UARTx receive function templates
 * @param handle optional pointer to the UART handle
 * @param pDataRx pointer to the buffer where data will be saved
 * @param size number of bytes to read
 * @param rxLen pointer to variable to keep the number of bytes actually received
 * @return Status of the data transfer request operation
 */
typedef ch9141_ErrorStatus_t (*ch9141_Receive_fp)(void *handle, char *pDataRx, uint16_t size, uint16_t *rxLen);

/**
 * @brief The one of UARTx transmit function templates
 * @param handle optional pointer to the UART handle
 * @param pDataTx pointer to the buffer from which data is being sent
 * @param size number of bytes to send
 * @return Status of the data transfer request operation
 */
typedef ch9141_ErrorStatus_t (*ch9141_Transmit_fp)(void *handle, char const *pDataTx, uint16_t size);

/**
 * @brief Provides minimum delay
 * @param ms specifies the delay time length, in milliseconds
 */
typedef void (*ch9141_Pin_Delay_fp)(uint32_t ms);

/**
 * @brief Alias for any pin set/reset function
 * @param newState new pin state
 */
typedef void (*ch9141_Pin_fp)(ch9141_PinState_t newState);

/* Device handle */
typedef struct ch9141_s {
    struct {
        ch9141_Receive_fp receive; // Pointer to the platform serial interface receive function
        ch9141_Transmit_fp transmit; // Pointer to the platform serial interface transmit function
        ch9141_Pin_Delay_fp delay; // Pointer to the platform `Delay` function
        ch9141_Pin_fp pinMode; // Pointer to the platform gpio pin `AT mode` set/reset function (CH9141 PIN6)
        ch9141_Pin_fp pinReset; // Pointer to the platform gpio pin `Reset` set/reset function (CH9141 PIN16)
        ch9141_Pin_fp pinReload; // Pointer to the platform gpio pin `Reload` set/reset function (CH9141 PIN23)
        ch9141_Pin_fp pinSleep; // Pointer to the platform gpio pin `Sleep` set/reset function (CH9141 PIN24)
        void *handle; // Optional pointer to the UART handle
    } interface;

    char rxBuf[50];
    char txBuf[50];
    uint16_t rxLen; // Indicates number of data available in reception buffer
    uint8_t responseLen; // Indicates length of response message received by MCU
    ch9141_State_t state; // Indicates current state of BLE IC
    ch9141_Error_t error; // Driver error codes
    ch9141_AT_Error_t errorAT; // Device error codes provided by manufacturer
} ch9141_t;

/**
 * @brief Initializes/Reinitializes the target device
 * @param handle pointer to the target device handle
 * @note In case of communication break between MCU and BLE IC, reinitialization is only possible if `pinReset` and
 * `pinReload` interface functions are provided. It is because UART parameters between devices do not match.
 */
void CH9141_Init(ch9141_t *handle, bool factoryRestore);

/**
 * @brief Gets serial interface parameters
 * @param handle pointer to the target device handle
 * @return Serial interface parameters as a null-terminated string
 */
char *CH9141_SerialGet(ch9141_t *handle);

/**
 * @brief Sets serial interface parameters
 * @param handle pointer to the target device handle
 * @param baudRate target device baudrate (up 1Mbit/s)
 * @param dataBit target device dataBit (8 or 9)
 * @param stopBit target device stopBit (1 or 2)
 * @param parity target device parity (none, odd or even)
 * @param timeout [ms]. Target device timeout in transparent transmission mode
 * @note Use this function with care, because it may break communication between MCU and BLE IC.
 * @note In case of communication break reinitialize BLE IC with `CH9141_Init` or configure new UART parameters on
 * the MCU side
 */
void CH9141_SerialSet(ch9141_t *handle, uint32_t baudRate, uint8_t dataBit, uint8_t stopBit,
                      ch9141_SerialParity_t parity, uint16_t timeout);

/**
 * @brief Connects to the slave with provided mac address and password
 * @param handle pointer to the target device handle
 * @param mac BLE slave MAC address (format xx:xx:xx:xx:xx:xx) as a null-terminated string
 * @param password BLE slave password (6 digit) as a null-terminated string. Pass `NULL` if no password is required
 * @note Check `handle.error == CH9141_ERR_NONE` after calling this function to ensure successful connection
 */
void CH9141_Connect(ch9141_t *handle, char const *mac, char const *password);

/**
 * @brief Disconnects the current connection
 * @param handle pointer to the target device handle
 */
void CH9141_Disconnect(ch9141_t *handle);

/**
 * @brief Gets welcome message from device
 * @param handle pointer to the target device handle
 * @return Welcome message as a null-terminated string or `NULL` if no response received
 */
char *CH9141_HelloGet(ch9141_t *handle);

/**
 * @brief Sets device welcome message
 * @param handle pointer to the target device handle
 * @param helloSet welcome message as a null-terminated string
 */
void CH9141_HelloSet(ch9141_t *handle, char const *helloSet);

/**
 * @brief Gets device name
 * @param handle pointer to the target device handle
 * @return Device name as a null-terminated string or `NULL` if no response received
 */
char *CH9141_DeviceNameGet(ch9141_t *handle);

/**
 * @brief Sets device name
 * @param handle pointer to the target device handle
 * @param nameSet device name as a null-terminated string
 */
void CH9141_DeviceNameSet(ch9141_t *handle, char const *nameSet);

/**
 * @brief Gets chip name
 * @param handle pointer to the target device handle
 * @return chip name as a null-terminated string or `NULL` if no response received
 */
char *CH9141_ChipNameGet(ch9141_t *handle);

/**
 * @brief Sets chip name
 * @param handle pointer to the target device handle
 * @param nameSet chip name as a null-terminated string
 */
void CH9141_ChipNameSet(ch9141_t *handle, char const *nameSet);

/**
 * @brief Used to put the device into low energy mode
 * @param handle pointer to the target device handle
 * @param funcState enable or disable low energy mode
 * @note Use only if `interface.pinSleep` is provided
 */
void CH9141_SleepSwitch(ch9141_t *handle, ch9141_FuncState_t funcState);

/**
 * @brief Gets device sleep mode
 * @param handle pointer to the target device handle
 * @return Device sleep mode
 */
ch9141_SleepMode_t CH9141_SleepGet(ch9141_t *handle);

/**
 * @brief Sets device sleep mode
 * @param handle pointer to the target device handle
 * @param sleepMode device sleep mode
 */
void CH9141_SleepSet(ch9141_t *handle, ch9141_SleepMode_t sleepMode);

/**
 * @brief Gets device BLE transmission power
 * @param handle pointer to the target device handle
 * @return Device BLE transmission power
 */
ch9141_Power_t CH9141_PowerGet(ch9141_t *handle);

/**
 * @brief Sets device BLE transmission power
 * @param handle pointer to the target device handle
 * @param power device BLE transmission power
 */
void CH9141_PowerSet(ch9141_t *handle, ch9141_Power_t power);

/**
 * @brief Gets device BLE working mode
 * @param handle pointer to the target device handle
 * @return Device BLE working mode
 */
ch9141_Mode_t CH9141_ModeGet(ch9141_t *handle);

/**
 * @brief Sets device BLE working mode
 * @param handle pointer to the target device handle
 * @param mode device BLE working mode
 */
void CH9141_ModeSet(ch9141_t *handle, ch9141_Mode_t mode);

/**
 * @brief Gets device slave password
 * @param handle pointer to the target device handle
 * @return Device slave password as a null-terminated string or `NULL` if no response received
 */
char *CH9141_PasswordGet(ch9141_t *handle);

/**
 * @brief Sets device slave password
 * @param handle pointer to the target device handle
 * @param passwordSet device slave password (6 numeric characters) as a null-terminated string
 * @param funcState enable or disable password check upon connection process
 */
void CH9141_PasswordSet(ch9141_t *handle, char const *passwordSet, ch9141_FuncState_t funcState);

/**
 * @brief Gets device BLE status
 * @param handle pointer to the target device handle
 * @return Device BLE status
 */
ch9141_BLEStatus_t CH9141_StatusGet(ch9141_t *handle);

/**
 * @brief Gets device BLE MAC address
 * @param handle pointer to the target device handle
 * @return Device BLE MAC address as a null-terminated string or `NULL` if no response received
 */
char *CH9141_MACLocalGet(ch9141_t *handle);

/**
 * @brief Sets device BLE MAC address
 * @param handle pointer to the target device handle
 * @param mac device new BLE MAC address as a null-terminated string
 */
void CH9141_MACLocalSet(ch9141_t *handle, char const *mac);

/**
 * @brief Gets connected device BLE MAC address
 * @param handle pointer to the target device handle
 * @return Connected device BLE MAC address as a null-terminated string or `NULL` if no response received
 */
char *CH9141_MACRemoteGet(ch9141_t *handle);

/**
 * @brief Gets supply voltage of the chip
 * @param handle pointer to the target device handle
 * @return Supply voltage of the chip [mV] or `UINT16_MAX` if no response received
 */
uint16_t CH9141_VCCGet(ch9141_t *handle);

/**
 * @brief Gets ADC value(0-4095) of the chip ADC pin (CH9141 PIN7)
 * @param handle pointer to the target device handle
 * @return ADC value of the chip ADC pin or `UINT16_MAX` if no response received
 */
uint16_t CH9141_ADCGet(ch9141_t *handle);

/**
 * @brief Gets GPIO pin level
 * @param handle pointer to the target device handle
 * @param pin pin number (1, 3, 4, 5, 6, 7)
 * @note If pin is in output mode, this command will reconfigure it to input mode
 * @return Current GPIO pin level
 */
ch9141_PinState_t CH9141_GPIOGet(ch9141_t *handle, uint8_t pin);

/**
 * @brief Sets GPIO pin level
 * @param handle pointer to the target device handle
 * @param pin pin number (0, 2, 4, 5, 6, 7)
 * @param pinState new GPIO pin level
 * @note If pin is in input mode, this command will reconfigure it to output mode
 */
void CH9141_GPIOSet(ch9141_t *handle, uint8_t pin, ch9141_PinState_t pinState);

/**
 * @brief Gets the default value of GPIO output in the configuration
 * @param handle pointer to the target device handle
 * @return Current default value of GPIO output (byte bitmask) or `UINT16_MAX` if no response received
 */
uint16_t CH9141_GPIOInitGet(ch9141_t *handle);

/**
 * @brief Sets the default value of GPIO output in the configuration
 * @param handle pointer to the target device handle
 * @param configIO new default value of GPIO output (byte bitmask)
 */
void CH9141_GPIOInitSet(ch9141_t *handle, uint8_t configIO);

/**
 * @brief Gets the GPIO enable config byte
 * @param handle pointer to the target device handle
 * @return Current GPIO enable config byte or `UINT16_MAX` if no response received
 */
uint16_t CH9141_GPIOEnGet(ch9141_t *handle);

/**
 * @brief Sets the GPIO enable config byte
 * @param handle pointer to the target device handle
 * @param configIO new GPIO enable config byte
 */
void CH9141_GPIOEnSet(ch9141_t *handle, uint8_t configIO);