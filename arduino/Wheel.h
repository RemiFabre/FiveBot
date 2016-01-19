#pragma once

#include "Motor.h"
#include "RotaryEncoder.h"

class Wheel {
    Motor& mMotor;
    RotaryEncoder& mEncoder;
    
    /**
     * Circular buffer of the last known changes in the encoder's absolute position.
     * Must be sampled at a regular interval.
     */
    int mPositionDeltas[8];
    
    /**
     * Index of the next available spot in #mPositionDeltas.
     */
    int mPositionDeltaIndex;
    
    /**
     * Sum of the values inside #mPositionDeltas.
     */
    int mPositionDeltaSum;
    
    public:
    
    Wheel(Motor& motor, RotaryEncoder& encoder);
    
    /**
     * Computes the speed from the history of encoder positions and regulates the motor's power accordingly.
     * Resets the encoder position as appropriate.
     * Should be called at regular intervals.
     */
    void update();
    
    /**
     * Dumps the state of the wheel to the Serial com.
     */
    void dump() const;
};
