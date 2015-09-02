


void setup_pcint () {
#if defined (PWM_IN)
  #if defined (ATMEGA328)
    PCICR |= (1 << PCIE2);                                                // enables pin-change-interrupts for port D
    for (uint8_t i = 2 ; ((i < CHANNEL_NUMBER + 2) && (i < 8)); i++) {
      PCMSK2 |= (1 << i);                                                 // enables pins to trigger PCInts leaving out D0, D1 as TX, RX
    }
    #if (CHANNEL_NUMBER > 6)
      PCICR |= (1 << PCIE0);                                              // enables pin-change-interrupts for port B
      for (uint8_t i = 0 ; i < CHANNEL_NUMBER - 6; i++) {
        PCMSK0 |= (1 << i);                                               // enables pins to trigger PCInts
      }
    #endif // CANNEL_NUMBER > 6
  #elif defined (ATMEGA2560)
    PCICR |= (1 << PCIE2);                                                // enables pin-change-interrupts for port K
    for (uint8_t i = 0 ; i < CHANNEL_NUMBER; i++) {
      PCMSK2 |= (1 << i);                                                 // enables pins to trigger PCInts
    }
  #endif // ATMEGA328 - ATMEGA2560
#endif // PWM_IN

#if defined (PPM_IN)
  #if defined (ATMEGA328)
    PCICR |= (1 << PCIE2);                                               // enables pin-change-interrupts for port D
    PCMSK2 |= (1 << PPM_IN_PIN);                                         // enables PPM pin to trigger PCInts
  #elif defined (ATMEGA2560)
    PCICR |= (1 << PCIE2);                                               // enables pin-change-interrupts for port K
    PCMSK2 |= (1 << (PPM_IN_PIN - 62));                                  // enables PPM pin to trigger PCInts since it is analog, it needs to be substacted by 62
  #endif // ATMEGA328 - ATMEGA2560
#endif // PPM_IN

  for (uint8_t i = 0; i < CHANNEL_NUMBER; i++) {
  // presets all ppm arrays
    ppm_pwm[i] = PULSE_PRESET;
    ppm_serial[i] = PULSE_PRESET;
  }
}
