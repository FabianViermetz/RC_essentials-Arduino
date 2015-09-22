#include "Arduino.h"
#include "config.h"
#include "input.h"
#include "basic_uart.h"
#include "timings.h"

extern int16_t rx[CHANNELS];
int8_t subtrim[CHANNELS] = SUBTRIMS;

/////////////////////////////////////////////// PWM ///////////////////////////////////////////////////////////
#if defined (PWM_IN)


volatile uint32_t cur_micros = 0;

void setup_input() {
  /*
   * in this setup the PORT-masks of the ATM328 are set to the correct pinModes (all inpus except TX)
   * below, the interrupts ar set. 
   */
  DDRD = B00000101; /*D0~D7  without Pin 0 and 1 for serial and D2 for PPM out*/
#if CHANNELS > 4
  DDRB = B11000000; /*D8~D13*/
#if CHANNELS > 10
  DDRC = B00000000; /*A0~A7*/
#endif
#endif

  /* following code is setting up the interrupts. essential for this project */

  // enable general Pin Change Interrupts
  PCICR |= (1 << PCIE2);             /*D0~D7*/
#if CHANNELS > 4
  PCICR |= (1 << PCIE0);             /*D8~D13*/
#if CHANNELS > 10
  PCICR |= (1 << PCIE1);             /*D0~A7*/
#endif
#endif

  // enable pins to trigger interrupts. up to 18 Channels
  for (uint8_t i = 0;  (i < CHANNELS) && (i < 4) ; i++) PCMSK2 |= (1 <<  i + 4);           /*D4~D7*/
  for (uint8_t i = 4;  (i < CHANNELS) && (i < 10); i++) PCMSK0 |= (1 << (i - 4));          /*D8~D13*/
  for (uint8_t i = 10; (i < CHANNELS)            ; i++) PCMSK1 |= (1 << (i - 10));         /*A0~A7*/

}


void  process_input() {
  // nothing to do here, everythin is interrupt-triggered
}


void ISR_function() {
  /*
   * executing at every Pin Change Interrupt. this checks which pin(s) changed
   * and calculates the corresponding pulse length
   */
  static uint32_t cur_state, last_state, change;
  static uint32_t pcint_last_change[CHANNELS];
  static uint16_t temp_pulse;
  
// writing the PIN masks into one uint32_t
  cur_state = (PIND >> 4);
#if CHANNELS > 4
  cur_state |= (PINB << 4);
#if CHANNELS > 10
  cur_state |= (PINC << 10);  // reads current state of all pins
#endif
#endif
  sei();
  change = cur_state ^ last_state;                // compares with last values
  last_state = cur_state;                         // updating last values with the new ones

  for (uint8_t i = 0 ; i < CHANNELS; i++) {
    if (((change >> i) & 0x000001) == 1) {
      if (((cur_state >> i) & 0x000001) == PWM_POLARITY) pcint_last_change[i] = cur_micros;    // pulse begin; rising or falling edge depending on PWM_POLARITY
      else {                                                                                   // pulse end
        temp_pulse = (cur_micros - pcint_last_change[i]);  // calculating pulse
        constrain(temp_pulse, MIN_PULSE, MAX_PULSE);
        temp_pulse += subtrim[i];
        cli();
        rx[i] = temp_pulse;
        sei();
      }
    }
  }
}

/*
 * these are the PCINT-routines executed by the controller
 */
ISR(PCINT2_vect) {
  cur_micros = micro_s();  // interrupt service routine for port B, D0~D7
  ISR_function();
}
#if CHANNELS > 6
ISR(PCINT0_vect) {
  cur_micros = micro_s();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#if CHANNELS > 12
ISR(PCINT1_vect) {
  cur_micros = micro_s();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#endif
#endif


#endif // PWM_IN

/////////////////////////////////////////////// SBUS ///////////////////////////////////////////////////////////
#if defined (SBUS_IN)

#define SBUS_MID_OFFSET 988
#define SBUS_STARTBYTE 0x0F
#define SBUS_ENDBYTE 0x00

void setup_input() {
  serial_begin(SBUS_PORT, 100000);
}

void process_input() {
  
  static uint16_t sbus[25] = {0}, sbus_rx[18];
  static uint8_t sbusIndex = 0;
  while (serial_available(SBUS_PORT)) {
    int val = serial_read(SBUS_PORT);
    if (sbusIndex == 0 && val != SBUS_STARTBYTE) continue;  // checking for the start byte
    sbus[sbusIndex++] = val;
    if (sbusIndex >= 25) {
      sbusIndex = 0;
      /*
       * reading and converting SBUS-data into PWM data
       */
      sbus_rx[0]  = ((sbus[1] | sbus[2] << 8) & 0x07FF)                          / 2 + SBUS_MID_OFFSET;
      sbus_rx[1]  = ((sbus[2]  >> 3 | sbus[3] << 5)  & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[2]  = ((sbus[3]  >> 6 | sbus[4] << 2   | sbus[5] << 10) & 0x07FF)  / 2 + SBUS_MID_OFFSET;
      sbus_rx[3]  = ((sbus[5]  >> 1 | sbus[6] << 7)  & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[4]  = ((sbus[6]  >> 4 | sbus[7] << 4)  & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[5]  = ((sbus[7]  >> 7 | sbus[8] << 1   | sbus[9] << 9) & 0x07FF)   / 2 + SBUS_MID_OFFSET;
      sbus_rx[6]  = ((sbus[9]  >> 2 | sbus[10] << 6) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[7]  = ((sbus[10] >> 5 | sbus[11] << 3) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[8]  = ((sbus[12]      | sbus[13] << 8) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[9]  = ((sbus[13] >> 3 | sbus[14] << 5) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[10] = ((sbus[14] >> 6 | sbus[15] << 2  | sbus[16] << 10) & 0x07FF) / 2 + SBUS_MID_OFFSET;
      sbus_rx[11] = ((sbus[16] >> 1 | sbus[17] << 7) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[12] = ((sbus[17] >> 4 | sbus[18] << 4) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[13] = ((sbus[18] >> 7 | sbus[19] << 1  | sbus[20] << 9) & 0x07FF)  / 2 + SBUS_MID_OFFSET;
      sbus_rx[14] = ((sbus[20] >> 2 | sbus[21] << 6) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
      sbus_rx[15] = ((sbus[21] >> 5 | sbus[22] << 3) & 0x07FF)                   / 2 + SBUS_MID_OFFSET;
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
#if defined ATMEGA2560
  #define PPM_IN_PIN A8
#else
  #define PPM_IN_PIN 3
#endif

volatile uint32_t cur_micros = 0;

void setup_input() {
  /*
   * setting up the PPM-IN pin and its Pin Change Interrupt
   */
  pinMode(PPM_IN_PIN, INPUT);
#if defined (ATMEGA2560)
  PCICR |= (1 << PCIE2);                                               // enables pin-change-interrupts for port K
  PCMSK2 |= (1 << (PPM_IN_PIN - 62));                                  // enables PPM pin to trigger PCInts since it is analog, it needs to be substacted by 62
#else
  PCICR |= (1 << PCIE2);                                               // enables pin-change-interrupts for port D
  PCMSK2 |= (1 << PPM_IN_PIN);                                         // enables PPM pin to trigger PCInts
#endif // ATMEGA328 - ATMEGA2560
}


void  process_input() {
  // nothing to do here, everything is interrupt-triggered
}

volatile uint8_t ppm_state = 0;
volatile uint8_t channel_counter = 0;


ISR(PCINT2_vect) {                  // interrupt service routine for port D
  cur_micros = micro_s();
  static uint32_t pulse_start, cur_pulse;
  static uint8_t i = 0, cur_state;
  cur_state = digitalRead(PPM_IN_PIN);
  sei();
  if (cur_state == PPM_POLARITY) { // rising or falling edge depending on PPM_POLARITY
    // with every falling (or rising) edge the current signal ends and a new signal starts
    cur_pulse = cur_micros - pulse_start;
    pulse_start = cur_micros;

    // if the endpulse is detected counter is reseted
    if (cur_pulse >= MIN_END_PULSE) i = 0;
    else {
      constrain(cur_pulse, MIN_PULSE, MAX_PULSE);
      cur_pulse += subtrim[i];
      cli();
      rx[i++] = cur_pulse;
      sei();
    }
  }
}

#endif // PPM_IN

/////////////////////////////////////////////// DISABLE_INPUT ///////////////////////////////////////////////////////////

#if defined DISABLE_INPUT

void setup_input() {
/*
 * empty, nothing to do here in this case
 */
}

void process_input() {
/*
 * empty, nothing to do here in this case
 */
}

#endif // DISABLE_INPUT

