#ifndef UART_H
#define UART_H

void serial_begin(uint32_t baudrate);
uint8_t serial_available();
byte serial_read();
void serial_write(byte data);
void serial_activate();

#endif // UART_H
