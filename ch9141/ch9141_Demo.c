#include "ch9141_Demo.h"

static ch9141_t *ble1 = &(ch9141_t){0};
static char paramSet[50] = {0};
static char bleResponse[50] = {0};
static uint16_t vcc;

void CH9141_Demo(void)
{
    CH9141_Link(ble1, CH9141_UART1_Receive, CH9141_UART1_Transmit, CH9141_Pin_Sleep1, CH9141_Pin_Mode1);
    CH9141_Init(ble1, true);

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

    CH9141_SleepSet(ble1, CH9141_SLEEPMODE_LOW_ENERGY);
    if (CH9141_SleepGet(ble1) != CH9141_SLEEPMODE_LOW_ENERGY)
        Error_Handler();

    CH9141_PowerSet(ble1, CH9141_POWER_3DB);
    if (CH9141_PowerGet(ble1) != CH9141_POWER_3DB)
        Error_Handler();

    CH9141_ModeSet(ble1, CH9141_MODE_DEVICE);
    if (CH9141_ModeGet(ble1) != CH9141_MODE_DEVICE)
        Error_Handler();

    strcpy(paramSet, "123456");
    CH9141_PasswordSet(ble1, paramSet, CH9141_FUNC_STATE_ENABLE);
    strncpy(bleResponse, CH9141_PasswordGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    strcpy(paramSet, "05:DF:39:4C:99:B4");
    CH9141_MACLocalSet(ble1, paramSet);
    strncpy(bleResponse, CH9141_MACLocalGet(ble1), ble1->responseLen);
    if (strcmp(bleResponse, paramSet) != 0)
        Error_Handler();

    vcc = CH9141_VCCGet(ble1);
    if (vcc == 0)
        Error_Handler();

    while (CH9141_StatusGet(ble1) != CH9141_BLESTAT_CONNECTED) {}
    strncpy(bleResponse, CH9141_MACRemoteGet(ble1), ble1->responseLen);

    CH9141_Disconnect(ble1);
}