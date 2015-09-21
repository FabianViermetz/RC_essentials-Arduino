#include "Arduino.h"
#include "timings.h"
#include "input.h"
#include "output.h"
#include "config.h"

void timing_init() {
#if defined HIGH_PRECESSION
/*
 * this sets the TIMER/COUNTER0 - prescalar from 64 to 8.
 * therefore you need to devide millis() and micros() by 8
 * simply use milli_s() and micro_s() (see "timings.h")
 */
  bitSet(TCCR0B, CS01);
  bitClear(TCCR0B, CS00);
#endif // HIGH_PRECESSION

  /*
   * the following code initialises TIMER/COUNTER2 as kind of a watchdog timer, so input and 
   * output are not depending from the looptime
   */
  bitSet(TCCR2B, CS20);
  bitSet(TCCR2B, CS21);
  bitSet(TIMSK2, TOIE2);
}

ISR(TIMER2_OVF_vect) {
  sei();
  process_input();
  process_output();
}
