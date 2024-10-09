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
static ch9141_ErrorStatus_t CH9141_CMD_Send(ch9141_t *handle, char const *CMD);

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
    if (handle == NULL)
        return CH9141_STAT_ERROR;

    char helloGet[30];

    WAIT(1000);
    CH9141_Sleep(handle, CH9141_FUNC_DISABLE);
    MODE_AT;

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

    // MODE_TRANSPARENT;

    return CH9141_STAT_SUCCESS;
}

ch9141_ErrorStatus_t CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState)
{
    if (handle == NULL)
        return CH9141_STAT_ERROR;

    /* Set new password */
    CH9141_CMD_Send(handle, "AT+PAS\r\n");

    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        /* code */
        break;

    case CH9141_FUNC_ENABLE:
        /* code */
        break;

    default:
        return CH9141_STAT_ERROR;
        break;
    }
}

ch9141_ErrorStatus_t CH9141_PasswordGet(ch9141_t *handle)
{
    if (handle == NULL)
        return CH9141_STAT_ERROR;
}

/**
 * @section Private func definitions
 */
static void CH9141_Sleep(ch9141_t *handle, ch9141_FuncState_t newState)
{
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

    /* Get hello message */
    CH9141_CMD_Send(handle, "AT+HELLO?\r\n");
    strncpy(helloGet, handle->rxBuf, strstr(handle->rxBuf, "\r\n") - handle->rxBuf);

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
    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if (helloSet == NULL)
        return CH9141_STAT_ERROR;
    if (strlen(helloSet) >= 30)
        return CH9141_STAT_ERROR;

    char msg[50] = "AT+HELLO=";
    strcat(msg, helloSet);
    strcat(msg, "\r\n");

    /* Set custom hello message */
    CH9141_CMD_Send(handle, msg);

    return CH9141_STAT_SUCCESS;
}

static ch9141_ErrorStatus_t CH9141_CMD_Send(ch9141_t *handle, char const *CMD)
{
    char *errorCodeStart;
    char const *errorResponseTemplate = "\r\nERR:";

    if (handle == NULL)
        return CH9141_STAT_ERROR;
    if (CMD == NULL)
        return CH9141_STAT_ERROR;

    WAIT(100);

    /* Send AT command */
    if (handle->interface.transmit(CMD, strlen(CMD), CH9141_TX_TIMEOUT) != CH9141_STAT_SUCCESS)
        return CH9141_STAT_ERROR;

    /* Get response */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen, CH9141_RX_TIMEOUT) !=
        CH9141_STAT_SUCCESS)
        return CH9141_STAT_ERROR;

    /* Check for error code in the buffer */
    if (strncmp(handle->rxBuf, errorResponseTemplate, strlen(errorResponseTemplate)) == 0)
    {
        /* Seek error code after colon */
        errorCodeStart = strchr(handle->rxBuf, ':') + 1;

        /* Fix error code in the device handle */
        handle->AT_Error = (ch9141_AT_Error_t) atoi(errorCodeStart);

        return CH9141_STAT_ERROR;
    }

    return CH9141_STAT_SUCCESS;
}