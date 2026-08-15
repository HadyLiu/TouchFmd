#ifndef SOFT_UART_H
#define SOFT_UART_H

void SoftUart_Init(void);
void SoftUart_PutChar(unsigned char ch);
void SoftUart_PutStr(const char* s);
void SoftUart_PutU16(unsigned int v);

#endif
