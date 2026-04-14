#ifndef TEST_UART_H
#define TEST_UART_H

#include "stdint.h"

void TestUart_Initialiser(void);
uint8_t TestUart_EnvoyerMessage(const char *message);
uint8_t TestUart_MessageRecuEstPret(void);
uint8_t TestUart_RecupererDernierMessageRecu(char *message, uint16_t tailleMessage);
void TestUart_AcquitterDernierMessageRecu(void);

#endif /* TEST_UART_H */
