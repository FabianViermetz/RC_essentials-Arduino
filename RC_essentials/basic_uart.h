#ifndef UART_H
#define UART_H

/*
 * following code asks the compiler, which chip it is compiling for
 * eg. used for Serial since ATMega2560 has 4 ports in total (ATM328 only 1)
 */
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
  #define ATMEGA328
#elif defined (__AVR_ATmega32U4__) || defined (TEENSY20)
  #define PROMICRO
#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1281__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
  #define ATMEGA2560
#endif

void serial_begin(uint8_t port, uint32_t const baudrate);     // initializes a specific serial port with a specific baudrate
uint8_t serial_available(uint8_t port);                       // gives back the numbers of bytes waiting in the serial Inbox
byte serial_read(uint8_t port);                               // reads bytes from a specific port
void serial_write(uint8_t port, byte data);                   // writes bytes to a specific port
void serial_activate(uint8_t port);                           // activates TX only required after serial_write()
void serial_print(uint8_t port, String output);               // human-readable-output: if you want to print a number, convert it to String, like " serial_print((String)var); "
void serial_println(uint8_t port, String output);             // human-readable-output: if you want to print a number, convert it to String, like " serial_println((String)var); "

#endif // UART_H
