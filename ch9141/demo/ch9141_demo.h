#pragma once

#include "../driver/ch9141.h"
#include "usart.h"

#define CH9141_MIN_DELAY  2
#define CH9141_RX_TIMEOUT 2000
#define CH9141_TX_TIMEOUT 2000

/**
 * @brief CH9141 driver basic functionality demonstration
 * @param none
 */
void CH9141_Demo(void);