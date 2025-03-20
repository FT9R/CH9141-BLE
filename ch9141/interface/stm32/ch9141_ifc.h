#pragma once

#include "../../driver/ch9141.h"
#include "usart.h"

ch9141_ErrorStatus_t CH9141_UART4_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
ch9141_ErrorStatus_t CH9141_UART4_Transmit(char const *pDataTx, uint16_t size);
void CH9141_Pin_Mode1(ch9141_PinState_t newState);
void CH9141_Pin_Reset1(ch9141_PinState_t newState);
void CH9141_Pin_Reload1(ch9141_PinState_t newState);
void CH9141_Pin_Sleep1(ch9141_PinState_t newState);
void CH9141_Delay(uint32_t ms);