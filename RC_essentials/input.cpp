#include "Arduino.h"
#include "config.h"
#include "input.h"
#include "basic_uart.h"

extern int16_t rx[CHANNELS];
extern int8_t subtrim[CHANNELS];

/////////////////////////////////////////////// PWM ///////////////////////////////////////////////////////////
#if defined (PWM_IN)

volatile uint32_t cur_micros = 0;

void setup_input() {
#if CHANNELS > 18
#error : More than 18 Channels not supported!
#endif
  // setting all pins as inputs except A3 as our PPM pin as output
  DDRD = B00000001; /*D0~D7  without Pin 0 and 1 for serial*/
#if CHANNELS > 6
  DDRB = B11000000; /*D8~D13  without Pin 13 for PPM out*/
#if CHANNELS > 12
  DDRC = B00000001; /*A0~A7*/
#endif
#endif

  /* following code is setting up the interrupts. essential for this project */

  // enable general Pin Change Interrupts
  PCICR |= (1 << PCIE2);             /*D0~D7*/
#if CHANNELS > 6
  PCICR |= (1 << PCIE0);             /*D8~D13*/
#if CHANNELS > 12
  PCICR |= (1 << PCIE1);             /*D0~A7*/
#endif
#endif

  // enable pins to trigger interrupts. up to 16 Channels
  for (uint8_t i = 0 ; (i < CHANNELS) && (i < 6); i++) PCMSK2 |= (1 <<  i + 2);           /*D2~D7*/
#if CHANNELS > 6
  for (uint8_t i = 6 ; i < CHANNELS && (i < 12); i++) PCMSK0 |= (1 << (i - 6));        /*D8~D12*/
#if CHANNELS > 12
  for (uint8_t i = 12 ; i < CHANNELS && (i < 16); i++) PCMSK1 |= (1 << (i - 12));        /*A0~A4*/
#endif
#endif

}


void  process_input() {
  // nothing to do here, everythin is interrupt-triggered
}


void ISR_function() {
  static uint32_t cur_state, last_state, change;
  static uint32_t pcint_last_change[CHANNELS];
  static uint16_t temp_pulse;
  cur_state = (PIND >> 2);
#if CHANNELS > 6
  cur_state |= (PINB << 6);
#if CHANNELS > 12
  cur_state |= (PINC << 12);  // reads current state of all pins
#endif
#endif
  sei();
  change = cur_state ^ last_state;               // compares with last values
  last_state = cur_state;                         // updating last values with the new ones

  for (uint8_t i = 0 ; i < CHANNELS; i++) {
    if ((change >> i) & 0x000001 == 1) {
      temp_pulse = (cur_micros - pcint_last_change[i]);
      if ((temp_pulse < MAX_PULSE) && (temp_pulse > MIN_PULSE)) {
        temp_pulse += subtrim[i];
        cli();
        rx[i] = temp_pulse;
        sei();
      }
      else {
        pcint_last_change[i] = cur_micros;
      }
    }
  }
}


ISR(PCINT2_vect) {
  cur_micros = micros();  // interrupt service routine for port B, D0~D7
  ISR_function();
}
#if CHANNELS > 6
ISR(PCINT0_vect) {
  cur_micros = micros();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#if CHANNELS > 12
ISR(PCINT1_vect) {
  cur_micros = micros();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#endif
#endif


#endif // PWM_IN

/////////////////////////////////////////////// SBUS ///////////////////////////////////////////////////////////
#if defined (SBUS_IN)

#define SBUS_MID_OFFSET 988

void setup_input() {
  serial_begin(100000);
}

void process_input() {
#define SBUS_SYNCBYTE 0xF0
  static uint16_t sbus[25] = {0}, sbus_rx[18];
  static uint8_t sbusIndex = 0;
  while (serial_available()) {
    int val = serial_read();
    if (sbusIndex == 0 && val != SBUS_SYNCBYTE)
      continue;
    sbus[sbusIndex++] = val;
    if (sbusIndex >= 25) {
      sbusIndex = 0;
      sbus_rx[0]  = ((sbus[1] | sbus[2] << 8) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[1]  = ((sbus[2] >> 3 | sbus[3] << 5) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[2]  = ((sbus[3] >> 6 | sbus[4] << 2 | sbus[5] << 10) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[3]  = ((sbus[5] >> 1 | sbus[6] << 7) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[4]  = ((sbus[6] >> 4 | sbus[7] << 4) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[5]  = ((sbus[7] >> 7 | sbus[8] << 1 | sbus[9] << 9) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[6]  = ((sbus[9] >> 2 | sbus[10] << 6) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[7]  = ((sbus[10] >> 5 | sbus[11] << 3) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[8]  = ((sbus[12] | sbus[13] << 8) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[9]  = ((sbus[13] >> 3 | sbus[14] << 5) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[10] = ((sbus[14] >> 6 | sbus[15] << 2 | sbus[16] << 10) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[11] = ((sbus[16] >> 1 | sbus[17] << 7) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[12] = ((sbus[17] >> 4 | sbus[18] << 4) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[13] = ((sbus[18] >> 7 | sbus[19] << 1 | sbus[20] << 9) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[14] = ((sbus[20] >> 2 | sbus[21] << 6) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[15] = ((sbus[21] >> 5 | sbus[22] << 3) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      // now the two Digital-Channels
      sbus_rx[16] = (sbus[23] & 0x0001) ? 2000 : 1000;
      sbus_rx[17] = ((sbus[23] >> 1) & 0x0001) ? 2000 : 1000;
      //      if ((sbus[23]) & 0x0001)       sbus_rx[16] = 2000; else sbus_rx[16] = 1000;
      //      if ((sbus[23] >> 1) & 0x0001)  sbus_rx[17] = 2000; else sbus_rx[17] = 1000;

      for (uint8_t i = 0; i < CHANNELS; i++) {
        sbus_rx[i] += subtrim[i];
        cli();
        rx[i] = sbus_rx[i];
        sei();
      }
    }
  }
}

#endif // SBUS_IN

/////////////////////////////////////////////// PPM ///////////////////////////////////////////////////////////

#if defined PPM_IN

#define MIN_END_PULSE 3000
#define PPM_IN_PIN 0            // D0 - RX

volatile uint32_t cur_micros = 0;

void setup_input() {
  pinMode(PPM_IN_PIN, INPUT);
  PCICR |= (1 << PCIE2);                                               // enables pin-change-interrupts for port D
  PCMSK2 |= (1 << PPM_IN_PIN);                                         // enables PPM pin to trigger PCInts

}

void  process_input() {
  // nothing to do here, everything is interrupt-triggered
}

volatile uint8_t ppm_state = 0;
volatile uint8_t channel_counter = 0;

void ISR_function_in() {
  static uint32_t pulse_start, cur_pulse;
  static uint8_t i = 0, cur_state;
  cur_state = (PIND & 0x01);
  sei();
  if (!cur_state) { // falling edge
    // with every falling (or rising) edge the current signal ends and a new signal starts
    cur_pulse = cur_micros - pulse_start;
    pulse_start = cur_micros;

    // if the endpulse is detected counter is reseted
    if (cur_pulse >= MIN_END_PULSE) i = 0;
    else if ((cur_pulse > MIN_PULSE) && (cur_pulse < MAX_PULSE)) {
      cur_pulse += subtrim[i];
      cli();
      rx[i++] = cur_pulse;
      sei();
    }
  }
}


ISR(PCINT2_vect) {                  // interrupt service routine for port D
  cur_micros = micros();
  ISR_function_in();
}

#endif // PPM_IN
