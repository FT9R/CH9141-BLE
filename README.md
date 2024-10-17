# Description
Platform-independent driver for CH9141 Bluetooth serial port transparent transmission chip with BLE4.2.

# Notes
1. Although CH9141 chip supports broadcast (advertising), master (host) and slave (device) modes, only slave mode is implemented by this driver at this time.
2. Most of driver's functionality intended to be used at MCU's init stage when serial interface is free. After initialization serial interface can be used as usual to write to or read from remote BLE device. But any transmissions have to be stopped if driver is used after MCU's init stage to free serial interface for driver routines.

# Quick start
* Mention the header:
```C
#include "ch9141.h"
```
* Declare the device handle:
```C
ch9141_t ble1;
```
* Declare variable for delay and timeouts: (skip in case of STM32 HAL usage)
```C
volatile uint32_t uwTick;
```
* Increment it within any 1kHz timer routine, e.g.: (skip in case of STM32 HAL usage)
```C
ISR(Timer1kHz_vect) {
  ++uwTick;
}
```
* Initialize serial interface and CH9141 sleep and AT mode related GPIO for each used device, e.g.:
```C
GPIOx_Init();
UARTx_Init();
```
* Provide platform depended implementations for functions below in the `ch9141_Interface.c`:
```C
ch9141_ErrorStatus_t CH9141_UARTx_Receive(char *pDataRx, uint16_t size, uint16_t *rxLen);
ch9141_ErrorStatus_t CH9141_UARTx_Transmit(char const *pDataTx, uint16_t size);
void CH9141_Pin_Sleepx(ch9141_PinState_t newState);
void CH9141_Pin_Modex(ch9141_PinState_t newState);
```
* Link the functions above to a device handle:
```C
CH9141_Link(&ble1, CH9141_UARTx_Receive, CH9141_UARTx_Transmit, CH9141_Pin_Sleepx,
            CH9141_Pin_Modex);
```
* Initialize the device and reset its settings to factory state:
```C
CH9141_Init(&ble1, true);
```

# Examples
* [STM32](platform/STM32F405RGT6/Core/Src/main.c)

# TODO
1. Broadcast (advertising) mode with dependent functionality.
2. Master (host) mode with dependent functionality.
3. GPIO.
4. ADC.
5. Full device information get/set.
6. Hardware reload through GPIO.

[Manufacturer link](https://www.wch-ic.com/products/CH9141.html)