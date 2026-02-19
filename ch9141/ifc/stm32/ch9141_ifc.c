#include "ch9141_ifc.h"

#define CH9141_RX_TIMEOUT 200
#define CH9141_TX_TIMEOUT 2000

ch9141_ErrorStatus_t CH9141_UART_Receive(void *handle, char *pDataRx, uint16_t size, uint16_t *rxLen)
{
    if (handle == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (pDataRx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    /* Abort ongoing reception if necessary */
    UART_HandleTypeDef huart = *(UART_HandleTypeDef *) handle;
    if (huart.RxState != HAL_UART_STATE_READY)
        HAL_UART_AbortReceive(&huart);

    return HAL_UARTEx_ReceiveToIdle(&huart, (uint8_t *) pDataRx, size, rxLen, CH9141_RX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
}

ch9141_ErrorStatus_t CH9141_UART_Transmit(void *handle, char const *pDataTx, uint16_t size)
{
    if (handle == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (pDataTx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    /* Abort ongoing transmission if necessary */
    UART_HandleTypeDef huart = *(UART_HandleTypeDef *) handle;
    if (huart.gState != HAL_UART_STATE_READY)
        HAL_UART_AbortTransmit(&huart);

    return HAL_UART_Transmit(&huart, (uint8_t const *) pDataTx, size, CH9141_TX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
}

void CH9141_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

void CH9141_Pin_Mode1(ch9141_PinState_t newState)
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

void CH9141_Pin_Reset1(ch9141_PinState_t newState)
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

void CH9141_Pin_Reload1(ch9141_PinState_t newState)
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

void CH9141_Pin_Sleep1(ch9141_PinState_t newState)
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

ErrorStatus CH9141_SetUp(ch9141_t *ble)
{
    const char *deviceName = "TAG044";
    const char *chipName = "RTS044";
    const uint8_t attempts = 3;
    char bleResponse[50] = {0};
    ErrorStatus cmpResult = SUCCESS;

    if (ble == NULL)
        return ERROR;

    /* Initialize the ble interface */
    memset(ble, 0, sizeof(ch9141_t));
    ble->interface.handle = &huart4;
    ble->interface.receive = CH9141_UART_Receive;
    ble->interface.transmit = CH9141_UART_Transmit;
    ble->interface.delay = CH9141_Delay;
    ble->interface.pinMode = CH9141_Pin_Mode1;
    ble->interface.pinSleep = CH9141_Pin_Sleep1;
    ble->interface.pinReset = CH9141_Pin_Reset1;
    ble->interface.pinReload = CH9141_Pin_Reload1;
    CH9141_Init(ble, false);
    if (ble->error != CH9141_ERR_NONE)
        return ERROR;

    /* Check device name */
    strncpy(bleResponse, CH9141_DeviceNameGet(ble), ble->responseLen);
    if (strcmp(bleResponse, deviceName) != 0)
    {
        for (size_t i = 0; i < attempts; i++)
        {
            cmpResult = SUCCESS;

            /* New or unknown device, need to factory restore and set up */
            CH9141_Init(ble, true); // Factory restore

            /* Device name */
            CH9141_DeviceNameSet(ble, deviceName);
            strncpy(bleResponse, CH9141_DeviceNameGet(ble), ble->responseLen);
            if (strcmp(bleResponse, deviceName) != 0)
            {
                cmpResult = ERROR;
                continue;
            }

            /* Chip name */
            CH9141_ChipNameSet(ble, chipName);
            strncpy(bleResponse, CH9141_ChipNameGet(ble), ble->responseLen);
            if (strcmp(bleResponse, chipName) != 0)
            {
                cmpResult = ERROR;
                continue;
            }

            CH9141_SleepSet(ble, CH9141_SLEEPMODE_LOW_ENERGY);
            CH9141_PowerSet(ble, CH9141_POWER_3DB);
            CH9141_ModeSet(ble, CH9141_MODE_DEVICE);
            CH9141_GPIOEnSet(ble, 0xF0);
            // CH9141_GPIOInitSet(ble, 1 << 6 | 1 << 7);
            // CH9141_PasswordSet(ble, "093728", CH9141_FUNC_STATE_ENABLE);

            if (ble->error == CH9141_ERR_NONE)
                break;
        }
    }

    /* Force gpio5-7 to input mode */
    CH9141_GPIOGet(ble, 5);
    CH9141_GPIOGet(ble, 6);
    CH9141_GPIOGet(ble, 7);

    return (cmpResult == ERROR || ble->error != CH9141_ERR_NONE) ? ERROR : SUCCESS;
}