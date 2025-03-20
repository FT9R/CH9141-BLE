# CH9141 UART driver
Platform-independent driver for CH9141 Bluetooth serial port transparent transmission chip with BLE4.2.

## Notes
* Most of driver's functionality intended to be used at MCU's init stage when serial interface is free. After initialization serial interface can be used as usual to write to or read from remote BLE device. But any transmissions have to be stopped if driver is used after MCU's init stage to free serial interface for driver routines.

## Features
* Platform-independent
* Only 2 files can be used: `ch9141.c` and `ch9141.h` (if platform functions exist in user files)
* Can be used without any additional pins, except for UART tx and rx pins

## Quick start
* Mention the header:
```C
#include "ch9141.h"
```
* Declare the device handle:
```C
ch9141_t ble1;
```
* Initialize serial interface and CH9141 related GPIO for each used device, e.g.:
```C
GPIOx_Init();
UARTx_Init();
```
* Provide platform depended implementations for functions:
```C
ch9141_ErrorStatus_t CH9141_UARTx_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
ch9141_ErrorStatus_t CH9141_UARTx_Transmit(char const *pDataTx, uint16_t size);
void CH9141_Delay(uint32_t ms);

/* Not necessary functions. Can be forced as `NULL` */
void CH9141_Pin_Modex(ch9141_PinState_t newState); 
void CH9141_Pin_Resetx(ch9141_PinState_t newState);
void CH9141_Pin_Reloadx(ch9141_PinState_t newState)
void CH9141_Pin_Sleepx(ch9141_PinState_t newState);
```
* Initialize the interface struct with these functions (force `.fun = NULL` if not used):
```C
ch9141_Interface_t interface = {.delay = CH9141_Delay,
                                .pinMode = CH9141_Pin_Mode1,
                                .pinReload = CH9141_Pin_Reload1,
                                .pinReset = CH9141_Pin_Reset1,
                                .pinSleep = CH9141_Pin_Sleep1,
                                .receive = CH9141_UART4_Receive,
                                .transmit = CH9141_UART4_Transmit};
```
* Link interface to a device handle:
```C
CH9141_Link(&ble1, &interface);
```
* Initialize the device and reset its settings to factory state if needed:
```C
CH9141_Init(&ble1, true);
```

## Examples
* [Common demo](ch9141/demo/ch9141_demo.c)
* [STM32](platform/STM32F405RGT6/Core/Src/main.c)

## TODO
1. Broadcast (advertising) mode with dependent functionality.
2. GPIO.
3. Full device information get/set.

[Manufacturer link](https://www.wch-ic.com/products/CH9141.html)
