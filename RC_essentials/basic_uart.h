#ifndef UART_H
#define UART_H

void serial_begin(uint8_t port, uint32_t const baudrate);     // initializes a specific serial port with a specific baudrate
uint8_t serial_available(uint8_t port);                       // gives back the numbers of bytes waiting in the serial Inbox
byte serial_read(uint8_t port);                               // reads bytes from a specific port
void serial_write(uint8_t port, byte data);                   // writes bytes to a specific port
void serial_activate(uint8_t port);                           // activates TX only required after serial_write()
void serial_print(uint8_t port, String output);               // human-readable-output: if you want to print a number, convert it to String, like " serial_print((String)var); "
void serial_println(uint8_t port, String output);             // human-readable-output: if you want to print a number, convert it to String, like " serial_println((String)var); "

#endif // UART_H
