#pragma once

#include "Protocol.h"
#include "Wheel.h"

ISR(PCINT0_vect);
ISR(PCINT1_vect);
ISR(PCINT2_vect);
ISR(TIMER2_COMPB_vect);

class Car {
    Protocol mCom;
    Motor mMotors[4];
    RotaryEncoder mEncoders[4];
    Wheel mWheels[4];
    float mPosition[2];
    float mOrientation;
    
    /**
     * Sets up the timer used to trigger calls to #update().
     */
    void setupUpdateTimer();
    
    /**
     * Updates the encoders and the odometry of the car.
     * Loop function called at a regular interval from a timer interrupt vector.
     * Must be quick.
     */
    void updateOdometry();
    
    /**
     * Sets the speed of each wheel to make the car move.
     * @param vx speed forward/backward in rad/s
     * @param vy speed sideways in rad/s
     * @param w rotation speed in rad/s
     */
    void setSpeed(float vx, float vy, float w);
    
    public:
    
    Car();
    
    /**
     * Loop function called by the main program.
     * Can be slow and can use the serial com.
     */
    void loop();
    
    /**
     * Publishes the position and orientation of the car.
     */
    void publishOdometry();
    
    /**
     * Publishes the speed of the wheels.
     */
    void publishWheels();
    
    /**
     * Interprets the commands given on the serial com.
     */
    void readCommand();
    
    /**
     * Interrupt vectors need to call private functions.
     */
    friend void PCINT0_vect();
    friend void PCINT1_vect();
    friend void PCINT2_vect();
    friend void TIMER2_COMPB_vect();
};
