#ifndef CONF_H
#define CONF_H


/* 
 * PWM in  - D4~D13, A0~A7  (max 18)
 * SBUS in - D0 (max 16 analog / 2 digital. for more information see https://developer.mbed.org/users/Digixx/notebook/futaba-s-bus-controlled-by-mbed/ )
 * PPM in  - D3
 * 
 * PPM out  - D2  (max 18)
 * SBUS out - D1  (max 16/2)
 * PWM out  - D4~D13, A0~A7 (max 18)
 */

////////////////////////////////////////////
////// choose, your type of rx-input  ////// 
////////////////////////////////////////////

//#define PWM_IN             // ATM328 only
//#define SBUS_IN            // ATM328 & ATM2560
#define PPM_IN             // ATM328 & ATM2560


////////////////////////////////////////////
////// choose, your type of rx-output //////
////////////////////////////////////////////

//#define PPM_OUT            // ATM328 only
//#define SBUS_OUT           // ATM328 & ATM2560
#define PWM_OUT            // ATM328 & ATM2560


/*
 * set the number of yor data-channels. Possible are up to 18 channels.
 * you will get better results with PWM and PPM for 10 or less channels but accuracy is 
 * in general enough. (see explanation in *.ino)
 * additionally you can set subtrims (similar to the subtrims in your RC)
 * an even number of channels may be required to avoid bugs (just add one unused)
 */
#define CHANNELS 18
#define SUBTRIMS {0}


////////////////////////////////////////////
///////// PPM and PWM definitions //////////
////////////////////////////////////////////

/*
 * normal RC give pulselengths of approx. 1000~2000 
 * the MIN and MAX settings musst be slightly bigger than the ones of your RC. 
 * probably set your RC's Endpoints first, before you change this.
 */
#define MIN_PULSE_IN     1000
#define MID_PULSE_IN     1500
#define MAX_PULSE_IN     2000

/*
 * many servos have have pulseranges much higher than RCs
 * eg. 500~2500 so to be able to get full range, set this
 */
#define MIN_PULSE_OUT     600
#define MID_PULSE_OUT     1500
#define MAX_PULSE_OUT     2400

/*
 * polarity is usually HIGH
 */
#define PWM_POLARITY HIGH
#define PPM_POLARITY HIGH

/*
 * PPM in is always a bit jittery, 
 * that why it is usually good to smooth it a little
 */
#define SMOOTH_PPM_IN


/*
 * PWM_OUT has a dynamic update rate, so if you send only short pulses 
 * you have a higher update rate than with long pulses.
 * some servos might have problems with that, so here you can define 
 * the maximum update rate (standard is 50 Hz)
 *
 * EXTENDED_PWM_OUT also increases possible rates, but also decreases accuracy. 
 * this is always set, if CHANNELS are to high, to sustain more than 50 Hz.
 * for more information see output.cpp -> "PPM_OUT"
 */
 #define MAX_PWM_RATE_HZ 100
 //#define EXTENDED_PWM_OUT


////////////////////////////////////////////
///////////// SBUS definitions /////////////
////////////////////////////////////////////

/*
 * SBUS can be transmitted "fast" or "slow", meaning every 7ms(~140Hz) or every 14ms (~70Hz)
 * in most cases you dont need fast_mode since common PPM and PWM is ~50Hz. This setting only 
 * affects SBUS_OUT
 * 
 * SBUS_PORT defines the serial port, that is used for SBUS. ATMEGA328 only has 1 port,
 * so "0" is the only choice, ATMEGA2560 has 4 ports (0~3).
 */
#define SBUS_FAST_MODE  false
#define SBUS_PORT 0


////////////////////////////////////////////
/////////// timing definitions /////////////
////////////////////////////////////////////

/*
 * enabling following definition will change the millis() and micros()
 * to ensure proper values for timing, use:
 *  
 *  milli_s()
 *  micro_s()
 *  
 *  they will always give the correct time.
 *  
 *  if you use it or not depends on your personal feeling and requirements
 */
 
#define HIGH_PRECESSION










///////////////////////////////////////
//////////   END OF CONFIG ////////////
///////////////////////////////////////

/*
 * dont change anything below here
 */

#if !defined PPM_IN && !defined PWM_IN && !defined SBUS_IN
  #define DISABLE_INPUT
#endif
#if !defined PPM_OUT && !defined PWM_OUT && !defined SBUS_OUT
  #define DISABLE_OUTPUT
#endif

#if defined SBUS_OUT
  #define MIN_PULSE_OUT     0
  #define MID_PULSE_OUT     1023
  #define MAX_PULSE_OUT     2047
#endif

#if defined SBUS_IN
  #define MIN_PULSE_IN     0
  #define MID_PULSE_IN     1023
  #define MAX_PULSE_IN     2047
#endif


/*
 * following code asks the compiler, which chip it is compiling for
 * eg. used for Serial since ATMega2560 has 4 ports in total (ATM328 only 1)
 */
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328P__)
  #define ATMEGA328
#elif defined (__AVR_ATmega32U4__) || defined (TEENSY20)
  #define PROMICRO
#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1281__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
  #define ATMEGA2560
#endif


#endif //CONF_H
