#pragma once

#include "usart.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ch9141_ErrorStatus_e { CH9141_ERROR_STATUS_SUCCESS = 10, CH9141_ERROR_STATUS_ERROR } ch9141_ErrorStatus_t;
typedef enum ch9141_PinState_e { CH9141_PIN_STATE_RESET = 20, CH9141_PIN_STATE_SET } ch9141_PinState_t;

ch9141_ErrorStatus_t CH9141_UART1_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
ch9141_ErrorStatus_t CH9141_UART1_Transmit(char const *pDataTx, uint16_t size);
void CH9141_Pin_Sleep1(ch9141_PinState_t newState);
void CH9141_Pin_Mode1(ch9141_PinState_t newState);
void CH9141_Delay(uint32_t ms);