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
#define PPM_END_PULSE 4000
//#define PPM_POLARITY HIGH

volatile uint16_t sequence_len;

void setup_output() {
  sequence_len = max((CHANNELS*MAX_PULSE) + PPM_END_PULSE, PPM_FRAME_LEN);
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

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

ISR (TIMER1_COMPA_vect) {
  static uint16_t rest = PPM_FRAME_LEN;
  static uint8_t i = 0, cur_chan;
  static bool state;
  TCNT1 = 0;
  if (state) {
    PORTD |= (1 << PPM_OUT_PIN);
    if (cur_chan < CHANNELS) {
      OCR1A = (rx[cur_chan] - PPM_PULSE_LEN) << 1;
      sei();
      rest = sequence_len - rx[cur_chan++];
    }
    else {
      OCR1A = (rest - PPM_PULSE_LEN) << 1;
      sei();
      cur_chan = 0;
      rest = sequence_len;
    }
    state = false;
  }
  else {
    PORTD &= ~(1 << PPM_OUT_PIN);
    OCR1A = PPM_PULSE_LEN << 1;
    sei();
    state = true;
  }
}


#endif // PPM_OUT

/////////////////////////////////////////////// SBUS ///////////////////////////////////////////////////////////

#if defined (SBUS_OUT)

void setup_output() {
}

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

#endif // SBUS_OUT

/////////////////////////////////////////////// PWM ///////////////////////////////////////////////////////////

#if defined PWM_OUT

                 //////////////////////// EXTENDED_PWM_OUT ////////////////////////

#if defined EXTENDED_PWM_OUT 

//volatile uint8_t flag=1;
volatile uint16_t ocr[CHANNELS];
volatile uint8_t fall[CHANNELS/2];
volatile uint16_t sequence_len, endpulse;

void setup_output() {
  sequence_len = max((CHANNELS/2)*(MAX_PULSE+10), 20000);
  
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

void process_output() {
  // nothing to do here, everythin is interrupt-triggered
}

void ISR_function_out_2() {
  sei();
  //if (flag) {
    endpulse = sequence_len;
    for (uint8_t i = 0; i < CHANNELS; i +=2) {
      static uint16_t temp[2];
      cli();
      temp[0] = rx[i]; temp[1] = rx[i+1];
      sei();
      temp[0] = constrain(temp[0], MIN_PULSE, MAX_PULSE); temp[1] = constrain(temp[1], MIN_PULSE, MAX_PULSE);
      if (temp[0] < temp[1]) { ocr[i] = temp[0]<<1; ocr[i+1] = max(temp[1] - temp[0], 10)<<1; fall[i>>1] = i; }
      else                   { ocr[i] = temp[1]<<1; ocr[i+1] = max(temp[0] - temp[1], 10)<<1; fall[i>>1] = i+1; }
      endpulse -= (ocr[i]+ocr[i+1]);
    }
    //flag = 0;
  //}
}

void ISR_function_out() {
  static uint8_t i = 0;
  if (i >= CHANNELS) {
    PORTD=0;
    PORTB=0;
    PORTC=0;
    OCR1A = endpulse;
    i = 0;
    ISR_function_out_2();
  }
  if (i < 6) {
    if (!(i%2)) {
      //if (!i) flag = 1;
      PORTB=0;
      PORTC=0;
      PORTD = ((1 << (i+2)) | (1 << (i+3)));
      OCR1A = ocr[i];
      i++;
    } else {
      PORTD &= ~(1 << (fall[i>>1]+2));
      OCR1A = ocr[i];
      i++;
    }
  }
  else if (i < 12) {
    if (!(i%2)) {
      PORTD=0;
      PORTC=0;
      PORTB = ((1 << (i-6)) | (1 << (i-5)));
      OCR1A = ocr[i];
      i++;
    } else {
      PORTB &= ~(1 << (fall[i>>1]-6));
      OCR1A = ocr[i];
      i++;
    }
  }
  else {  // i > 12
    if (!(i%2)) {
      PORTD=0;
      PORTB=0;
      PORTC = ((1 << (i-12)) | (1 << (i-11)));
      OCR1A = ocr[i];
      i++;
    } else {
      PORTC &= ~(1 << (fall[i>>1]-12));
      OCR1A = ocr[i];
      i++;
    }
  }
}

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  ISR_function_out();
}

                 ////////////////////// not EXTENDED_PWM_OUT //////////////////////
#else 

#if CHANNELS > 9
  #error : CHANNEL NUMBER only supported in EXTENDED PWM MODE
#endif

volatile uint16_t sequence_len;

void setup_output() {
  sequence_len = max(CHANNELS*2110, 20000);
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
    OCR1A = endPulse << 1;
    endPulse = sequence_len;
    i = 0;
  } else {
    if ( i < 6 ) {
      PORTD = (1 << i+2);
      PORTB = 0;
      PORTC = 0;
      endPulse -= rx[i];
      OCR1A = rx[i++] << 1;
    }
    else if ( i < 12 ) {
      PORTB = (1 << (i - 6));
      PORTD = 0;
      endPulse -= rx[i];
      OCR1A = rx[i++] << 1;
    }
    else {
      PORTC = (1 << (i - 12));
      PORTD = 0;
      endPulse -= rx[i];
      OCR1A = rx[i++] << 1;
    }
  }
}

#endif // EXTENDED_PWM_OUT

#endif // PWM_OUT
