#include "Arduino.h"
#define CHANNELS 8

#define MAX_PULSE 2100
#define MIN_PULSE 900
#define PULSE_PRESET 1500
#define DEADBAND 2000

#define CHANNELS_W_ENDPULSE (CHANNELS+1)

#define PPM_PULSE_LEN 300
#define PPM_END_PULSE 5000
#define PPM_POLARITY HIGH

#if CHANNELS > 16
#error : More than 16 Channels not suported!
#endif

volatile uint32_t cur_micros = 0;
volatile uint32_t cur_state, last_state, change;
volatile uint16_t pcint_last_change[CHANNELS];
volatile int16_t pwm[CHANNELS_W_ENDPULSE], temp_pwm[CHANNELS];

void setup() {
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
  for (uint8_t i = 0 ; (i < CHANNELS) && (i <  8); i++) PCMSK2 |= (1 <<  i);             /*D0~D7*/
  for (uint8_t i = 8 ; i < CHANNELS; i++)               PCMSK0 |= (1 << (i - 8));        /*A0~A7*/

  for (uint8_t i = 0 ; i < CHANNELS; i++) pwm[i] = PULSE_PRESET;
  pwm[CHANNELS_W_ENDPULSE] = PPM_END_PULSE;

  // ppm out
  pinMode(10, OUTPUT);
  digitalWrite(10, !PPM_POLARITY);
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();

  Serial.begin(115200);

}

void loop () {
  // nothing happens here since everything is interrupt trigered
  for (uint8_t i = 2; i< 8; i++){
    Serial.print(pwm[i]);
    Serial.print("\t");
  }
  Serial.println();  
  delay(20);
}

void ISR_function() {
  static int16_t temp_pulse;
  cur_state = (PIND | (PINC << 8)); // reads current state of all pins
  change = cur_state ^ last_state;               // compares with last values
  last_state = cur_state;                         // updating last values with the new ones

  for (uint8_t i = 0 ; i < CHANNELS; i++) {
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

void calculate_pulses() {
  static uint8_t ready_to_kill[CHANNELS];
  for ( uint8_t i = 0; i < CHANNELS; i++) {
    if ((abs(temp_pwm[i] - pwm[i]) > DEADBAND) && (ready_to_kill[i])) {
      ready_to_kill[i] = 0;
    }
    else {
      pwm[i] = (temp_pwm[i] >> 1) << 1;
      ready_to_kill[i] = 1;
    }
  }
}


ISR(PCINT2_vect) {
  cur_micros = micros();  // interrupt service routine for port B, D0~D7
  ISR_function();
}
#if CHANNELS > 8
ISR(PCINT0_vect) {
  cur_micros = micros();  // interrupt service routine for port C, A0~A7
  ISR_function();
}
#endif


#if PPM_POLARITY == 1
    #define TOGGLE_REST_PULSE  PINB &= ~(1 << 2)
    #define TOGGLE_BEGIN_PULSE PINB |= (1 << 2)
#else
    #define TOGGLE_REST_PULSE  PINB |= (1 << 2)
    #define TOGGLE_BEGIN_PULSE PINB &= ~(1 << 2)
#endif


#define chanel_number 8  //set the number of chanels
#define default_servo_value 1500  //set the default servo value
#define PPM_FrLen 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 300  //set the pulse length
#define onState 1  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 10  //set PPM signal output pin on the arduino

ISR (TIMER1_COMPA_vect) {
  static uint8_t i = 0;
  TCNT1 = 0;
  if (OCR1A < (PPM_PULSE_LEN << 1)+50) {
    digitalWrite(10, 0);
    OCR1A = (pwm[(i++%CHANNELS_W_ENDPULSE)-1] - PPM_PULSE_LEN) << 1;
    if (i == 0) {
      calculate_pulses();
    }
  }
  else {
    digitalWrite(10, 1);
    OCR1A = PPM_PULSE_LEN<<1;
  }
}



