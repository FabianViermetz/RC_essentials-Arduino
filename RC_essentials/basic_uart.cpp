#include "Arduino.h"
#include "basic_uart.h"
#include "config.h"

#if defined SBUS_IN || defined SBUS_OUT

#define RING_SIZE 96          // size of TX_buffer and RX_buffer

volatile unsigned char TX_ring[RING_SIZE];
volatile unsigned char RX_ring[RING_SIZE];
volatile uint8_t TX_head;
volatile uint8_t RX_head;
volatile uint8_t TX_tail;
volatile uint8_t RX_tail;


void serial_begin(uint32_t baudrate) {
  uint8_t h = ((F_CPU  / 4 / baudrate - 1) / 2) >> 8;
  uint8_t l = ((F_CPU  / 4 / baudrate - 1) / 2);
  cli();
  UCSR0A  = (1 << U2X0); UBRR0H = h; UBRR0L = l; UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  sei(); // activate interrupts
}

uint8_t serial_available() {
  if (RX_head >= RX_tail) return (RX_head - RX_tail);
  else                    return ((RING_SIZE - RX_tail) + RX_head);
}

byte serial_read() {
  byte temp = RX_ring[RX_tail];
  if (++RX_tail == RING_SIZE) RX_tail = 0;
  return temp;
}

void serial_write(byte data) {
  TX_ring[TX_head] = data;
  if (++TX_head == RING_SIZE) TX_head = 0;
}

void serial_activate() {
  UCSR0B |= (1 << UDRIE0);
}

ISR(USART_RX_vect) {
  RX_ring[RX_head] = UDR0;
  if (++RX_head == RING_SIZE) RX_head = 0;
  if (RX_head == RX_tail) {
    if (++RX_tail == RING_SIZE) RX_tail = 0;
  }
}

ISR(USART_UDRE_vect) {
  if (TX_tail != TX_head) {
    UDR0 = TX_ring[TX_tail];
    if (++TX_tail == RING_SIZE) TX_tail = 0;
  }
  else {
    UCSR0B &= ~(1 << UDRIE0);
  }
}

#endif // SBUS_IN || defined SBUS_OUT
