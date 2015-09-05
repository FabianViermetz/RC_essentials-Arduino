#include "Arduino.h"
#include "config.h"
#include "output.h"


#if defined PPM_OUT

#define PPM_FRAME_LEN 22500
#define PPM_PULSE_LEN 300
#define PPM_END_PULSE 5000
#define PPM_POLARITY HIGH

#if defined PWM_IN
extern int16_t pwm[PWM_IN_CHANNELS];
#endif

void setup_output() {
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
  static uint16_t rest = PPM_FRAME_LEN;
  static uint8_t i = 0, cur_chan;
  static bool state;
  TCNT1 = 0;
  if (state) {
    digitalWrite(PPM_OUT_PIN, !PPM_POLARITY);
    if (cur_chan >= PWM_IN_CHANNELS) {
      OCR1A = rest - PPM_PULSE_LEN;
      cur_chan = 0;
      rest = PPM_FRAME_LEN;
    }
    else {
      OCR1A = (pwm[cur_chan] - PPM_PULSE_LEN) << 1;
      rest = PPM_FRAME_LEN - pwm[cur_chan++];
    }
    state = false;
  }
  else {
    digitalWrite(PPM_OUT_PIN, PPM_POLARITY);
    OCR1A = PPM_PULSE_LEN << 1;
    state = true;
  }
}


#endif // PPM_OUT
