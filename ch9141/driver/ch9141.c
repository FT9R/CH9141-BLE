#include "ch9141.h"

typedef enum { MODE_AT, MODE_TRANSPARENT } mode_t;

static void ModeSwitch(ch9141_t *handle, mode_t mode);
static void CMD_Get(ch9141_t *handle, char const *cmd);
static void CMD_Set(ch9141_t *handle, char const *cmd);
static void Reset(ch9141_t *handle);
static void Reload(ch9141_t *handle);

void CH9141_Link(ch9141_t *handle, ch9141_Interface_t *interface)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_LINK;

    /* Check platform functions */
    if (interface->receive == NULL || interface->transmit == NULL || interface->delay == NULL)
    {
        handle->error = CH9141_ERR_INTERFACE;
        return;
    }
    if ((interface->pinReload != NULL) && (interface->pinReset == NULL))
    {
        handle->error = CH9141_ERR_INTERFACE;
        return;
    }

    /* Clear all interface fields */
    memset(&handle->interface, 0, sizeof(handle->interface));

    /* Link platform functions to the device */
    handle->interface = interface;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_Init(ch9141_t *handle, bool factoryRestore)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_INIT;

    /* Exit from sleep mode */
    if (handle->interface->pinSleep != NULL)
        handle->interface->pinSleep(CH9141_PIN_STATE_SET);
    handle->interface->delay(100);

    /* Restore factory settings if requested */
    if (factoryRestore)
        Reload(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_SerialGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_SERIAL_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+UART?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_SerialSet(ch9141_t *handle, uint32_t baudRate, uint8_t dataBit, uint8_t stopBit,
                      ch9141_SerialParity_t parity, uint16_t timeout)
{
    char cmd[30] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_SERIAL_SET;

    if (baudRate == 0 || baudRate > 1e6)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (dataBit != 8 && dataBit != 9)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (stopBit != 1 && stopBit != 2)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (timeout == 0)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+UART=%u,%u,%u,%u,%u", baudRate, dataBit, stopBit, parity, timeout);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_Connect(ch9141_t *handle, char const *mac, char const *password)
{
    char cmd[40] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_CONNECT;

    /* Check arguments */
    if (mac == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(mac) != 17)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (password != NULL)
    {
        if (strlen(password) != 6)
        {
            handle->error = CH9141_ERR_ARGUMENT;
            return;
        }
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+CONN=%s,%s", mac, password);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_Disconnect(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_DISCONNECT;

    /* Set the parameter */
    CMD_Set(handle, "AT+DISCONN");
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_HelloGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_HELLO_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+HELLO?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_HelloSet(ch9141_t *handle, char const *helloSet)
{
    char cmd[50] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_HELLO_SET;

    if (helloSet == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(helloSet) >= 30)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+HELLO=%s", helloSet);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_DeviceNameGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_DEVICENAME_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+PNAME?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_DeviceNameSet(ch9141_t *handle, char const *nameSet)
{
    char cmd[30] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_DEVICENAME_SET;

    if (nameSet == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(nameSet) > 18)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+PNAME=%s", nameSet);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_ChipNameGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_CHIPNAME_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+NAME?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_ChipNameSet(ch9141_t *handle, char const *nameSet)
{
    char cmd[30] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_CHIPNAME_SET;

    if (nameSet == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(nameSet) > 18)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+NAME=%s", nameSet);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_SleepSwitch(ch9141_t *handle, ch9141_FuncState_t funcState)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_SWITCH;

    /* Check if it is supported by platform */
    if (handle->interface->pinSleep == NULL)
    {
        handle->error = CH9141_ERR_INTERFACE;
        return;
    }

    switch (funcState)
    {
    case CH9141_FUNC_STATE_DISABLE:
        handle->interface->pinSleep(CH9141_PIN_STATE_SET);
        break;

    case CH9141_FUNC_STATE_ENABLE:
        handle->interface->pinSleep(CH9141_PIN_STATE_RESET);
        break;

    default:
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    handle->interface->delay(100);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_SleepMode_t CH9141_SleepGet(ch9141_t *handle)
{
    char *pResponse = handle->rxBuf;

    if (handle == NULL)
        return CH9141_SLEEPMODE_UNDEFINED;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_SLEEPMODE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+SLEEP?");
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_SLEEPMODE_UNDEFINED;

    /* This cmd does not match the regular response message pattern */
    /* Seek for the first digit in response message */
    while (!isdigit(*pResponse))
    {
        if ((*(pResponse) == '\r') || (*(pResponse) == '\0'))
        {
            /* Can't find any digit */
            handle->error = CH9141_ERR_RESPONSE;

            return CH9141_SLEEPMODE_UNDEFINED;
        }
        ++pResponse;
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (ch9141_SleepMode_t) atoi(pResponse);
}

void CH9141_SleepSet(ch9141_t *handle, ch9141_SleepMode_t sleepMode)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_SET;

    if (sleepMode == CH9141_SLEEPMODE_UNDEFINED)
        return;

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+SLEEP=%u", sleepMode);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_Power_t CH9141_PowerGet(ch9141_t *handle)
{
    char *pResponse = handle->rxBuf;

    if (handle == NULL)
        return CH9141_POWER_UNDEFINED;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_POWER_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_POWER_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+TPL?");
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_POWER_UNDEFINED;

    /* This cmd does not match the regular response message pattern */
    /* Seek for the first digit in response message */
    while (!isdigit(*pResponse))
        if (*(pResponse++) == '\0')
        {
            /* Can't find any digit */
            handle->error = CH9141_ERR_RESPONSE;
            return CH9141_POWER_UNDEFINED;
        }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (ch9141_Power_t) atoi(pResponse);
}

void CH9141_PowerSet(ch9141_t *handle, ch9141_Power_t power)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_POWER_SET;

    if (power == CH9141_POWER_UNDEFINED)
        return;

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+TPL=%u", power);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_Mode_t CH9141_ModeGet(ch9141_t *handle)
{
    if (handle == NULL)
        return CH9141_MODE_UNDEFINED;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_MODE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_MODE_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+BLEMODE?");
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_MODE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (ch9141_Mode_t) atoi(handle->rxBuf);
}

void CH9141_ModeSet(ch9141_t *handle, ch9141_Mode_t mode)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MODE_SET;

    if (mode == CH9141_MODE_UNDEFINED)
        return;

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+BLEMODE=%u", mode);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_PasswordGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_PASSWORD_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+PASS?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_PasswordSet(ch9141_t *handle, char const *passwordSet, ch9141_FuncState_t funcState)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_PASSWORD_SET;

    /* Check arguments */
    if (passwordSet == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(passwordSet) != 6)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+PASS=%s", passwordSet);

    /* Update device password */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set password state ON/OFF */
    switch (funcState)
    {
    case CH9141_FUNC_STATE_DISABLE:
        CMD_Set(handle, "AT+PASEN=OFF");
        break;

    case CH9141_FUNC_STATE_ENABLE:
        CMD_Set(handle, "AT+PASEN=ON");
        break;

    default:
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_BLEStatus_t CH9141_StatusGet(ch9141_t *handle)
{
    if (handle == NULL)
        return CH9141_BLESTAT_UNDEFINED;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_BLESTAT_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_STATUS_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+BLESTA?");
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_BLESTAT_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (ch9141_BLEStatus_t) atoi(handle->rxBuf);
}

char *CH9141_MACLocalGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_LOCAL_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+MAC?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

void CH9141_MACLocalSet(ch9141_t *handle, char const *mac)
{
    char cmd[30] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_LOCAL_SET;

    if (mac == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (strlen(mac) != 17)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+MAC=%s", mac);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

char *CH9141_MACRemoteGet(ch9141_t *handle)
{
    if (handle == NULL)
        return NULL;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_REMOTE_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+CCADD?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

uint16_t CH9141_VCCGet(ch9141_t *handle)
{
    if (handle == NULL)
        return UINT16_MAX;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_VCC_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+BAT?");
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return atoi(handle->rxBuf);
}

uint16_t CH9141_ADCGet(ch9141_t *handle)
{
    if (handle == NULL)
        return UINT16_MAX;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_ADC_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+ADC?");
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return atoi(handle->rxBuf);
}

ch9141_PinState_t CH9141_GPIOGet(ch9141_t *handle, uint8_t pin)
{
    ch9141_PinState_t pinState;
    char cmd[20] = {0};

    if (handle == NULL)
        return CH9141_PIN_STATE_UNDEFINED;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_PIN_STATE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_GET;

    /* Check pin number */
    if ((pin == 0) || (pin == 2) || (pin > 7))
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return CH9141_PIN_STATE_UNDEFINED;
    }

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+GPIO%i?", pin);

    /* Request the parameter */
    CMD_Get(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_PIN_STATE_UNDEFINED;

    /* Map response message with ch9141_PinState_t */
    switch (atoi(handle->rxBuf))
    {
    case 0:
        pinState = CH9141_PIN_STATE_RESET;
        break;

    case 1:
        pinState = CH9141_PIN_STATE_SET;
        break;

    default:
        pinState = CH9141_PIN_STATE_UNDEFINED;
        break;
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return pinState;
}

void CH9141_GPIOSet(ch9141_t *handle, uint8_t pin, ch9141_PinState_t pinState)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_SET;

    /* Check pin number */
    if ((pin == 1) || (pin == 3) || (pin > 7))
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Prepare the command */
    switch (pinState)
    {
    case CH9141_PIN_STATE_RESET:
        snprintf(cmd, sizeof(cmd), "AT+GPIO%i=0", pin);
        break;

    case CH9141_PIN_STATE_SET:
        snprintf(cmd, sizeof(cmd), "AT+GPIO%i=1", pin);
        break;

    default:
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

uint16_t CH9141_GPIOInitGet(ch9141_t *handle)
{
    if (handle == NULL)
        return UINT16_MAX;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_INIT_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+INITIO?");
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (uint8_t) strtoul(handle->rxBuf, NULL, 16);
}

void CH9141_GPIOInitSet(ch9141_t *handle, uint8_t configIO)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_INIT_SET;

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+INITIO=%X", configIO);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

uint16_t CH9141_GPIOEnGet(ch9141_t *handle)
{
    if (handle == NULL)
        return UINT16_MAX;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_EN_GET;

    /* Request the parameter */
    CMD_Get(handle, "AT+IOEN?");
    if (handle->error != CH9141_ERR_NONE)
        return UINT16_MAX;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return (uint8_t) strtoul(handle->rxBuf, NULL, 16);
}

void CH9141_GPIOEnSet(ch9141_t *handle, uint8_t configIO)
{
    char cmd[20] = {0};

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_GPIO_EN_SET;

    /* Prepare the command */
    snprintf(cmd, sizeof(cmd), "AT+IOEN=%X", configIO);

    /* Set the parameter */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

/**
 * @section Private func definitions
 */

/**
 * @brief Internal function used to switch between AT and transparent modes
 * @param handle pointer to the device handle
 * @param mode mode to switch to
 */
static void ModeSwitch(ch9141_t *handle, mode_t mode)
{
    char const *successResponseTemplate = "OK\r\n";
    char response[10] = {0};
    uint16_t responseLen = 0;

    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    handle->interface->delay(500); // Enter AT configuration cmd is sent when UART is free for 500mS
    switch (mode)
    {
    case MODE_AT:
        handle->errorAT = CH9141_AT_ERR_NONE;
        if (handle->interface->pinMode != NULL)
            /* Hardware AT mode enter */
            handle->interface->pinMode(CH9141_PIN_STATE_RESET);
        else
        {
            /* Software AT mode enter */
            /* Send command */
            snprintf(handle->txBuf, sizeof(handle->txBuf), "AT...\r\n");
            if (handle->interface->transmit(handle->txBuf, strlen(handle->txBuf)) != CH9141_ERROR_STATUS_SUCCESS)
            {
                handle->error = CH9141_ERR_SERIAL_TX;
                return;
            }

            /* Get response */
            /* Use separated buffer, because driver rx buffer is used outside to keep the original cmd response */
            if (handle->interface->receive(response, sizeof(response), &responseLen) != CH9141_ERROR_STATUS_SUCCESS)
            {
                handle->error = CH9141_ERR_SERIAL_RX;
                return;
            }

            /* Check response */
            if (strncmp(response, successResponseTemplate, strlen(successResponseTemplate)) != 0)
            {
                /* Unexpected response message */
                handle->error = CH9141_ERR_RESPONSE;
                return;
            }
        }
        break;
    case MODE_TRANSPARENT:
        if (handle->interface->pinMode != NULL)
            /* Hardware transparent mode enter */
            handle->interface->pinMode(CH9141_PIN_STATE_SET);
        else
        {
            /* Software transparent mode enter */
            /* Send command */
            snprintf(handle->txBuf, sizeof(handle->txBuf), "AT+EXIT\r\n");
            if (handle->interface->transmit(handle->txBuf, strlen(handle->txBuf)) != CH9141_ERROR_STATUS_SUCCESS)
            {
                handle->error = CH9141_ERR_SERIAL_TX;
                return;
            }

            /* Get response */
            /* Use separated buffer, because driver rx buffer is used outside to keep the original cmd response */
            if (handle->interface->receive(response, sizeof(response), &responseLen) != CH9141_ERROR_STATUS_SUCCESS)
            {
                handle->error = CH9141_ERR_SERIAL_RX;
                return;
            }

            /* Check response */
            if (strncmp(response, successResponseTemplate, strlen(successResponseTemplate)) != 0)
            {
                /* Unexpected response message */
                handle->error = CH9141_ERR_RESPONSE;
                return;
            }
        }
        break;

    default:
        break;
    }
    handle->interface->delay(10);
}

/**
 * @brief Internal function used to get any device parameter represented as string
 * @param handle pointer to the device handle
 * @param cmd AT command to get the parameter. Should be null-terminated string
 */
static void CMD_Get(ch9141_t *handle, char const *cmd)
{
    if (handle == NULL)
        return;
    if (cmd == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Send the request */
    CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Retrieve response from the whole message */
    if (strtok(handle->rxBuf, "\r") == NULL)
    {
        /* Unexpected response message - `\r` token not found */
        handle->error = CH9141_ERR_RESPONSE;
        return;
    }

    /* Update response length field */
    handle->responseLen = strlen(handle->rxBuf) + 1;
}

/**
 * @brief Internal function used to set any device parameter
 * @param handle pointer to the device handle
 * @param cmd AT command to set the parameter. Should be null-terminated string
 */
static void CMD_Set(ch9141_t *handle, char const *cmd)
{
    char const *connectSuccessResponse = "LINK OK\r\n";
    char const *successResponseTemplate = "OK\r\n";
    char const *errorResponseTemplate = "\r\nERR:";
    char *pResponse = handle->rxBuf;
    char response[10] = {0};
    uint16_t responseLen = 0;
    uint8_t connectAttempt = 0;

    if (handle == NULL)
        return;
    if (cmd == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    ModeSwitch(handle, MODE_AT);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Clear RX buffer */
    memset(handle->rxBuf, '\0', sizeof(handle->rxBuf));

    /* Add trailing symbols to message and pass to the tx buffer */
    snprintf(handle->txBuf, sizeof(handle->txBuf), "%s\r\n", cmd);

    /* Send AT command */
    if (handle->interface->transmit(handle->txBuf, strlen(handle->txBuf)) != CH9141_ERROR_STATUS_SUCCESS)
    {
        handle->error = CH9141_ERR_SERIAL_TX;
        return;
    }

    /* Get response */
    if (handle->interface->receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen) != CH9141_ERROR_STATUS_SUCCESS)
    {
        handle->error = CH9141_ERR_SERIAL_RX;
        return;
    }

    /* Seek for `OK` in response message */
    if (strstr(handle->rxBuf, successResponseTemplate) == NULL)
    {
        /* Unexpected response message */
        handle->error = CH9141_ERR_RESPONSE;
        return;
    }

    /* Check for error message in the buffer */
    if (strncmp(handle->rxBuf, errorResponseTemplate, strlen(errorResponseTemplate)) == 0)
    {
        /* Seek for the first digit in response message */
        while (!isdigit(*pResponse))
            if (*(pResponse++) == '\0')
            {
                /* Can't find any digit */
                handle->error = CH9141_ERR_RESPONSE;
                return;
            }

        /* Convert msg->string->integer and fill the field within handle */
        handle->error = CH9141_ERR_AT;
        handle->errorAT = (ch9141_AT_Error_t) atoi(strtok(pResponse, "\r"));
        return;
    }

    /* Special case: if connect cmd is issued, check for "LINK OK" before enter transparent mode */
    if (strncmp(cmd, "AT+CONN", strlen("AT+CONN")) == 0)
    {
        /* Check the connect status response */
        for (connectAttempt = 5; connectAttempt; --connectAttempt)
        {
            /* Get response */
            /* Use separated buffer, because driver rx buffer is used outside to keep the original cmd response */
            if (handle->interface->receive(response, sizeof(response), &responseLen) == CH9141_ERROR_STATUS_SUCCESS)
                break;
        }
        if (!connectAttempt)
        {
            /* Run out of attempts to get the message from device */
            handle->error = CH9141_ERR_SERIAL_RX;
            return;
        }

        /* Check response */
        if (strncmp(response, connectSuccessResponse, strlen(connectSuccessResponse)) != 0)
        {
            /* Unexpected response message */
            handle->error = CH9141_ERR_RESPONSE;
            return;
        }
    }

    /* Back to transparent mode, except for reset cmd */
    if (strncmp(cmd, "AT+RESET", strlen("AT+RESET")) != 0)
        ModeSwitch(handle, MODE_TRANSPARENT);
}

/**
 * @brief Internal function used to reset the device after any settting command
 * @param handle pointer to the device handle
 */
static void Reset(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    if (handle->interface->pinReset == NULL)
    {
        /* Set the parameter */
        CMD_Set(handle, "AT+RESET");
        if (handle->error != CH9141_ERR_NONE)
            return;
    }
    else
    {
        handle->interface->pinReset(CH9141_PIN_STATE_RESET);
        handle->interface->delay(10);
        handle->interface->pinReset(CH9141_PIN_STATE_SET);
    }

    handle->interface->delay(300);
}

/**
 * @brief Internal function used to restore factory settings
 * @param handle pointer to the device handle
 * @note Device can be reloaded with two ways:
 *
 * 1. Through AT command
 *
 * 2. By pulling down `RELOAD/LED` pin for 2 sec after device is powered on
 */
static void Reload(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    if (handle->interface->pinReload == NULL)
    {
        /* Set the parameter */
        CMD_Set(handle, "AT+RELOAD");
        if (handle->error != CH9141_ERR_NONE)
            return;
    }
    else
    {
        if (handle->interface->pinReset == NULL)
        {
            handle->error = CH9141_ERR_INTERFACE;
            return;
        }

        handle->interface->pinReload(CH9141_PIN_STATE_RESET);
        Reset(handle);
        if (handle->error != CH9141_ERR_NONE)
            return;

        handle->interface->delay(2500);
        handle->interface->pinReload(CH9141_PIN_STATE_SET);
    }

    /* Reset device to take effect */
    Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;
}