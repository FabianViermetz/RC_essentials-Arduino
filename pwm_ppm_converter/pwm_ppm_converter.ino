#include "Arduino.h"
#include "config.h"
#include "input.h"
#include "output.h"


volatile int16_t rx[CHANNELS];

#define PULSE_PRESET 1500

void setup() {
  for (uint8_t i = 0 ; i < CHANNELS; i++) rx[i] = PULSE_PRESET;
  setup_input();
  setup_output();
//  Serial.begin(115200);

}

void loop () {
  read_input();
  write_output();
//    Serial.print(rx[i]); Serial.print("\t");
//  }
//  Serial.println();
//  delay(20);
}



