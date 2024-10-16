#include "CH9141_Interface.h"

#define CH9141_MIN_DELAY  2
#define CH9141_RX_TIMEOUT 2000
#define CH9141_TX_TIMEOUT 2000

ch9141_ErrorStatus_t CH9141_UART1_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen)
{
    if (pDataRx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    return HAL_UARTEx_ReceiveToIdle(&huart4, (uint8_t *) pDataRx, size, rxLen, CH9141_RX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
}

ch9141_ErrorStatus_t CH9141_UART1_Transmit(char const *pDataTx, uint16_t size)
{
    if (pDataTx == NULL)
        return CH9141_ERROR_STATUS_ERROR;
    if (size == 0u)
        return CH9141_ERROR_STATUS_ERROR;

    return HAL_UART_Transmit(&huart4, (uint8_t const *) pDataTx, size, CH9141_TX_TIMEOUT) == HAL_OK
               ? CH9141_ERROR_STATUS_SUCCESS
               : CH9141_ERROR_STATUS_ERROR;
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

void CH9141_Delay(uint32_t ms)
{
    extern volatile uint32_t uwTick;
    uint32_t tickStart = uwTick;

    /* Add a freq to guarantee minimum wait */
    if (ms < CH9141_MIN_DELAY)
        ++ms;

    while ((uwTick - tickStart) < ms) {}
}