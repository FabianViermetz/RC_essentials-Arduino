#include "Arduino.h"
#include "config.h"
#include "input.h"


#if (defined PWM_IN) && (defined PPM_OUT)

#define MAX_PULSE 2100
#define MIN_PULSE 900
#define PULSE_PRESET 1500

volatile uint32_t cur_micros = 0;
volatile uint32_t cur_state, last_state, change;
volatile uint16_t pcint_last_change[PWM_IN_CHANNELS];
volatile int16_t pwm[PWM_IN_CHANNELS], temp_pwm[PWM_IN_CHANNELS];

void setup_PWM() {
#if PWM_IN_CHANNELS > 16
#error : More than 16 Channels not supported!
#endif
  // setting all pins as inputs except A3 as our PPM pin as output
  DDRD = B00000000; /*D0~D7*/
  DDRC = B00000000; /*A0~A7*/

  /* following code is setting up the interrupts. essential for this project */

  // enable general Pin Change Interrupts
  PCICR |= (1 << PCIE2);             /*D0~D7*/
#if CHANNELS > 8
  PCICR |= (1 << PCIE0);             /*A0~A7*/
#endif

  // enable pins to trigger interrupts. up to D0~D7 + A0~A7 = 16 Channels
  for (uint8_t i = 0 ; (i < PWM_IN_CHANNELS) && (i <  8); i++) PCMSK2 |= (1 <<  i);             /*D0~D7*/
  for (uint8_t i = 8 ; i < PWM_IN_CHANNELS; i++)               PCMSK0 |= (1 << (i - 8));        /*A0~A7*/

  for (uint8_t i = 0 ; i < PWM_IN_CHANNELS; i++) pwm[i] = PULSE_PRESET;
}


void ISR_function() {
  static int16_t temp_pulse;
  cur_state = (PIND | (PINC << 8)); // reads current state of all pins
  change = cur_state ^ last_state;               // compares with last values
  last_state = cur_state;                         // updating last values with the new ones

  for (uint8_t i = 0 ; i < PWM_IN_CHANNELS; i++) {
    if ((change >> i) & 1UL == 1) {
      temp_pulse = (cur_micros - pcint_last_change[i]);
      if ((temp_pulse < MAX_PULSE) && (temp_pulse > MIN_PULSE)) {
        temp_pwm[i] = temp_pulse;
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
#if PWM_IN_CHANNELS > 8
ISR(PCINT0_vect) {
  cur_micros = micros();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#endif


#endif // PPM_OUT
