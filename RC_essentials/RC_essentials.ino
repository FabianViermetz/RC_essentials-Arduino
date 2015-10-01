/*
  This project is capable of converting common RX-transmission-methods into one another.
  It is possible to convert:

  - PPM         - ATMega328 & ATMega2560
  - PWM         - ATMega328 only
  - SBUS(*)     - ATMega328 & ATMega2560

  (*) SBUS is not tested jet very much, so test this with extra caution first.
  all modes are capable of up to 18 channels.

  since it is a sofware PWM/PPM it uses TIMER1 (16bit) and TIMER2 and might jitter a little.
  To reduce jittering reduce PWM_OUT to at most 10 channels. for PPM and SBUS, the
  channel-number doesnt matter, but for PPM it doesn't make sense to have more than
  10 channels, because the update rate can get very poor above that (bellow 50Hz).

  As mentioned above, SBUS is beta, so please doublecheck, if you are using it.
  In any case you need a logic-level inverter since SBUS normaly is inverted. Like this one:
  http://www.hobbyking.com/hobbyking/store/__24523__ZYX_S_S_BUS_Connection_Cable.html

  I am not responsible for anything, that happens while using this code or parts of it.
  Please keep this in mind.

  Author: Fabian Viermetz

*/

/*
 * FOR INDIVIDUALISATION GO TO CONFIG.H
 */

#include "Arduino.h"
#include "basic_uart.h"
#include "config.h"
#include "input.h"
#include "output.h"
#include "timings.h"

int16_t chan[CHANNELS];

void setup() {
////// dont touch this //
  for (uint8_t i = 0 ; i < CHANNELS; i++) chan[i] = MID_PULSE_OUT;
  setup_output();
  setup_input();
  timing_init();
////////////////////////
  serial_begin(0, 115200);

  /*
  if you want to modify RX-Data, place your setup here
  */
  
}

void loop () {
  static uint32_t temp;
  if (milli_s() - temp > 20) {
    temp = milli_s();
    for (uint8_t i = 0; i < CHANNELS; i++) {
      cli();
      uint16_t temp = chan[i];
      sei();
      serial_print(0, (String)temp); serial_print(0, "\t");
    }
    serial_print(0,"\n");
  }
  /*
  if you want to modify RX-Data, place your Main here
  */
  
}



