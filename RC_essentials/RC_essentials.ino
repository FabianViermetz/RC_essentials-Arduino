#include "Arduino.h"
#include "config.h"
#include "input.h"
#include "output.h"


volatile int16_t rx[CHANNELS];
volatile int8_t subtrim[CHANNELS] = SUBTRIMS;

#define PULSE_PRESET 1500

void setup() {
  cli();
  TCCR2A = 0; // set entire TCCR1 register to 0
  TCCR2B = 0;
  TCCR2B |= (1 << WGM22);  // turn on CTC mode
  TCCR2B |= (1 << CS21);  // 8 prescaler: 0,5 microseconds at 16mhz
  sei();
  
  for (uint8_t i = 0 ; i < CHANNELS; i++) rx[i] = PULSE_PRESET;
  setup_input();
  setup_output();
  Serial.begin(115200);

}

void loop () {
  process_input();
  process_output();
  for (uint8_t i = 0 ; i < CHANNELS; i++) {
    Serial.print(rx[i]); Serial.print("\t");
 }
  Serial.println();
}



