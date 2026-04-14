#include "test_uart.h"

#include "string.h"
#include "usart.h"

#define TAILLE_BUFFER_RECEPTION     160U

typedef struct
{
  uint32_t nbMessagesEmis;
  uint32_t nbMessagesRecus;
  uint8_t indexReception;
  uint8_t receptionArmee;
  uint8_t octetRecu;
  uint8_t messageRecuPret;
  char bufferReception[TAILLE_BUFFER_RECEPTION];
  char dernierMessage[TAILLE_BUFFER_RECEPTION];
} EtatTestUart;

static EtatTestUart etatTestUart;
static void ReinitialiserReception(void);
static void DemarrerReceptionUart(void);
static void TraiterOctetRecu(uint8_t octetRecu);

void TestUart_Initialiser(void)
{
  memset(&etatTestUart, 0, sizeof(etatTestUart));
  ReinitialiserReception();
  DemarrerReceptionUart();
}

uint8_t TestUart_EnvoyerMessage(const char *message)
{
  uint16_t longueurMessage;

  if (message == NULL)
  {
    return 0U;
  }

  longueurMessage = (uint16_t)strlen(message);
  if (longueurMessage == 0U)
  {
    return 0U;
  }

  if (HAL_UART_Transmit(&huart7, (uint8_t *)message, longueurMessage, 100U) != HAL_OK)
  {
    return 0U;
  }

  etatTestUart.nbMessagesEmis++;
  if (longueurMessage >= sizeof(etatTestUart.dernierMessage))
  {
    longueurMessage = (uint16_t)(sizeof(etatTestUart.dernierMessage) - 1U);
  }

  strncpy(etatTestUart.dernierMessage, message, longueurMessage);
  etatTestUart.dernierMessage[longueurMessage] = '\0';

  return 1U;
}

uint8_t TestUart_MessageRecuEstPret(void)
{
  return etatTestUart.messageRecuPret;
}

uint8_t TestUart_RecupererDernierMessageRecu(char *message, uint16_t tailleMessage)
{
  size_t longueurMessage;

  if ((message == NULL) || (tailleMessage == 0U) || (etatTestUart.messageRecuPret == 0U))
  {
    return 0U;
  }

  longueurMessage = strlen(etatTestUart.dernierMessage);
  if (longueurMessage >= tailleMessage)
  {
    return 0U;
  }

  strncpy(message, etatTestUart.dernierMessage, tailleMessage - 1U);
  message[tailleMessage - 1U] = '\0';
  return 1U;
}

void TestUart_AcquitterDernierMessageRecu(void)
{
  etatTestUart.messageRecuPret = 0U;
}

static void ReinitialiserReception(void)
{
  memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
  memset(etatTestUart.dernierMessage, 0, sizeof(etatTestUart.dernierMessage));
  etatTestUart.indexReception = 0U;
  etatTestUart.nbMessagesRecus = 0U;
  etatTestUart.nbMessagesEmis = 0U;
  etatTestUart.messageRecuPret = 0U;
  etatTestUart.receptionArmee = 0U;
  etatTestUart.octetRecu = 0U;
}

static void DemarrerReceptionUart(void)
{
  if (etatTestUart.receptionArmee != 0U)
  {
    return;
  }

  if (HAL_UART_Receive_IT(&huart7, &etatTestUart.octetRecu, 1U) == HAL_OK)
  {
    etatTestUart.receptionArmee = 1U;
  }
}

static void TraiterOctetRecu(uint8_t octetRecu)
{
  if ((octetRecu == '\r') || (octetRecu == '\n'))
  {
    if (etatTestUart.indexReception == 0U)
    {
      return;
    }

    etatTestUart.bufferReception[etatTestUart.indexReception] = '\0';
    strncpy(etatTestUart.dernierMessage, etatTestUart.bufferReception, sizeof(etatTestUart.dernierMessage) - 1U);
    etatTestUart.dernierMessage[sizeof(etatTestUart.dernierMessage) - 1U] = '\0';
    etatTestUart.nbMessagesRecus++;
    etatTestUart.messageRecuPret = 1U;
    etatTestUart.indexReception = 0U;
    memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
    return;
  }

  if (etatTestUart.indexReception >= (TAILLE_BUFFER_RECEPTION - 1U))
  {
    etatTestUart.indexReception = 0U;
    memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
  }

  etatTestUart.bufferReception[etatTestUart.indexReception] = (char)octetRecu;
  etatTestUart.indexReception++;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != UART7)
  {
    return;
  }

  etatTestUart.receptionArmee = 0U;

  TraiterOctetRecu(etatTestUart.octetRecu);
  DemarrerReceptionUart();
}
