
#define CHANNELS 8

#define MAX_PULSE 2100
#define MIN_PULSE 900
#define PULSE_PRESET 1500
#define DEADBAND 20

#if CHANNELS > 16
#error : More than 16 Channels not suported!
#endif

volatile uint32_t cur_micros = 0;
volatile uint32_t cur_state, last_state, change;
volatile uint32_t pcint_last_change[CHANNELS];
volatile int16_t pwm[CHANNELS];

void setup() {
  // setting all pins as inputs except A3 as our PPM pin as output
  DDRD = B00000000; /*D0~D7*/
  DDRB = B00000000; /*D8~D13*/
  DDRC = B00001000; /*A0~A7*/

  /* following code is setting up the interrupts. essential for this project */

  // enable general Pin Change Interrupts
  PCICR |= (1 << PCIE2);             /*D0~D7*/
#if CHANNELS > 8
  PCICR |= (1 << PCIE0);             /*D8~D13*/
#if CHANNELS > 14
  PCICR |= (1 << PCIE1);             /*A0~A7*/
#endif
#endif

  // enable pins to trigger interrupts. up to D0~D13 + A0~A1 = 16 Channels
  for (uint8_t i = 0 ; ((i < CHANNELS) && (i <  8)); i++) PCMSK2 |= (1 <<  i);             /*D0~D7*/
  for (uint8_t i = 8 ; ((i < CHANNELS) && (i < 14)); i++) PCMSK0 |= (1 << (i - 8));        /*D8~D13*/
  for (uint8_t i = 14 ; ((i < CHANNELS) && (i < 16)); i++) PCMSK1 |= (1 << (i - 14));      /*A0~A1*/

  for (uint8_t i = 0 ; i < CHANNELS; i++) pwm[i] = PULSE_PRESET;
}

void loop () {
  // nothing happens here since everything is interrupt trigered
}

void ISR_function() {
  static int16_t temp_pulse;
  static uint8_t ready_to_kill[CHANNELS];
  cur_state = (PIND | (PINB << 8) | (PINC << 14)); // reads current state of all pins
  change = cur_state ^ last_state;               // compares with last values
  last_state = cur_state;                         // updating last values with the new ones

  for (uint8_t i = 0 ; i < CHANNELS; i++) {
    if ((change >> i) & 1UL == 1) {
      temp_pulse = (cur_micros - pcint_last_change[i]);
      if ((temp_pulse < MAX_PULSE) && (temp_pulse > MIN_PULSE)) {
        if ((abs(temp_pulse - pwm[i]) > DEADBAND) && (ready_to_kill[i])) {
          ready_to_kill[i] = 0;
          continue;
        }
        else {
          pwm[i] = (temp_pulse >> 1) << 1;
          ready_to_kill[i] = 1;
        }
      }
      else {
        pcint_last_change[i] = cur_micros;
      }
    }
  }
}

#define COMPUTE_PCINT {cur_micros = micros(); sei(); ISR_function();}

ISR(PCINT2_vect) COMPUTE_PCINT        // interrupt service routine for port B, D0~D7
#if CHANNELS > 8
ISR(PCINT1_vect) COMPUTE_PCINT        // interrupt service routine for port D, D8~D13
#if CHANNELS > 14
ISR(PCINT0_vect) COMPUTE_PCINT        // interrupt service routine for port C, A0~A1
#endif
#endif
