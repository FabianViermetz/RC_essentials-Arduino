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

#define DISABLE_INPUT
//#define PWM_IN             // ATM328 only
//#define SBUS_IN            // ATM328 & ATM2560
//#define PPM_IN             // ATM328 & ATM2560


////////////////////////////////////////////
////// choose, your type of rx-output //////
////////////////////////////////////////////

#define DISABLE_OUTPUT
//#define PPM_OUT            // ATM328 only
//#define SBUS_OUT           // ATM328 & ATM2560
//#define PWM_OUT            // ATM328 & ATM2560


/*
 * set the number of yor data-channels. Possible are up to 18 channels.
 * you will get better results with PWM and PPM for 10 or less channels but accuracy is 
 * in general more than enough. (see explanation in *.ino)
 * additionally you can set subtrims (similar to the subtrims in your RC)
 */
#define CHANNELS 8
#define SUBTRIMS {0}


////////////////////////////////////////////
///////// PPM and PWM definitions //////////
////////////////////////////////////////////

/*
 * normal RC give pulselengths of approx. 1000~2000 
 * the MIN and MAX settings musst be slightly bigger than the ones of your RC. 
 * probably set your RC's Endpoints first.
 */
#define MIN_PULSE     900
#define MID_PULSE     1500
#define MAX_PULSE     2100

// polarity is usually HIGH
#define PWM_POLARITY HIGH
#define PPM_POLARITY HIGH



////////////////////////////////////////////
///////////// SBUS definitions /////////////
////////////////////////////////////////////

/*
 * SBUS can be transmitted "fast" or "slow", meaning every 7µs(~140Hz) or every 14µs (~70Hz)
 * in most cases you dont need fast_mode since common PPM and PWM is ~50Hz
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


#endif //CONF_H
