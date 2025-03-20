#include "ch9141_demo.h"

static ch9141_t *ble1 = &(ch9141_t) {0};
static char paramSet[50] = {0};
static char bleResponse[50] = {0};
static uint16_t vcc;

/* Private functions prototypes */
/* Platform-specific functions prototypes*/
static ch9141_ErrorStatus_t CH9141_UART4_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
static ch9141_ErrorStatus_t CH9141_UART4_Transmit(char const *pDataTx, uint16_t size);
static void CH9141_Pin_Mode1(ch9141_PinState_t newState);
static void CH9141_Pin_Reset1(ch9141_PinState_t newState);
static void CH9141_Pin_Reload1(ch9141_PinState_t newState);
static void CH9141_Pin_Sleep1(ch9141_PinState_t newState);
static void CH9141_Delay(uint32_t ms);

void CH9141_Demo(void)
{
    CH9141_Link(ble1, CH9141_UART4_Receive, CH9141_UART4_Transmit, CH9141_Delay, CH9141_Pin_Mode1, CH9141_Pin_Reset1,
                CH9141_Pin_Reload1, CH9141_Pin_Sleep1);
    if (ble1->error != CH9141_ERR_NONE)
        Error_Handler();
    CH9141_Init(ble1, false);
    if (ble1->error != CH9141_ERR_NONE)
        Error_Handler();

    /* Actually, MCU's UART has baudrate == 115200, so setting BLE IC's baudrate to 9600
     * will break communications between MCU and BLE IC. And it is OK, because here we will try to reinitialize BLE IC.
     */
    if (ble1->interface.pinReload != NULL)
    {
        strcpy(paramSet, "9600,8,1,0,50");
        CH9141_SerialSet(ble1, 9600, 8, 1, CH9141_SERIAL_PARITY_NONE, 50); // OK
        strncpy(bleResponse, CH9141_SerialGet(ble1), ble1->responseLen); // Won't response here because wrong baudrate
        if (strcmp(bleResponse, paramSet) != 0) // No response received
        {
            /* Reinitialize the device with factory restore option */
            ble1->error = CH9141_ERR_NONE; // Reset existing `CH9141_ERR_SERIAL_RX` error
            CH9141_Init(ble1, true);
            if (ble1->error != CH9141_ERR_NONE)
                Error_Handler();

            /* Set BLE IC's baudrate to match MCU's UART baudrate */
            strcpy(paramSet, "115200,8,1,0,100");
            CH9141_SerialSet(ble1, 115200, 8, 1, CH9141_SERIAL_PARITY_NONE, 100);
            strncpy(bleResponse, CH9141_SerialGet(ble1), ble1->responseLen);
            if (strcmp(bleResponse, paramSet) != 0)
                Error_Handler();
        }
    }

    strcpy(paramSet, "Hello!!!");
    CH9141_HelloSet(ble1, paramSet);
    strncpy(bleResponse, CH9141_HelloGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    strcpy(paramSet, "DeviceName");
    CH9141_DeviceNameSet(ble1, paramSet);
    strncpy(bleResponse, CH9141_DeviceNameGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    strcpy(paramSet, "ChipName");
    CH9141_ChipNameSet(ble1, paramSet);
    strncpy(bleResponse, CH9141_ChipNameGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    ch9141_SleepMode_t sleepMode = CH9141_SLEEPMODE_LOW_ENERGY;
    CH9141_SleepSet(ble1, sleepMode);
    if (CH9141_SleepGet(ble1) != sleepMode)
        Error_Handler();

    ch9141_Power_t power = CH9141_POWER_3DB;
    CH9141_PowerSet(ble1, power);
    if (CH9141_PowerGet(ble1) != power)
        Error_Handler();

    // ch9141_Mode_t mode = CH9141_MODE_HOST;
    // CH9141_ModeSet(ble1, mode);
    // if (CH9141_ModeGet(ble1) != mode)
    //     Error_Handler();

    // CH9141_Connect(ble1, "EF:49:66:A7:14:54", "654321");

    ch9141_Mode_t mode = CH9141_MODE_DEVICE;
    CH9141_ModeSet(ble1, mode);
    if (CH9141_ModeGet(ble1) != mode)
        Error_Handler();

    strcpy(paramSet, "123456");
    CH9141_PasswordSet(ble1, paramSet, CH9141_FUNC_STATE_DISABLE);
    strncpy(bleResponse, CH9141_PasswordGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    strcpy(paramSet, "05:DF:39:4C:99:B4");
    CH9141_MACLocalSet(ble1, paramSet);
    strncpy(bleResponse, CH9141_MACLocalGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    vcc = CH9141_VCCGet(ble1);
    if (vcc < 2500)
        Error_Handler();

    while (CH9141_StatusGet(ble1) != CH9141_BLESTAT_CONNECTED) {}
    strncpy(bleResponse, CH9141_MACRemoteGet(ble1), ble1->responseLen);

    // CH9141_Disconnect(ble1);
}

/**
 * @section Platform-specific functions
 */
static ch9141_ErrorStatus_t CH9141_UART4_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen)
{
    if (pDataRx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    return HAL_UARTEx_ReceiveToIdle(&huart4, (uint8_t *) pDataRx, size, rxLen, CH9141_RX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
}

static ch9141_ErrorStatus_t CH9141_UART4_Transmit(char const *pDataTx, uint16_t size)
{
    if (pDataTx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    return HAL_UART_Transmit(&huart4, (uint8_t const *) pDataTx, size, CH9141_TX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
}

static void CH9141_Delay(uint32_t ms)
{
    extern volatile uint32_t uwTick;
    uint32_t tickStart = uwTick;

    /* Add a freq to guarantee minimum wait */
    if (ms < CH9141_MIN_DELAY)
        ++ms;

    while ((uwTick - tickStart) < ms) {}
}

static void CH9141_Pin_Mode1(ch9141_PinState_t newState)
{
    switch (newState)
    {
    case CH9141_PIN_STATE_SET:
        HAL_GPIO_WritePin(BLE_AT_GPIO_Port, BLE_AT_Pin, GPIO_PIN_SET);
        break;

    case CH9141_PIN_STATE_RESET:
        HAL_GPIO_WritePin(BLE_AT_GPIO_Port, BLE_AT_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

static void CH9141_Pin_Reset1(ch9141_PinState_t newState)
{
    switch (newState)
    {
    case CH9141_PIN_STATE_SET:
        HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET);
        break;

    case CH9141_PIN_STATE_RESET:
        HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

static void CH9141_Pin_Reload1(ch9141_PinState_t newState)
{
    switch (newState)
    {
    case CH9141_PIN_STATE_SET:
        HAL_GPIO_WritePin(BLE_RLD_GPIO_Port, BLE_RLD_Pin, GPIO_PIN_SET);
        break;

    case CH9141_PIN_STATE_RESET:
        HAL_GPIO_WritePin(BLE_RLD_GPIO_Port, BLE_RLD_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

static void CH9141_Pin_Sleep1(ch9141_PinState_t newState)
{
    switch (newState)
    {
    case CH9141_PIN_STATE_SET:
        HAL_GPIO_WritePin(BLE_SLEEP_GPIO_Port, BLE_SLEEP_Pin, GPIO_PIN_SET);
        break;

    case CH9141_PIN_STATE_RESET:
        HAL_GPIO_WritePin(BLE_SLEEP_GPIO_Port, BLE_SLEEP_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}