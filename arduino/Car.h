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
    
    /**
     * Sets up the Serial com.
     */
    void setupSerial();
    
    /**
     * Sets up the timer used to trigger calls to #update().
     */
    void setupUpdateTimer();
    
    /**
     * Loop function called from a timer interrupt vector.
     * Must be quick.
     * Executes at a regular interval.
     */
    void timerLoop();
    
    public:
    
    Car();
    
    /**
     * Loop function called by the main program.
     * Can be slow and can use the Serial com.
     */
    void mainLoop();
    
    /**
     * Interrupt vectors need to call private functions.
     */
    friend void PCINT0_vect();
    friend void PCINT1_vect();
    friend void PCINT2_vect();
    friend void TIMER2_COMPB_vect();
};
