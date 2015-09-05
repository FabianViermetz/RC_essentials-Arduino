#include "Arduino.h"
#include "input.h"
#include "output.h"

void setup() {
  setup_input();
  setup_output();
  
//  Serial.begin(115200);

}

void loop () {
  // nothing happens here since everything is interrupt trigered
  //  for (uint8_t i = 2; i< 8; i++){
  //    Serial.print(pwm[i]);
  //    Serial.print("\t");
  //  }
  //  Serial.println();
  //  delay(20);
}



