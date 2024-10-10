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
#define CH9141_ERROR_CHECK                    \
    do                                        \
    {                                         \
        if (handle->error != CH9141_ERR_NONE) \
        {                                     \
            MODE_TRANSPARENT;                 \
            return;                           \
        }                                     \
    }                                         \
    while (0)
#define CH9141_ERROR_SET(ERR, BEHAVIOUR) \
    do                                   \
    {                                    \
        handle->error = (ERR);           \
        if ((BEHAVIOUR) == CH9141_KEEP)  \
            CH9141_ErrorHandler(handle); \
        else                             \
            return;                      \
    }                                    \
    while (0)

typedef enum { CH9141_KEEP = 15, CH9141_EXIT } ch9141_Behaviour;

static void CH9141_Sleep(ch9141_t *handle, ch9141_FuncState_t newState);
static void CH9141_ModeGet(ch9141_t *handle);
static void CH9141_ModeSet(ch9141_t *handle);
static void CH9141_HelloGet(ch9141_t *handle, char *helloDest, uint8_t helloSize, char const *helloRef);
static void CH9141_HelloSet(ch9141_t *handle, char const *helloSet);
static void CH9141_CMD_Get(ch9141_t *handle, char *dest, uint8_t destSize, const char *cmd);
static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd);
static void CH9141_ErrorHandler(ch9141_t *handle);

void CH9141_Link(ch9141_t *handle, ch9141_Receive_fp fpReceive, ch9141_Transmit_fp fpTransmit,
                 ch9141_Pin_Sleep_fp fpPinSleep, ch9141_Pin_Mode_fp fpPinMode, ch9141_Delay_fp fpDelay)
{
    if (handle == NULL)
        return;
    if ((fpReceive == NULL) || (fpTransmit == NULL) || (fpPinSleep == NULL) || (fpPinMode == NULL) || (fpDelay == NULL))
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);

    /* Clear all handle fields */
    memset(handle, '\0', sizeof(ch9141_t));

    /* Link platform functions to the device */
    handle->interface.receive = fpReceive;
    handle->interface.transmit = fpTransmit;
    handle->interface.pinSleep = fpPinSleep;
    handle->interface.pinMode = fpPinMode;
    handle->interface.delay = fpDelay;
}

void CH9141_Init(ch9141_t *handle)
{
    ch9141_Mode_t modeToSet;

    if (handle == NULL)
        return;

    /* Exit from sleep mode */
    WAIT(100);
    CH9141_Sleep(handle, CH9141_FUNC_DISABLE);

    /* Get any hello message after startup */
    handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen, CH9141_RX_TIMEOUT);

    /* Fix mode to be set to */
    modeToSet = handle->mode;

    /* Set new BLE mode */
    CH9141_ModeSet(handle);

    /* Reset device to take effect */
    CH9141_CMD_Set(handle, "AT+RESET");
    WAIT(100);

    /* Check mode switch result */
    CH9141_ModeGet(handle);
    if (handle->mode != modeToSet)
        CH9141_ERROR_SET(CH9141_ERR_RESPONSE, CH9141_KEEP);
}

void CH9141_PasswordGet(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    /* Get password and fill the field within handle */
    CH9141_CMD_Get(handle, handle->password, sizeof(handle->password), "AT+PASS?");
    CH9141_ERROR_CHECK;
}

void CH9141_PasswordSet(ch9141_t *handle, ch9141_FuncState_t newState)
{
    char cmd[20];

    if (handle == NULL)
        return;

    /* Prepare the command */
    strcpy(cmd, "AT+PASS=");
    strcat(cmd, handle->password);

    /* Set new password */
    CH9141_CMD_Set(handle, cmd);
    CH9141_ERROR_CHECK;

    switch (newState)
    {
    case CH9141_FUNC_DISABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=OFF");
        CH9141_ERROR_CHECK;
        break;

    case CH9141_FUNC_ENABLE:
        CH9141_CMD_Set(handle, "AT+PASEN=ON");
        CH9141_ERROR_CHECK;
        break;

    default:
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);
    }

    /* Reset device to take effect */
    CH9141_CMD_Set(handle, "AT+RESET");
    CH9141_ERROR_CHECK;
    WAIT(100);
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
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);
    }
}

static void CH9141_ModeGet(ch9141_t *handle)
{
    char mode[2];

    if (handle == NULL)
        return;

    /* Get current BLE working mode */
    CH9141_CMD_Get(handle, mode, sizeof(mode), "AT+BLEMODE?");
    CH9141_ERROR_CHECK;

    /* Fill the field within handle */
    handle->mode = (ch9141_Mode_t) atoi(mode);
}

static void CH9141_ModeSet(ch9141_t *handle)
{
    if (handle == NULL)
        return;

    switch (handle->mode)
    {
    case CH9141_MODE_BROADCAST: // Restricted by driver
        CH9141_CMD_Set(handle, "AT+BLEMODE=0");
        CH9141_ERROR_CHECK;
        break;

    case CH9141_MODE_HOST: // Restricted by driver
        CH9141_CMD_Set(handle, "AT+BLEMODE=1");
        CH9141_ERROR_CHECK;
        break;

    case CH9141_MODE_DEVICE:
        CH9141_CMD_Set(handle, "AT+BLEMODE=2");
        CH9141_ERROR_CHECK;
        break;

    default:
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);
    }
}

static void CH9141_HelloGet(ch9141_t *handle, char *helloDest, uint8_t helloSize, char const *helloRef)
{
    if (handle == NULL)
        return;
    if (helloDest == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);

    /* Get hello message before */
    CH9141_CMD_Get(handle, helloDest, helloSize, "AT+HELLO?");
    CH9141_ERROR_CHECK;

    /* Compare it with reference message */
    if (helloRef != NULL)
    {
        if (strncmp(helloDest, helloRef, strlen(helloRef)) != 0)
            CH9141_ERROR_SET(CH9141_ERR_RESPONSE, CH9141_KEEP);
    }
}

static void CH9141_HelloSet(ch9141_t *handle, char const *newHello)
{
    char cmd[50];

    if (handle == NULL)
        return;
    if (newHello == NULL)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);
    if (strlen(newHello) >= 30)
        CH9141_ERROR_SET(CH9141_ERR_ARGUMENT, CH9141_KEEP);

    /* Prepare the command */
    strcpy(cmd, "AT+HELLO=");
    strcat(cmd, newHello);

    /* Set custom hello message */
    CH9141_CMD_Set(handle, cmd);
    CH9141_ERROR_CHECK;
}

static void CH9141_CMD_Get(ch9141_t *handle, char *dest, uint8_t destSize, const char *cmd)
{
    uint8_t responseSize;

    /* Send the request */
    CH9141_CMD_Set(handle, cmd);
    CH9141_ERROR_CHECK;

    /* Calculate response size and check it to match the destination size */
    responseSize = strstr(handle->rxBuf, "\r\nOK") - handle->rxBuf;
    if (responseSize > destSize)
        CH9141_ERROR_SET(CH9141_ERR_MEMORY, CH9141_EXIT); // Response don't fit the destination buffer size

    /* Extract response and pass it to the destination */
    strncpy(dest, handle->rxBuf, responseSize);
}

static void CH9141_CMD_Set(ch9141_t *handle, char const *cmd)
{
    char *errorCodeStart;
    char const *errorResponseTemplate = "\r\nERR:";

    /* Enter AT mode */
    MODE_AT;
    WAIT(100);

    /* Clear RX buffer */
    memset(handle->rxBuf, '\0', sizeof(handle->rxBuf));

    /* Add trailing symbols */
    strcpy(handle->txBuf, cmd);
    strcat(handle->txBuf, "\r\n");

    /* Send AT command */
    if (handle->interface.transmit(handle->txBuf, strlen(handle->txBuf), CH9141_TX_TIMEOUT) != CH9141_STAT_SUCCESS)
        CH9141_ERROR_SET(CH9141_ERR_SERIAL, CH9141_EXIT);

    /* Get response */
    if (handle->interface.receive(handle->rxBuf, sizeof(handle->rxBuf), &handle->rxLen, CH9141_RX_TIMEOUT) !=
        CH9141_STAT_SUCCESS)
        CH9141_ERROR_SET(CH9141_ERR_SERIAL, CH9141_EXIT);

    /* Check for error code in the buffer */
    if (strncmp(handle->rxBuf, errorResponseTemplate, strlen(errorResponseTemplate)) == 0)
    {
        /* Seek error code after colon */
        errorCodeStart = strstr(handle->rxBuf, errorResponseTemplate) + strlen(errorResponseTemplate);

        /* Fix AT error code in the device handle */
        handle->errorAT = (ch9141_AT_Error_t) atoi(errorCodeStart);

        CH9141_ERROR_SET(CH9141_ERR_AT, CH9141_EXIT);
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