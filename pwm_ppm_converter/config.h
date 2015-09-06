#ifndef CONF_H
#define CONF_H


/* 
  PWM in  - D2~D13, A1~A4  (max 16)
  PPM in  - D0 (max 8)
  SBUS in - D0 (Serial max 16 for more info google it)
  
  PPM out  - D1 (max 8)
  PWM out  - D2~D13, A0~A5 (max 18)
  
  TODO:
  SBUS out - D1 (Serial max 16 for more info google it)
  
  extra: SBUS inverted!!
  
*/
  

//#define PWM_IN
//#define SBUS_IN
#define PPM_IN

//#define PPM_OUT
//#define SBUS_OUT
#define PWM_OUT

//#define EXTENDED_PWM_OUT // not Working !! for more than 10 pwm out channels (up to 18, keep it an even number), might not be so accurate

#define CHANNELS 8
#define PWM_IN_SUBTRIMS {0}


#endif //CONF_H
