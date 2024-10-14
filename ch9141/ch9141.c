#include "ch9141.h"

#define WAIT(MSEC)                     \
    do                                 \
    {                                  \
        handle->interface.delay(MSEC); \
    }                                  \
    while (0)
#define MODE_TRANSPARENT                           \
    do                                             \
    {                                              \
        handle->interface.pinMode(CH9141_PIN_SET); \
        WAIT(100);                                 \
    }                                              \
    while (0)
#define MODE_AT                                      \
    do                                               \
    {                                                \
        handle->interface.pinMode(CH9141_PIN_RESET); \
        WAIT(100);                                   \
    }                                                \
    while (0)
#define CH9141_ERROR_SET(ERR)        \
    do                               \
    {                                \
        handle->error = (ERR);       \
        CH9141_ErrorHandler(handle); \
    }                                \
    while (0)

static void CH9141_CMD_Get(ch9141_t *handle, char *dest, uint8_t destSize, const char *cmd);
static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd);
static void CH9141_ErrorHandler(ch9141_t *handle);

void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode)
{
    if (handle == NULL)
        return;

    /* Clear all handle fields */
    memset(handle, '\0', sizeof(ch9141_t));

    /* Set operational state */
    handle->state = CH9141_STATE_LINK;

    if ((fpReceive == NULL) || (fpTransmit == NULL) || (fpPinSleep == NULL) || (fpPinMode == NULL))
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);

    /* Link platform functions to the device */
    handle->interface.receive = fpReceive;
    handle->interface.transmit = fpTransmit;
    handle->interface.pinSleep = fpPinSleep;
    handle->interface.pinMode = fpPinMode;
    handle->interface.delay = CH9141_Delay;

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_Init(ch9141_t *handle)
{
    ch9141_Mode_t modeSet = handle->bleMode;
    ch9141_Power_t powerSet = handle->power;

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_INIT;

    /* Exit from sleep mode */
    WAIT(100);
    CH9141_SleepSwitch(handle, CH9141_FUNC_DISABLE);

    /* Get any hello message after startup */
    handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen);

    /* Set new device parameters */
    CH9141_ModeSet(handle);
    CH9141_PowerSet(handle);

    /* Reset device to take effect */
    CH9141_Reset(handle);

    /* Check results after reset */
    if (CH9141_ModeGet(handle) != modeSet)
        CH9141_ERROR_SET(CH9141_ERR_RESPONSE);
    if (CH9141_PowerGet(handle) != powerSet)
        CH9141_ERROR_SET(CH9141_ERR_RESPONSE);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_Reset(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_RESET;

    CH9141_CMD_Set(handle, "AT+RESET");
    WAIT(100);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_SleepSwitch(ch9141_t *handle, ch9141_FuncState_t newState)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_SWITCH;

    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        handle->interface.pinSleep(CH9141_PIN_SET);
        break;

    case CH9141_FUNC_ENABLE:
        handle->interface.pinSleep(CH9141_PIN_RESET);
        break;

    default:
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_SleepMode_t CH9141_SleepGet(ch9141_t *handle)
{
    char sleep[7];
    char *pResponse = sleep;

    if (handle == NULL)
        return CH9141_SLEEPMODE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_GET;

    /* Request parameter */
    CH9141_CMD_Get(handle, sleep, sizeof(sleep), "AT+SLEEP?");

    /* This cmd does not match the regular response message pattern */
    /* Seek for the first digit in response message */
    while (!isdigit(*pResponse))
        if (*(pResponse++) == '\0')
            CH9141_ERROR_SET(CH9141_ERR_MEMORY); // Can't find any digit

    /* Convert msg->string->integer and fill the field within handle */
    handle->sleepMode = (ch9141_SleepMode_t) atoi(strtok(pResponse, "\r\n"));

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->sleepMode;
}

void CH9141_SleepSet(ch9141_t *handle)
{
    char cmd[20];
    char param[2];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_SLEEP_SET;

    /* Prepare the command */
    strcpy(cmd, "AT+SLEEP=");
    sprintf(param, "%d", handle->sleepMode);
    strcat(cmd, param);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_Power_t CH9141_PowerGet(ch9141_t *handle)
{
    char tpl[6];
    char *pResponse = tpl;

    if (handle == NULL)
        return CH9141_POWER_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_POWER_GET;

    /* Request parameter */
    CH9141_CMD_Get(handle, tpl, sizeof(tpl), "AT+TPL?");

    /* This cmd does not match the regular response message pattern */
    /* Seek for the first digit in response message */
    while (!isdigit(*pResponse))
        if (*(pResponse++) == '\0')
            CH9141_ERROR_SET(CH9141_ERR_MEMORY); // Can't find any digit

    /* Convert msg->string->integer and fill the field within handle */
    handle->power = (ch9141_Power_t) atoi(strtok(pResponse, "\r\n"));

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->power;
}

void CH9141_PowerSet(ch9141_t *handle)
{
    char cmd[20];
    char param[2];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_POWER_SET;

    /* Prepare the command */
    strcpy(cmd, "AT+TPL=");
    sprintf(param, "%d", handle->power);
    strcat(cmd, param);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_PasswordGet(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_PASSWORD_GET;

    /* Get password and fill the field within handle */
    CH9141_CMD_Get(handle, handle->password, sizeof(handle->password), "AT+PASS?");

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState)
{
    char cmd[20];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_PASSWORD_SET;

    /* Prepare the command */
    strcpy(cmd, "AT+PASS=");
    strcat(cmd, handle->password);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=OFF");
        break;

    case CH9141_FUNC_ENABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=ON");
        break;

    default:
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_HelloGet(ch9141_t *handle, char *helloDest, uint8_t helloSize, char const *helloRef)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_HELLO_GET;

    if (helloDest == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);

    /* Get hello message before */
    CH9141_CMD_Get(handle, helloDest, helloSize, "AT+HELLO?");

    /* Compare it with reference message */
    if (helloRef != NULL)
    {
        if (strncmp(helloDest, helloRef, strlen(helloRef)) != 0)
            CH9141_ERROR_SET(CH9141_ERR_RESPONSE);
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_HelloSet(ch9141_t *handle, char const *newHello)
{
    char cmd[50];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_HELLO_SET;

    if (newHello == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    if (strlen(newHello) >= 30)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);

    /* Prepare the command */
    strcpy(cmd, "AT+HELLO=");
    strcat(cmd, newHello);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

ch9141_Mode_t CH9141_ModeGet(ch9141_t *handle)
{
    char mode[2];

    if (handle == NULL)
        return CH9141_MODE_UNDEFINED;

    /* Set operational state */
    handle->state = CH9141_STATE_MODE_GET;

    /* Get current BLE working mode */
    CH9141_CMD_Get(handle, mode, sizeof(mode), "AT+BLEMODE?");

    /* Fill the field within handle */
    handle->bleMode = (ch9141_Mode_t) atoi(mode);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->bleMode;
}

void CH9141_ModeSet(ch9141_t *handle)
{
    char cmd[20];
    char param[2];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MODE_SET;

    /* Prepare the command */
    strcpy(cmd, "AT+BLEMODE=");
    sprintf(param, "%d", handle->bleMode);
    strcat(cmd, param);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

bool CH9141_StatusGet(ch9141_t *handle)
{
    char status[3];

    if (handle == NULL)
        return false;

    /* Set operational state */
    handle->state = CH9141_STATE_STATUS_GET;

    /* Get current BLE status */
    CH9141_CMD_Get(handle, status, sizeof(status), "AT+BLESTA?");

    switch (handle->bleMode)
    {
    case CH9141_MODE_BROADCAST:
        handle->bleStatus.statusAdv = (ch9141_BLEStatAdv_t) atoi(status);
        break;

    case CH9141_MODE_HOST:
        handle->bleStatus.statusHost = (ch9141_BLEStatHost_t) atoi(status);
        handle->isConnected = (handle->bleStatus.statusHost == CH9141_BLESTA_HOST_CONNECTED) ? true : false;
        break;

    case CH9141_MODE_DEVICE:
        handle->bleStatus.statusSlave = (ch9141_BLEStatSlave_t) atoi(status);
        handle->isConnected = (handle->bleStatus.statusSlave == CH9141_BLESTA_SLAVE_CONNECTED) ? true : false;
        break;

    default:
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    }

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->isConnected;
}

void CH9141_MACLocalGet(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_LOCAL_GET;

    /* Get local MAC address and fill the field within handle */
    CH9141_CMD_Get(handle, handle->macLocal, sizeof(handle->macLocal), "AT+MAC?");

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

void CH9141_MACLocalSet(ch9141_t *handle)
{
    char cmd[30];

    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_LOCAL_SET;

    /* Prepare the command */
    strcpy(cmd, "AT+MAC=");
    strcat(cmd, handle->macLocal);

    /* Set the parameter */
    CH9141_CMD_Set(handle, cmd);
}

void CH9141_MACRemoteGet(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Set operational state */
    handle->state = CH9141_STATE_MAC_REMOTE_GET;

    /* Get remote MAC address and fill the field within handle */
    CH9141_CMD_Get(handle, handle->macRemote, sizeof(handle->macRemote), "AT+CCADD?");

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;
}

uint16_t CH9141_VCCGet(ch9141_t *handle)
{
    char voltage[5];

    if (handle == NULL)
        return 0;

    /* Set operational state */
    handle->state = CH9141_STATE_VCC_GET;

    /* Get chip voltage and fill the field within handle */
    CH9141_CMD_Get(handle, voltage, sizeof(voltage), "AT+BAT?");

    /* Fill the field within handle */
    handle->vcc = atoi(voltage);

    /* Set operational state */
    handle->state = CH9141_STATE_IDLE;

    return handle->vcc;
}

/**
 * @section Private func definitions
 */

/**
 * @brief Internal function used to get any device parameter represented as string
 * @param handle Pointer to the device handle
 * @param dest Pointer to the buffer where save the parameter response message
 * @param destSize A size of buffer where save the parameter response message
 * @param cmd AT command to get the parameter
 * @note The destination buffer must be large enough to fit the expected null-terminated message string
 */
static void CH9141_CMD_Get(ch9141_t *handle, char *dest, uint8_t destSize, const char *cmd)
{
    uint8_t responseSize;

    if (handle == NULL)
        return;
    if (dest == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    if (destSize == 0)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);
    if (cmd == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);

    /* Send the request */
    CH9141_CMD_Set(handle, cmd);

    /* Calculate response size and check it to match the destination size */
    responseSize = strstr(handle->rxBuf, "\r\nOK") - handle->rxBuf;
    if (responseSize > destSize)
        CH9141_ERROR_SET(CH9141_ERR_MEMORY); // Response don't fit the destination buffer size

    /* Extract response and pass it to the destination */
    strncpy(dest, handle->rxBuf, responseSize);
}

/**
 * @brief Internal function used to set any device parameter
 * @param handle Pointer to the device handle
 * @param cmd AT command to set the parameter
 */
static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd)
{
    char *pResponse;
    char const *errorResponseTemplate = "\r\nERR:";

    if (handle == NULL)
        return;
    if (cmd == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT);

    /* Enter AT mode */
    MODE_AT;
    WAIT(100);

    /* Clear RX buffer */
    memset(handle->rxBuf, '\0', sizeof(handle->rxBuf));

    /* Add trailing symbols */
    strcpy(handle->txBuf, cmd);
    strcat(handle->txBuf, "\r\n");

    /* Send AT command */
    if (handle->interface.transmit(handle->txBuf, strlen(handle->txBuf)) != CH9141_STAT_SUCCESS)
        CH9141_ERROR_SET(CH9141_ERR_SERIAL_TX);

    /* Get response */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen) != CH9141_STAT_SUCCESS)
        CH9141_ERROR_SET(CH9141_ERR_SERIAL_RX);

    /* Check for error message in the buffer */
    if (strncmp(handle->rxBuf, errorResponseTemplate, strlen(errorResponseTemplate)) == 0)
    {
        /* Seek for the first digit in response message */
        pResponse = handle->rxBuf;
        while (!isdigit(*pResponse))
            if (*(pResponse++) == '\0')
                CH9141_ERROR_SET(CH9141_ERR_MEMORY); // Can't find any digit

        /* Convert msg->string->integer and fill the field within handle */
        handle->errorAT = (ch9141_AT_Error_t) atoi(strtok(pResponse, "\r\n"));

        CH9141_ERROR_SET(CH9141_ERR_AT);
    }
    MODE_TRANSPARENT;
}

static void CH9141_ErrorHandler(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    while (true)
    {
        LEDR_ON; // FIXME: platform
    }
}