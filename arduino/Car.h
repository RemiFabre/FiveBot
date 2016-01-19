#pragma once

#include "Wheel.h"

ISR(PCINT0_vect);
ISR(PCINT1_vect);
ISR(PCINT2_vect);
ISR(TIMER2_COMPB_vect);

class Car {
    Motor mMotors[4];
    RotaryEncoder mEncoders[4];
    Wheel mWheels[4];
    
    void setupSerial();
    void setupUpdateTimer();
    
    void updateWheels();
    
    public:
    
    Car();
    
    /**
     * Dump the state of the car to the Serial com.
     */
    void dump() const;
    
    friend void PCINT0_vect();
    friend void PCINT1_vect();
    friend void PCINT2_vect();
    friend void TIMER2_COMPB_vect();
};
