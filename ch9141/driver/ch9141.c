#include "ch9141.h"

static void CH9141_CMD_Get(ch9141_t *handle, char const *cmd);
static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd);
static void CH9141_Reset(ch9141_t *handle);
static void CH9141_Reload(ch9141_t *handle);

void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Delay_fp fpDelay, ch9141_Pin_Mode_fp fpPinMode, ch9141_Pin_Reset_fp fpPinReset,
                 ch9141_Pin_Reload_fp fpPinReload, ch9141_Pin_Sleep_fp fpPinSleep)
{
    if (handle == NULL)
        return;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_LINK;

    /* Check platform functions */
    if (fpReceive == NULL || fpTransmit == NULL || fpDelay == NULL || fpPinMode == NULL || fpPinSleep == NULL)
    {
        handle->error = CH9141_ERR_INTERFACE;
        return;
    }
    if ((fpPinReload != NULL) && (fpPinReset == NULL))
    {
        handle->error = CH9141_ERR_INTERFACE;
        return;
    }

    /* Clear all interface fields */
    memset(&handle->interface, 0, sizeof(handle->interface));

    /* Link platform functions to the device */
    handle->interface.receive = fpReceive;
    handle->interface.transmit = fpTransmit;
    handle->interface.delay = fpDelay;
    handle->interface.pinReset = fpPinReset;
    handle->interface.pinMode = fpPinMode;
    handle->interface.pinReload = fpPinReload;
    handle->interface.pinSleep = fpPinSleep;

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
    handle->interface.pinSleep(CH9141_PIN_STATE_SET);
    handle->interface.delay(100);

    /* Restore factory settings if requested */
    if (factoryRestore)
        CH9141_Reload(handle);
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
    CH9141_CMD_Get(handle, "AT+UART?");
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
    sprintf(cmd, "AT+UART=%u,%u,%u,%u,%u", baudRate, dataBit, stopBit, parity, timeout);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Set(handle, "AT+DISCONN");
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
    CH9141_CMD_Get(handle, "AT+HELLO?");
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
    strcpy(cmd, "AT+HELLO=");
    strcat(cmd, helloSet);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+PNAME?");
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
    strcpy(cmd, "AT+PNAME=");
    strcat(cmd, nameSet);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+NAME?");
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
    strcpy(cmd, "AT+NAME=");
    strcat(cmd, nameSet);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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

    switch (funcState)
    {
    case CH9141_FUNC_STATE_DISABLE:
        handle->interface.pinSleep(CH9141_PIN_STATE_SET);
        break;

    case CH9141_FUNC_STATE_ENABLE:
        handle->interface.pinSleep(CH9141_PIN_STATE_RESET);
        break;

    default:
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

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
    CH9141_CMD_Get(handle, "AT+SLEEP?");
    if (handle->error != CH9141_ERR_NONE)
        return CH9141_SLEEPMODE_UNDEFINED;

    /* This cmd does not match the regular response message pattern */
    /* Seek for the first digit in response message */
    while (!isdigit(*pResponse))
        if (*(pResponse++) == '\0')
        {
            /* Can't find any digit */
            handle->error = CH9141_ERR_RESPONSE;
            return CH9141_SLEEPMODE_UNDEFINED;
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
    sprintf(cmd, "AT+SLEEP=%u", sleepMode);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+TPL?");
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
    sprintf(cmd, "AT+TPL=%u", power);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+BLEMODE?");
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
    sprintf(cmd, "AT+BLEMODE=%u", mode);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+PASS?");
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
    strcpy(cmd, "AT+PASS=");
    strcat(cmd, passwordSet);

    /* Update device password */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Set password state ON/OFF */
    switch (funcState)
    {
    case CH9141_FUNC_STATE_DISABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=OFF");
        break;

    case CH9141_FUNC_STATE_ENABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=ON");
        break;

    default:
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+BLESTA?");
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
    CH9141_CMD_Get(handle, "AT+MAC?");
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
    strcpy(cmd, "AT+MAC=");
    strcat(cmd, mac);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Reset device to take effect */
    CH9141_Reset(handle);
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
    CH9141_CMD_Get(handle, "AT+CCADD?");
    if (handle->error != CH9141_ERR_NONE)
        return NULL;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->rxBuf;
}

uint16_t CH9141_VCCGet(ch9141_t *handle)
{
    if (handle == NULL)
        return 0;

    /* Check any existing errors */
    if (handle->error != CH9141_ERR_NONE)
        return 0;

    /* Set operational state */
    handle->state = CH9141_STATE_VCC_GET;

    /* Request the parameter */
    CH9141_CMD_Get(handle, "AT+BAT?");
    if (handle->error != CH9141_ERR_NONE)
        return 0;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return atoi(handle->rxBuf);
}

/**
 * @section Private func definitions
 */

/**
 * @brief Internal function used to get any device parameter represented as string
 * @param handle Pointer to the device handle
 * @param cmd AT command to get the parameter. Should be null-terminated string
 */
static void CH9141_CMD_Get(ch9141_t *handle, char const *cmd)
{
    if (handle == NULL)
        return;
    if (cmd == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* Send the request */
    CH9141_CMD_Set(handle, cmd);
    if (handle->error != CH9141_ERR_NONE)
        return;

    /* Retrieve response from the whole message */
    if (strtok(handle->rxBuf, "\r") == NULL)
    {
        handle->error = CH9141_ERR_RESPONSE;
        return;
    }

    /* Update response length field */
    handle->responseLen = strlen(handle->rxBuf) + 1;
}

/**
 * @brief Internal function used to set any device parameter
 * @param handle Pointer to the device handle
 * @param cmd AT command to set the parameter. Should be null-terminated string
 */
static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd)
{
    char const *errorResponseTemplate = "\r\nERR:";
    char *pResponse = handle->rxBuf;

    if (handle == NULL)
        return;
    if (cmd == NULL)
    {
        handle->error = CH9141_ERR_ARGUMENT;
        return;
    }

    /* AT mode */
    handle->errorAT = CH9141_AT_ERR_NONE;
    handle->interface.pinMode(CH9141_PIN_STATE_RESET);
    handle->interface.delay(10);

    /* Clear RX buffer */
    memset(handle->rxBuf, '\0', sizeof(handle->rxBuf));

    /* Add trailing symbols */
    strcpy(handle->txBuf, cmd);
    strcat(handle->txBuf, "\r\n");

    /* Send AT command */
    if (handle->interface.transmit(handle->txBuf, strlen(handle->txBuf)) != CH9141_ERROR_STATUS_SUCCESS)
    {
        handle->error = CH9141_ERR_SERIAL_TX;
        handle->interface.pinMode(CH9141_PIN_STATE_SET);
        return;
    }

    /* Get response */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen) != CH9141_ERROR_STATUS_SUCCESS)
    {
        handle->error = CH9141_ERR_SERIAL_RX;
        handle->interface.pinMode(CH9141_PIN_STATE_SET);
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
                handle->interface.pinMode(CH9141_PIN_STATE_SET);
                return;
            }

        /* Convert msg->string->integer and fill the field within handle */
        handle->error = CH9141_ERR_AT;
        handle->errorAT = (ch9141_AT_Error_t) atoi(strtok(pResponse, "\r"));
        handle->interface.pinMode(CH9141_PIN_STATE_SET);
        return;
    }

    /* Transparent mode */
    handle->interface.pinMode(CH9141_PIN_STATE_SET);
}

/**
 * @brief Internal function used to reset the device after any settting command
 * @param handle Pointer to the device handle
 */
static void CH9141_Reset(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    if (handle->interface.pinReset == NULL)
    {
        /* Set the parameter */
        CH9141_CMD_Set(handle, "AT+RESET");
        if (handle->error != CH9141_ERR_NONE)
            return;
    }
    else
    {
        handle->interface.pinReset(CH9141_PIN_STATE_RESET);
        handle->interface.delay(10);
        handle->interface.pinReset(CH9141_PIN_STATE_SET);
    }

    handle->interface.delay(200);
}

/**
 * @brief Internal function used to restore factory settings
 * @param handle Pointer to the device handle
 * @note Device can be reloaded with two ways:
 *
 * 1. Through AT command
 *
 * 2. By pulling down `RELOAD/LED` pin for 2 sec after device is powered on
 */
static void CH9141_Reload(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    if (handle->interface.pinReload == NULL)
    {
        /* Set the parameter */
        CH9141_CMD_Set(handle, "AT+RELOAD");
        if (handle->error != CH9141_ERR_NONE)
            return;
    }
    else
    {
        if (handle->interface.pinReset == NULL)
        {
            handle->error = CH9141_ERR_INTERFACE;
            return;
        }

        handle->interface.pinReload(CH9141_PIN_STATE_RESET);
        CH9141_Reset(handle);
        if (handle->error != CH9141_ERR_NONE)
            return;

        handle->interface.delay(2500);
        handle->interface.pinReload(CH9141_PIN_STATE_SET);
    }

    /* Reset device to take effect */
    CH9141_Reset(handle);
    if (handle->error != CH9141_ERR_NONE)
        return;
}