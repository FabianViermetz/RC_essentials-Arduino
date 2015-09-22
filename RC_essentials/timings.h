#ifndef TIMINGS_H
#define TIMINGS_H

/*
 *  this is to set the prescalar of millis() and micros() to 8 from 64
 *  precession is +- 4 microseconds if you leave it and now it is theoretically 0.5 micros.
 *
 *  because of this micros() and millis() give values 8 times higher than real,
 *  so you MUST USE micro_s() and milli_s() instead of micros() and millis()
 */

void timing_init();
unsigned long micro_s();
unsigned long milli_s();

#if defined HIGH_PRECESSION
  #define micro_s() (micros() >> 3)
  #define milli_s() (millis() >> 3)
  #define delay_millis(ms) delay(ms<<3)
  #define delay_micros(mis) delayMicroseconds(mis<<3)
#else
  #define micro_s() micros()
  #define milli_s() millis()
#endif // HIGH_PRECESSION

#if defined PWM_OUT && defined PWM_IN
  #error you can define PWM only as input OR output, never both!
#endif

#endif // TIMINGS_H
