#include "Arduino.h"
#include "config.h"
#include "output.h"
#include "basic_uart.h"

extern int16_t rx[CHANNELS];

/////////////////////////////////////////////// PPM ///////////////////////////////////////////////////////////

#if defined (PPM_OUT)

#define PPM_OUT_PIN 1        // D1 - TX
#define PPM_FRAME_LEN 22500
#define PPM_PULSE_LEN 300
#define PPM_END_PULSE 5000
//#define PPM_POLARITY HIGH

void setup_output() {
  // ppm out
  pinMode(PPM_OUT_PIN, OUTPUT);
  digitalWrite(PPM_OUT_PIN, LOW);
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void write_output() {
  // nothing to do here, everythin is interrupt-triggered
}

ISR (TIMER1_COMPA_vect) {
  static uint16_t rest = PPM_FRAME_LEN;
  static uint8_t i = 0, cur_chan;
  static bool state;
  TCNT1 = 0;
  if (state) {
    PORTD |= (1 << PPM_OUT_PIN);
    if (cur_chan < CHANNELS && (cur_chan < 8 /*ppm is only capable of 8 channels*/ )) {
      OCR1A = (rx[cur_chan] - PPM_PULSE_LEN) << 1;
      rest = PPM_FRAME_LEN - rx[cur_chan++];
    }
    else {
      OCR1A = rest - PPM_PULSE_LEN;
      cur_chan = 0;
      rest = PPM_FRAME_LEN;
    }
    state = false;
  }
  else {
    PORTD &= ~(1 << PPM_OUT_PIN);
    OCR1A = PPM_PULSE_LEN << 1;
    state = true;
  }
}


#endif // PPM_OUT

/////////////////////////////////////////////// SBUS ///////////////////////////////////////////////////////////

#if defined (SBUS_OUT)

void setup_output() {
}

void write_output() {
  // nothing to do here, everythin is interrupt-triggered
}

#endif // SBUS_OUT

/////////////////////////////////////////////// PWM ///////////////////////////////////////////////////////////

#if defined PWM_OUT

                 //////////////////////// EXTENDED_PWM_OUT ////////////////////////

#if defined EXTENDED_PWM_OUT 

#define PWM_FRAME_LEN 20000
#define MIN_PULSE 500
#define MAX_PULSE (PART_FRAME_LENGTH-50)

void setup_output() {
  DDRD |= B11111100; // D2~D7
  DDRB |= B00111111; // D8~D13
  DDRC |= B00111111; // A0~A5
  
  cli();
  TCCR1A = 0;                      // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;                     // compare match register, change this
  TCCR1B |= (1 << WGM12);          // turn on CTC mode
  TCCR1B |= (1 << CS11);           // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A);         // enable timer compare interrupt
  sei();
}

void write_output() {
  // nothing to do here, everythin is interrupt-triggered
}

void ISR_function_out() {
#define PART_FRAME_LENGTH (PWM_FRAME_LEN/(CHANNELS/2))
  static uint8_t state = 0, first_to_fall, second_to_fall;
  static uint8_t i = 0;
  static uint16_t temp[2], dif;
  sei();
  if (i < 6) {
    if (state == 0) {
      PORTD |= (1 << (i+2)) | (1 << (i+3));
//      if (rx[i] == rx[i+1]) {
//        state = 2; 
//        temp[1] = rx[i];
//        first_to_fall = i;
//        second_to_fall = i+1;
//        OCR1A = temp[1] << 1;
//      } else {
        if (rx[i] < rx[i + 1]) { // rx[i] kleiner
          temp[0] = rx[i];
          temp[1] = rx[i+1]; 
          first_to_fall = i;
          second_to_fall = i+1;
        } else {                  // rx[i+1] kleiner
          temp[0] = rx[i+1];
          temp[1] = rx[i];
          first_to_fall = i+1;
          second_to_fall = i;
        }
        uint16_t dif = temp[1] - temp[0];
//        if (dif < 6) {
//          temp[1] -= (dif>>1); 
//          state = 1;                         // skip state 1
//          OCR1A = temp[1] << 1;
//        } else if (dif < 12) { 
//          temp[0] -= ((12-dif)>>1); 
//          temp[1] += ((12-dif)>>1);
//          OCR1A = temp[0] << 1;
//        }
        state++;
//      }
    }
    else if (state == 1) {
      PORTD &= ~(1 << (first_to_fall+2));
      OCR1A = dif << 1;
      state++;
    }
    else if (state == 2) {
      PORTD &= ~(1 << (first_to_fall+2)) & ~(1 << (second_to_fall+2));
      OCR1A = (PART_FRAME_LENGTH - temp[1]) << 1;
      state = 0;
      i +=2;
    }
  }
  else if (i < 12) {
    if (state == 0) {
      PORTB |= (1 << (i-6)) | (1 << (i-5));
//      if (rx[i] == rx[i+1]) {
//        state = 2; 
//        temp[1] = rx[i];
//        first_to_fall = i;
//        second_to_fall = i+1;
//        OCR1A = temp[1] << 1;
//      } else {
        if (rx[i] < rx[i + 1]) { // rx[i] kleiner
          temp[0] = rx[i];
          temp[1] = rx[i+1]; 
          first_to_fall = i;
          second_to_fall = i+1;
        } else {                  // rx[i+1] kleiner
          temp[0] = rx[i+1];
          temp[1] = rx[i];
          first_to_fall = i+1;
          second_to_fall = i;
        }
        uint16_t dif = temp[1] - temp[0];
//        if (dif < 6) {
//          temp[1] -= (dif>>1); 
//          state = 1;                         // skip state 1
//          OCR1A = temp[1] << 1;
//        } else if (dif < 12) { 
//          temp[0] -= ((12-dif)>>1); 
//          temp[1] += ((12-dif)>>1);
//          OCR1A = temp[0] << 1;
//        }
        state++;
//      }
    }
    else if (state == 1) {
      PORTB &= ~(1 << (first_to_fall-6));
      OCR1A = dif << 1;
      state++;
    }
    else if (state == 2) {
      PORTB &= ~(1 << (first_to_fall-6)) & ~(1 << (second_to_fall-6));
      OCR1A = (PART_FRAME_LENGTH - temp[1]) << 1;
      state = 0;
      i +=2;
    }
  }
  else {  // i > 12
    if (state == 0) {
      PORTC |= (1 << (i-12)) | (1 << (i-11));
      if (rx[i] == rx[i+1]) {
        state = 2; 
        temp[1] = rx[i];
        first_to_fall = i;
        second_to_fall = i+1;
        OCR1A = temp[1] << 1;
      } else {
        if (rx[i] < rx[i + 1]) { // rx[i] kleiner
          temp[0] = rx[i];
          temp[1] = rx[i+1]; 
          first_to_fall = i;
          second_to_fall = i+1;
        } else {                  // rx[i+1] kleiner
          temp[0] = rx[i+1];
          temp[1] = rx[i];
          first_to_fall = i+1;
          second_to_fall = i;
        }
        uint16_t dif = temp[1] - temp[0];
//        if (dif < 6) {
//          temp[1] -= (dif>>1); 
//          state = 1;                         // skip state 1
//          OCR1A = temp[1] << 1;
//        } else if (dif < 12) { 
//          temp[0] -= ((12-dif)>>1); 
//          temp[1] += ((12-dif)>>1);
//          OCR1A = temp[0] << 1;
//        }
        state++;
      }
    }
    else if (state == 1) {
      PORTC &= ~(1 << (first_to_fall-12));
      OCR1A = dif << 1;
      state++;
    }
    else if (state == 2) {
      PORTC &= ~(1 << (first_to_fall-12)) & ~(1 << (second_to_fall-12));
      OCR1A = (PART_FRAME_LENGTH - temp[1]) << 1;
      state = 0;
      i +=2;
    }
  }
  if (i >= CHANNELS) i = 0;
}

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  sei();
  ISR_function_out();
}

                 ////////////////////// not EXTENDED_PWM_OUT //////////////////////
#else 


#define ENDPULSE 4000

void setup_output() {
  DDRD |= B11111100; // D2~D7
  DDRB |= B00111111; // D8~D13
  DDRC |= B00111111; // DA0~A5
  
  cli();
  TCCR1A = 0;                      // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;                     // compare match register, change this
  TCCR1B |= (1 << WGM12);          // turn on CTC mode
  TCCR1B |= (1 << CS11);           // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A);         // enable timer compare interrupt
  sei();
}

void write_output() {
  // nothing to do here, everythin is interrupt-triggered
}

ISR(TIMER1_COMPA_vect) {
  static uint8_t i = 0;
  static uint16_t pwm_sum;
  TCNT1 = 0;
  if (i >= CHANNELS) {
    PORTD = 0;
    PORTB = 0;
    OCR1A = ENDPULSE;
    i = 0;
    pwm_sum = 0;
  } else {
    if ( i < 6 ) {
      PORTD = (1 << i+2);
      PORTB = 0;
      OCR1A = rx[i++] << 1;
    }
    else if ( i < 12 ) {
      PORTB = (1 << (i - 6));
      PORTD = 0;
      OCR1A = rx[i++] << 1;
    }
    else {
      PORTB = (1 << (i - 12));
      PORTD = 0;
      OCR1A = rx[i++] << 1;
    }
  }
}

#endif // EXTENDED_PWM_OUT

#endif // PWM_OUT
