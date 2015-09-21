#include "Arduino.h"
#include "basic_uart.h"
#include "config.h"

// low level serial configuration
#define RING_SIZE 96          // size of TX_buffer and RX_buffer
#if defined ATMEGA328
  #define NUM_UART 1
#elif defined ATMEGA2560
  #define NUM_UART 3
#endif

// init of important variables
volatile unsigned char TX_ring[NUM_UART][RING_SIZE];
volatile unsigned char RX_ring[NUM_UART][RING_SIZE];
volatile uint8_t TX_head[NUM_UART];
volatile uint8_t RX_head[NUM_UART];
volatile uint8_t TX_tail[NUM_UART];
volatile uint8_t RX_tail[NUM_UART];

void serial_print(uint8_t port, String output) {
  for (uint8_t i=0; i<output.length(); i++){
    serial_write(port, output[i]);
  }
  serial_activate(port);
}

void serial_println(uint8_t port, String output) {
  for (uint8_t i=0; i<output.length(); i++){
    serial_write(port, output[i]);
  }
  serial_write(port, '\n');
  serial_activate(port);
}

byte serial_read(uint8_t port) {
  byte temp = RX_ring[port][RX_tail[port]];
  if (++RX_tail[port] == RING_SIZE) RX_tail[port] = 0;
  return temp;
}

void serial_write(uint8_t port, byte data) {
  TX_ring[port][TX_head[port]] = data;
  if (++TX_head[port] == RING_SIZE) TX_head[port] = 0;
}

void serial_begin(uint8_t port, uint32_t baudrate) {
  uint8_t h = ((F_CPU  / 4 / baudrate - 1) / 2) >> 8;
  uint8_t l = ((F_CPU  / 4 / baudrate - 1) / 2);
  cli();
  bitSet(UCSR0C, USBS0); bitSet(UCSR0C, UPM01);       // enable 2 stop bits and even parity
  switch (port) {
    case 0: UCSR0A  = (1 << U2X0); UBRR0H = h; UBRR0L = l; UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); bitSet(UCSR0C, USBS0); bitSet(UCSR0C, UPM01); break;
#if defined ATMEGA2560
    case 1: UCSR1A  = (1 << U2X1); UBRR1H = h; UBRR1L = l; UCSR1B |= (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1); bitSet(UCSR1C, USBS1); bitSet(UCSR1C, UPM11); break;
    case 2: UCSR2A  = (1 << U2X2); UBRR2H = h; UBRR2L = l; UCSR2B |= (1 << RXEN2) | (1 << TXEN2) | (1 << RXCIE2); bitSet(UCSR2C, USBS2); bitSet(UCSR2C, UPM21); break;
    case 3: UCSR3A  = (1 << U2X3); UBRR3H = h; UBRR3L = l; UCSR3B |= (1 << RXEN3) | (1 << TXEN3) | (1 << RXCIE3); bitSet(UCSR3C, USBS3); bitSet(UCSR3C, UPM31); break;
#endif
    default: break;
  }
  sei(); // activate interrupts
}

uint8_t serial_available(uint8_t port) {
  if (RX_head[port] >= RX_tail[port]) return (RX_head[port] - RX_tail[port]);
  else                                return ((RING_SIZE - RX_tail[port]) + RX_head[port]);
}

void serial_activate(uint8_t port) {
  switch (port) {
    case 0: UCSR0B |= (1 << UDRIE0); break;
  #if defined ATMEGA2560
    case 1: UCSR1B |= (1 << UDRIE1); break;
    case 2: UCSR2B |= (1 << UDRIE2); break;
    case 3: UCSR3B |= (1 << UDRIE3); break;
  #endif
  }
}


#if defined ATMEGA328
ISR(USART_RX_vect) {
#elif defined ATMEGA2560
ISR(USART0_RX_vect) {
#endif
  RX_ring[0][RX_head[0]] = UDR0;
  if (++RX_head[0] == RING_SIZE) RX_head[0] = 0;
  if (RX_head[0] == RX_tail[0]) {
    if (++RX_tail[0] == RING_SIZE) RX_tail[0] = 0;
  }
}

#if defined ATMEGA328
ISR(USART_UDRE_vect) {
#elif defined ATMEGA2560
ISR(USART0_UDRE_vect) {
#endif
  if (TX_tail[0] != TX_head[0]) {
    UDR0 = TX_ring[0][TX_tail[0]];
    if (++TX_tail[0] == RING_SIZE) TX_tail[0] = 0;
  }
  else {
    UCSR0B &= ~(1 << UDRIE0);
  }
}


#if defined ATMEGA2560

ISR(USART1_RX_vect) {
  RX_ring[1][RX_head[1]] = UDR1;
  if (++RX_head[1] == RING_SIZE) RX_head[1] = 0;
  if (RX_head[1] == RX_tail[1]) {
    if (++RX_tail[1] == RING_SIZE) RX_tail[1] = 0;
  }
}


ISR(USART2_RX_vect) {
  RX_ring[2][RX_head[2]] = UDR2;
  if (++RX_head[2] == RING_SIZE) RX_head[2] = 0;
  if (RX_head[2] == RX_tail[2]) {
    if (++RX_tail[2] == RING_SIZE) RX_tail[2] = 0;
  }
}


ISR(USART3_RX_vect) {
  RX_ring[3][RX_head[3]] = UDR3;
  if (++RX_head[3] == RING_SIZE) RX_head[3] = 0;
  if (RX_head[3] == RX_tail[3]) {
    if (++RX_tail[3] == RING_SIZE) RX_tail[3] = 0;
  }
}


ISR(USART1_UDRE_vect) {
  if (TX_tail[1] != TX_head[1]) {
    UDR1 = TX_ring[1][TX_tail[1]];
    if (++TX_tail[1] == RING_SIZE) TX_tail[1] = 0;
  }
  else {
    UCSR1B &= ~(1 << UDRIE1);
  }
}


ISR(USART2_UDRE_vect) {
  if (TX_tail[2] != TX_head[2]) {
    UDR2 = TX_ring[2][TX_tail[2]];
    if (++TX_tail[2] == RING_SIZE) TX_tail[2] = 0;
  }
  else {
    UCSR2B &= ~(1 << UDRIE2);
  }
}


ISR(USART3_UDRE_vect) {
  if (TX_tail[3] != TX_head[3]) {
    UDR3 = TX_ring[3][TX_tail[3]];
    if (++TX_tail[3] == RING_SIZE) TX_tail[3] = 0;
  }
  else {
    UCSR3B &= ~(1 << UDRIE3);
  }
}

#endif // ATMEGA2560
