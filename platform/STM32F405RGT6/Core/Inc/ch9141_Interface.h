#pragma once

#include "usart.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ch9141_ErrorStatus_e { CH9141_ERROR_STATUS_SUCCESS = 10, CH9141_ERROR_STATUS_ERROR } ch9141_ErrorStatus_t;
typedef enum ch9141_PinState_e { CH9141_PIN_STATE_RESET = 20, CH9141_PIN_STATE_SET } ch9141_PinState_t;

/**
 * @brief The one of UARTx receive function templates
 * @param pDataRx Pointer to the buffer where data will be saved
 * @param size Number of bytes to read
 * @param rxLen Number of bytes actually received
 * @return Status of the data transfer request operation
 */
ch9141_ErrorStatus_t CH9141_UART1_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);

/**
 * @brief The one of UARTx transmit function templates
 * @param pDataTx Pointer to the buffer from which data is being sent
 * @param size Number of bytes to send
 * @return Status of the data transfer request operation
 */
ch9141_ErrorStatus_t CH9141_UART1_Transmit(char const *pDataTx, uint16_t size);

/**
 * @brief Sleep pin set/reset function template associated with the 1st device
 * @param newState new sleep pin state
 */
void CH9141_Pin_Sleep1(ch9141_PinState_t newState);

/**
 * @brief AT mode pin set/reset function template associated with the 1st device
 * @param newState new mode pin state
 */
void CH9141_Pin_Mode1(ch9141_PinState_t newState);

/**
 * @brief Provides minimum delay (in milliseconds) based on incremented variable
 * @param ms: specifies the delay time length, in milliseconds
 * @note In the default implementation, timer with 1kHz freq is the source of time base.
 * It is used to generate interrupts at regular time intervals where uwTick is incremented.
 * `volatile uint32_t uwTick` has to be declared with user source file
 */
void CH9141_Delay(uint32_t ms);