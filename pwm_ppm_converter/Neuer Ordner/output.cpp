#include "Arduino.h"
#include "config.h"
#include "output.h"


#if defined PPM_OUT

#define PPM_PULSE_LEN 300
#define PPM_END_PULSE 5000
#define PPM_POLARITY HIGH

#if defined PWM_IN
extern int16_t pwm[PWM_IN_CHANNELS];
#endif

void setup_PPM() {
  // ppm out
  pinMode(PPM_OUT_PIN, OUTPUT);
  digitalWrite(PPM_OUT_PIN, !PPM_POLARITY);
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}


ISR (TIMER1_COMPA_vect) {
  static uint8_t i = 0;
  TCNT1 = 0;
  if (OCR1A < (PPM_PULSE_LEN << 1) + 50) {
    digitalWrite(PPM_OUT_PIN, !PPM_POLARITY);
    if (i >= PWM_IN_CHANNELS) OCR1A = PPM_END_PULSE;
    OCR1A = (pwm[(i++ % PWM_IN_CHANNELS) - 1] - PPM_PULSE_LEN) << 1;
  }
  else {
    digitalWrite(PPM_OUT_PIN, PPM_POLARITY);
    OCR1A = PPM_PULSE_LEN << 1;
  }
}


#endif // PPM_OUT
