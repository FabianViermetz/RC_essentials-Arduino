#include "Arduino.h"
#include "config.h"
#include "output.h"
#include "basic_uart.h"
#include "timings.h"

extern int16_t chan[CHANNELS];

/////////////////////////////////////////////// PPM ///////////////////////////////////////////////////////////

#if defined (PPM_OUT)

#define PPM_OUT_PIN 2
#define PPM_FRAME_LEN 22500
#define PPM_PULSE_LEN 300
#define PPM_END_PULSE 4000

volatile uint16_t sequence_len;

void setup_output() {
  sequence_len = max((CHANNELS*MAX_PULSE_OUT) + PPM_END_PULSE, PPM_FRAME_LEN);
  pinMode(PPM_OUT_PIN, OUTPUT);
  digitalWrite(PPM_OUT_PIN, !PPM_POLARITY);
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;               // compare match register, dont change this
  TCCR1B |= (1 << WGM12);    // turn on CTC mode
  TCCR1B |= (1 << CS11);     // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A);   // enable timer compare interrupt
  sei();
}

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

ISR (TIMER1_COMPA_vect) {
  static uint16_t rest = PPM_FRAME_LEN;
  static uint8_t i = 0, cur_chan;
  static bool state;
  TCNT1 = 0;
  if (state) {
    digitalWrite(PPM_OUT_PIN, !PPM_POLARITY);
    if (cur_chan < CHANNELS) { // finish the channel-pulse
      OCR1A = (chan[cur_chan] - PPM_PULSE_LEN) << 1;
      sei();
      rest = sequence_len - chan[cur_chan++];
    }
    else {  // give the Endpulse
      OCR1A = (rest - PPM_PULSE_LEN) << 1;
      sei();
      cur_chan = 0;
      rest = sequence_len;
    }
    state = false;
  }
  else { // start new channel pulse
    digitalWrite(PPM_OUT_PIN, PPM_POLARITY);
    OCR1A = PPM_PULSE_LEN << 1;
    sei();
    state = true;
  }
}


#endif // PPM_OUT

/////////////////////////////////////////////// SBUS ///////////////////////////////////////////////////////////

#if defined (SBUS_OUT)

#define SBUS_MID_OFFSET 988
#define SBUS_STARTBYTE 0x0F
#define SBUS_ENDBYTE 0x00

uint16_t sbus_ch[18] = {1023};

void setup_output() {
  serial_begin(SBUS_PORT, 100000);
}

void process_output() {
  static uint32_t last_sbus_out = 0;
  static uint8_t flagbyte = 0;
  if ((milli_s() - last_sbus_out) >= (14 - (7 * SBUS_FAST_MODE))) {  // decide wether you have fast or slow SBUS
    last_sbus_out = milli_s();
    for (uint8_t i = 0; i < CHANNELS; i++) {
      cli();
      sbus_ch[i] = chan[i];         // read out all channel values
      sei();
    }
    /*
     * start the communication and write all channels (first convert them)
     */
    serial_write(SBUS_PORT, SBUS_STARTBYTE);
    serial_write(SBUS_PORT, sbus_ch[0]);                                                               //  chan0 8/11   
    serial_write(SBUS_PORT, ((sbus_ch[0]>>8) & 0x07) | (sbus_ch[1]<<3));                               //  chan0 3/3   chan1 5/11
    serial_write(SBUS_PORT, ((sbus_ch[1]>>5) & 0x3F) | (sbus_ch[2]<<6));                               //  chan1 6/6   chan2 2/11
    serial_write(SBUS_PORT, sbus_ch[2]>>2);                                                            //  chan2 8/9   
    serial_write(SBUS_PORT, ((sbus_ch[2]>>10) & 0x01) | (sbus_ch[3]<<1));                              //  chan2 1/1   chan3 7/11
    serial_write(SBUS_PORT, ((sbus_ch[3]>>7) & 0x0F) | (sbus_ch[4]<<4));                               //  chan3 4/4   chan4 4/11
    serial_write(SBUS_PORT, ((sbus_ch[4]>>4) & 0x7F) | (sbus_ch[5]<<7));                               //  chan4 7/7   chan5 1/11  
    serial_write(SBUS_PORT, sbus_ch[5]>>1);                                                            //  chan5 8/10   
    serial_write(SBUS_PORT, ((sbus_ch[5]>>9) & 0x03) | (sbus_ch[6]<<2));                               //  chan5 2/2   chan6 6/11
    serial_write(SBUS_PORT, ((sbus_ch[6]>>6) & 0x1F) | (sbus_ch[7]<<5));                               //  chan6 5/5   chan7 3/11
    serial_write(SBUS_PORT, sbus_ch[7]>>3);                                                            //  chan7 8/8
    serial_write(SBUS_PORT, sbus_ch[8]);                                                               //  chan8 8/11   
    serial_write(SBUS_PORT, ((sbus_ch[8]>>8) & 0x07) | (sbus_ch[9]<<3));                               //  chan8 3/3   chan9 5/11
    serial_write(SBUS_PORT, ((sbus_ch[9]>>5) & 0x3F) | (sbus_ch[10]<<6));                              //  chan9 6/6  chan10 2/11
    serial_write(SBUS_PORT, sbus_ch[10]>>2);                                                           // chan10 8/9   
    serial_write(SBUS_PORT, ((sbus_ch[10]>>10) & 0x01) | (sbus_ch[11]<<1));                            // chan10 1/1  chan11 7/11
    serial_write(SBUS_PORT, ((sbus_ch[11]>>7) & 0x0F) | (sbus_ch[12]<<4));                             // chan11 4/4  chan12 4/11
    serial_write(SBUS_PORT, ((sbus_ch[12]>>4) & 0x7F) | (sbus_ch[13]<<7));                             // chan12 7/7  chan13 1/11  
    serial_write(SBUS_PORT, sbus_ch[13]>>1);                                                           // chan13 8/10  
    serial_write(SBUS_PORT, ((sbus_ch[13]>>9) & 0x03) | (sbus_ch[14]<<2));                             // chan13 2/2  chan14 6/11
    serial_write(SBUS_PORT, ((sbus_ch[14]>>6) & 0x1F) | (sbus_ch[15]<<5));                             // chan14 5/5  chan15 3/11
    serial_write(SBUS_PORT, sbus_ch[15]>>3);                                                           // chan15 8/8
    if (sbus_ch[16] > MID_PULSE_OUT) bitSet(flagbyte,0); else bitClear(flagbyte,0);                        // chan16 digital
    if (sbus_ch[17] > MID_PULSE_OUT) bitSet(flagbyte,1); else bitClear(flagbyte,1);                        // chan17 digital
    serial_write(SBUS_PORT, flagbyte);
    serial_write(SBUS_PORT, SBUS_ENDBYTE);
    serial_activate(SBUS_PORT);             // activate serial port to finally send the data block
  }
}

#endif // SBUS_OUT

/////////////////////////////////////////////// PWM ///////////////////////////////////////////////////////////

#if defined PWM_OUT

#if (CHANNELS*(MAX_PULSE_OUT+10)) > 20000
#define EXTENDED_PWM_OUT // for more than 10 pwm out channels (up to 18), might not be so accurate
#if CHANNELS%2
#error select even number of channels to avoid complications!
#endif
#endif

                 //////////////////////// EXTENDED_PWM_OUT ////////////////////////

#if defined EXTENDED_PWM_OUT 

volatile uint16_t ocr[CHANNELS];
volatile uint8_t fall[CHANNELS/2];
volatile uint16_t sequence_len, endpulse;

/*
 * Method:
 * to control 18 servos with one 16bit timer, you can just take care of one or max two signals in parallel.
 *   
 *   
 * "lines"||                 |                    |              |              |--endpulse--|                 |
 *        |
 * chans: | _____________     ____________________ ______________ _____________               _____________                             ___ logic PWM_POLARITY
 *2,4,6.. ||             |   |                    |              |             |             |             |
 * (even) ||    chan2    |   |       chan4        |    chan6     |     .....   |             |    chan2    |       and so on                          
 *        ||_____________|___|____________________|______________|_____________|_____________|_____________|_________________________   ___ logic !PPM_POLARITY
 * chans: | _________________ _______________      __________     ______________              _________________                         ___ logic PPM_POLARITY
 *1,3,5.. ||                 |               |    |          |   |              |            |                 |                    
 *(uneven)||      chan1      |     chan3     |    |  chan5   |   |     .....    |            |     chan1       |    and so on                           
 *        ||_________________|_______________|____|__________|___|______________|____________|_________________|_____________________   ___ logic !PPM_POLARITY 
 *                                                                                                                            TIME
 *        
 *        you need one "parallel line" for every 10 channels in order to get ~50Hz update rate, the more lines we have, 
 *        the more problems and race conditions can occure.
 *        
 *        
 */

void setup_output() {
  sequence_len = max((CHANNELS/2)*(MAX_PULSE_OUT+10),(1000/MAX_PWM_RATE_HZ)*1000) << 1;      // servo update rate is dynamic. for standard pulselengths (~2000 max) it is 50Hz+ max (18chans)
  /*
   * setting all output pins
   * and configure TIMER/COUNTER1
   */
  DDRD |= B11110000; // D4~D7
  DDRB |= B00111111; // D8~D13
  DDRC |= B11111111; // A0~A7
  
  cli();
  TCCR1A = 0;                      // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;                     // compare match register, change this
  TCCR1B |= (1 << WGM12);          // turn on CTC mode
  TCCR1B |= (1 << CS11);           // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A);         // enable timer compare interrupt
  sei();
}

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

uint32_t calc_schedule() {
  sei();
  uint32_t time_temp = micro_s();
  endpulse = sequence_len;
  for (uint8_t i = 0; i < CHANNELS; i +=2) {
    static uint16_t temp[2];
    cli();
    temp[0] = chan[i]; temp[1] = chan[i+1];
    sei();
    if (temp[0] < temp[1]) { ocr[i] = temp[0]<<1; ocr[i+1] = max(temp[1] - temp[0], 10)<<1; fall[i>>1] = i; }
    else                   { ocr[i] = temp[1]<<1; ocr[i+1] = max(temp[0] - temp[1], 10)<<1; fall[i>>1] = i+1; }
    endpulse -= (ocr[i]+ocr[i+1]);
  }
  return (micro_s() - time_temp);
}

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  static uint8_t i = 0;
  if (i >= CHANNELS) {
    PORTD=0;
    PORTB=0;
    PORTC=0;
    OCR1A = endpulse;  // all pins are cleared and we wait until the frame (see "sequence_len") is completed
    i = 0;                  // clear channel counter
  }
  if (i < 4) {
    if (!(i%2)) {
      PORTB=0;                                // clear all other pins
      PORTC=0;                                // clear all other pins
      PORTD = ((1 << (i+4)) | (1 << (i+5)));  // writing both parallel channels high
      if (!i) ocr[0] -= calc_schedule()<<1;        // calculate all coming stuff at the beginning, to save as much time, thus precession, as possible
      OCR1A = ocr[i];                         
      i++;
    }
    else {
      PORTD &= ~(1 << (fall[i>>1]+4));  // clearing the shorter of the parallel channels 
      OCR1A = ocr[i];
      i++;
    }
  }
  else if (i < 10) {
    if (!(i%2)) {
      PORTD=0;
      PORTC=0;
      PORTB = ((1 << (i-4)) | (1 << (i-3)));
      OCR1A = ocr[i];
      i++;
    } else {
      PORTB &= ~(1 << (fall[i>>1]-4));
      OCR1A = ocr[i];
      i++;
    }
  }
  else {  // i > 10
    if (!(i%2)) {
      PORTD=0;
      PORTB=0;
      PORTC = ((1 << (i-10)) | (1 << (i-9)));
      OCR1A = ocr[i];
      i++;
    } else {
      PORTC &= ~(1 << (fall[i>>1]-10));
      OCR1A = ocr[i];
      i++;
    }
  }
}

                 ////////////////////// not EXTENDED_PWM_OUT //////////////////////
#else 

volatile uint16_t sequence_len;

/*
 * this mode works exactly as the EXTENDED_PWM but only with one line. 
 * this reduces calculation and increases precession
 */

void setup_output() {
  sequence_len = max(CHANNELS*(MAX_PULSE_OUT+10),(1000/MAX_PWM_RATE_HZ)*1000);
  /*
   * setting all output pins
   * and configure TIMER/COUNTER1
   */
  DDRD |= B11110000; // D4~D7
  DDRB |= B00111111; // D8~D13
  DDRC |= B11111111; // DA0~A7
  
  cli();
  TCCR1A = 0;                      // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;                     // compare match register, change this
  TCCR1B |= (1 << WGM12);          // turn on CTC mode
  TCCR1B |= (1 << CS11);           // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A);         // enable timer compare interrupt
  sei();
}

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

ISR(TIMER1_COMPA_vect) {
  static uint8_t i = 0;
  static uint16_t endPulse;
  TCNT1 = 0;
  if (i >= CHANNELS) {
    PORTD = 0;
    PORTB = 0;
    PORTC = 0;
    OCR1A = endPulse << 1;       // all pins are cleared and we wait until the frame (see "sequence_len") is completed
    endPulse = sequence_len;     // reset endPulse
    i = 0;                       // clear channel counter
  } 
  else if ( i < 4 ) {
    PORTD = (1 << i+4);          // set only the one pin and clear all others
    PORTB = 0;
    PORTC = 0;
    endPulse -= chan[i];           // recalculate endPulse
    OCR1A = chan[i++] << 1;
  }
  else if ( i < 10 ) {
    PORTB = (1 << (i - 4));
    PORTD = 0;
    endPulse -= chan[i];
    OCR1A = chan[i++] << 1;
  }
  else {
    PORTC = (1 << (i - 10));
    PORTD = 0;
    endPulse -= chan[i];
    OCR1A = chan[i++] << 1;
  }
}

#endif // EXTENDED_PWM_OUT

#endif // PWM_OUT

/////////////////////////////////////////////// DISABLE_OUTPUT ///////////////////////////////////////////////////////////

#if defined DISABLE_OUTPUT

void setup_output() {
/*
 * empty, nothing to do here in this case
 */
}

void process_output() {
/*
 * empty, nothing to do here in this case
 */
}

#endif // DISABLE_OUTPUT
