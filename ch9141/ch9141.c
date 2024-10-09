#include "ch9141.h"

#define CH9141_RX_TIMEOUT 500
#define CH9141_TX_TIMEOUT 500
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

static void CH9141_Sleep(ch9141_t *handle, ch9141_FuncState_t newState);
static ch9141_ErrorStatus_t CH9141_HelloGet(ch9141_t *handle, char *helloGet, char const *helloCmp);
static ch9141_ErrorStatus_t CH9141_HelloSet(ch9141_t *handle, char const *helloSet);
static ch9141_ErrorStatus_t CH9141_CMD_Send(ch9141_t *handle, char const *cmd);

ch9141_ErrorStatus_t CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode, ch9141_Delay_fp fpDelay)
{
    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if ((fpReceive == NULL) || (fpTransmit == NULL) || (fpPinSleep == NULL) || (fpPinMode == NULL) || (fpDelay == NULL))
        return CH9141_STAT_ERROR;

    handle->interface.receive = fpReceive;
    handle->interface.transmit = fpTransmit;
    handle->interface.pinSleep = fpPinSleep;
    handle->interface.pinMode = fpPinMode;
    handle->interface.delay = fpDelay;

    return CH9141_STAT_SUCCESS;
}

ch9141_ErrorStatus_t CH9141_Init(ch9141_t *handle, char const *helloSet)
{
    char helloGet[30];

    if (handle == NULL)
        return CH9141_STAT_ERROR;

    /* Exit from sleep mode */
    CH9141_Sleep(handle, CH9141_FUNC_DISABLE);
    WAIT(100);

    /* Get any hello message after startup */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen, CH9141_RX_TIMEOUT) !=
        CH9141_STAT_SUCCESS)
        return CH9141_STAT_ERROR;

    if (helloSet != NULL)
    {
        /* Set new hello message */
        CH9141_HelloSet(handle, helloSet);

        /* Read hello message back and compare it with reference */
        CH9141_HelloGet(handle, helloGet, helloSet);
    }
    WAIT(100);

    return CH9141_STAT_SUCCESS;
}

ch9141_ErrorStatus_t CH9141_PasswordGet(ch9141_t *handle)
{
    if (handle == NULL)
        return CH9141_STAT_ERROR;

    /* Clear password field */
    memset(handle->password, '\0', sizeof(handle->password));

    /* Get password before '\r\nOK' and fill the field within handle */
    CH9141_CMD_Send(handle, "AT+PASS?");
    strncpy(handle->password, handle->rxBuf, strstr(handle->rxBuf, "\r\nOK") - handle->rxBuf); // FIXME: HardFault
}

ch9141_ErrorStatus_t CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState)
{
    char cmd[20];

    if (handle == NULL)
        return CH9141_STAT_ERROR;

    /* Prepare the command */
    strcpy(cmd, "AT+PASS=");
    strcat(cmd, handle->password);

    /* Set new password */
    CH9141_CMD_Send(handle, cmd);

    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        CH9141_CMD_Send(handle, "AT+PASEN=OFF");
        break;

    case CH9141_FUNC_ENABLE:
        CH9141_CMD_Send(handle, "AT+PASEN=ON");
        break;

    default:
        return CH9141_STAT_ERROR;
        break;
    }

    /* Reset device to take effect */
    CH9141_CMD_Send(handle, "AT+RESET");
    WAIT(100);

    return CH9141_STAT_SUCCESS;
}

/**
 * @section Private func definitions
 */
static void CH9141_Sleep(ch9141_t *handle, ch9141_FuncState_t newState)
{
    if (handle == NULL)
        return;

    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        handle->interface.pinSleep(CH9141_PIN_SET);
        break;

    case CH9141_FUNC_ENABLE:
        handle->interface.pinSleep(CH9141_PIN_RESET);
        break;

    default:
        break;
    }
}

static ch9141_ErrorStatus_t CH9141_HelloGet(ch9141_t *handle, char *helloGet, char const *helloCmp)
{
    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if (helloGet == NULL)
        return CH9141_STAT_ERROR;

    /* Get hello message before '\r\nOK' */
    CH9141_CMD_Send(handle, "AT+HELLO?");
    strncpy(helloGet, handle->rxBuf, strstr(handle->rxBuf, "\r\nOK") - handle->rxBuf);

    /* Compare it with reference message */
    if (helloCmp != NULL)
    {
        if (strncmp(helloGet, helloCmp, strlen(helloCmp)) != 0)
            return CH9141_STAT_ERROR;
    }

    return CH9141_STAT_SUCCESS;
}

static ch9141_ErrorStatus_t CH9141_HelloSet(ch9141_t *handle, char const *helloSet)
{
    char cmd[50];

    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if (helloSet == NULL)
        return CH9141_STAT_ERROR;
    if (strlen(helloSet) >= 30)
        return CH9141_STAT_ERROR;

    /* Prepare the command */
    strcpy(cmd, "AT+HELLO=");
    strcat(cmd, helloSet);

    /* Set custom hello message */
    CH9141_CMD_Send(handle, cmd);

    return CH9141_STAT_SUCCESS;
}

static ch9141_ErrorStatus_t CH9141_CMD_Send(ch9141_t *handle, char const *cmd)
{
    char *errorCodeStart;
    char const *errorResponseTemplate = "\r\nERR:";

    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if (cmd == NULL)
        return CH9141_STAT_ERROR;

    /* Enter AT mode */
    MODE_AT;
    WAIT(100);

    /* Add trailing symbols */
    strcpy(handle->txBuf, cmd);
    strcat(handle->txBuf, "\r\n");

    /* Send AT command */
    if (handle->interface.transmit(handle->txBuf, strlen(handle->txBuf), CH9141_TX_TIMEOUT) != CH9141_STAT_SUCCESS)
    {
        MODE_TRANSPARENT;
        return CH9141_STAT_ERROR;
    }

    /* Get response */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen, CH9141_RX_TIMEOUT) !=
        CH9141_STAT_SUCCESS)
    {
        MODE_TRANSPARENT;
        return CH9141_STAT_ERROR;
    }

    /* Check for error code in the buffer */
    if (strncmp(handle->rxBuf, errorResponseTemplate, strlen(errorResponseTemplate)) == 0)
    {
        /* Seek error code after colon */
        errorCodeStart = strstr(handle->rxBuf, errorResponseTemplate) + strlen(errorResponseTemplate);

        /* Fix error code in the device handle */
        handle->AT_Error = (ch9141_AT_Error_t) atoi(errorCodeStart);

        MODE_TRANSPARENT;
        return CH9141_STAT_ERROR;
    }
    MODE_TRANSPARENT;

    return CH9141_STAT_SUCCESS;
}