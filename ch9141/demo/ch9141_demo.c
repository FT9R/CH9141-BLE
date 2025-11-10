#include "ch9141_demo.h"
static ch9141_t *ble1 = &(ch9141_t) {.interface = {.delay = CH9141_Delay,
                                                   .pinMode = CH9141_Pin_Mode1,
                                                   .pinReload = NULL,
                                                   .pinReset = NULL,
                                                   .pinSleep = CH9141_Pin_Sleep1,
                                                   .handle = &huart4,
                                                   .receive = CH9141_UART_Receive,
                                                   .transmit = CH9141_UART_Transmit}};
static char paramSet[50] = {0};
static char bleResponse[50] = {0};
static uint16_t vcc;
static uint16_t adc;

void CH9141_Demo(void)
{
    CH9141_Init(ble1, true);
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
    // if (ble1->error != CH9141_ERR_NONE)
    //     Error_Handler();

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
    if (vcc == UINT16_MAX)
        Error_Handler();

    adc = CH9141_ADCGet(ble1);
    if (adc == UINT16_MAX)
        Error_Handler();

    /* GPIO functions */
    CH9141_GPIOInitSet(ble1, 0xFF);
    if (CH9141_GPIOInitGet(ble1) != 0xFF)
        Error_Handler();

    CH9141_GPIOInitSet(ble1, 0x00);
    if (CH9141_GPIOInitGet(ble1) != 0x00)
        Error_Handler();

    CH9141_GPIOEnSet(ble1, 0xFF);
    if (CH9141_GPIOEnGet(ble1) != 0xFF)
        Error_Handler();

    CH9141_GPIOEnSet(ble1, 0x00);
    if (CH9141_GPIOEnGet(ble1) != 0x00)
        Error_Handler();

    while (CH9141_StatusGet(ble1) != CH9141_BLESTAT_CONNECTED)
        if (ble1->error != CH9141_ERR_NONE)
            Error_Handler();
    strncpy(bleResponse, CH9141_MACRemoteGet(ble1), ble1->responseLen);

    // CH9141_Disconnect(ble1);
}