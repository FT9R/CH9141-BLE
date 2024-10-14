#pragma once

#include "usart.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ch9141_ErrorStatus_e { CH9141_STAT_SUCCESS = 10, CH9141_STAT_ERROR } ch9141_ErrorStatus_t;
typedef enum ch9141_PinState_e { CH9141_PIN_RESET = 20, CH9141_PIN_SET } ch9141_PinState_t;
typedef enum ch9141_FuncState_e { CH9141_FUNC_DISABLE = 30, CH9141_FUNC_ENABLE } ch9141_FuncState_t;

ch9141_ErrorStatus_t CH9141_UART1_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
ch9141_ErrorStatus_t CH9141_UART1_Transmit(char const *pDataTx, uint16_t size);
void CH9141_Pin_Sleep1(ch9141_PinState_t newState);
void CH9141_Pin_Mode1(ch9141_PinState_t newState);
void CH9141_Delay(uint32_t ms);