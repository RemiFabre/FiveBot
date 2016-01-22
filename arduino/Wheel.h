#pragma once

#include "Motor.h"
#include "RotaryEncoder.h"

class Wheel {
    Motor& mMotor;
    RotaryEncoder& mEncoder;
    
    /**
     * Frequency (in Hertz) between consecutive calls to #updateOdometry().
     */
    const int mUpdateFrequency;
    
    /**
     * PID coefficients.
     */
    int mKP, mKI, mKD;
    
    /**
     * Circular buffer of the last known changes in the encoder's absolute position.
     * Must be sampled at a regular interval.
     */
    int mPositionDeltas[8];
    
    /**
     * Index of the latest value written in #mPositionDeltas.
     */
    int mPositionDeltaIndex;
    
    /**
     * Sum of #mPositionDeltas.
     */
    int mPositionDeltaSum;
    
    /**
     * Desired sum (speed).
     */
    int mPositionDeltaSumTarget;
    
    /**
     * Cumulated error between #mPositionDeltaSum and #mPositionDeltaSumTarget.
     */
    int mPositionDeltaSumIntegratedError;
    
    public:
    
    Wheel(Motor& motor, RotaryEncoder& encoder, float updateFrequency);
    
    /**
     * Computes the speed from the history of encoder positions.
     * Resets the encoder position as appropriate.
     * Should be called at the frequency given in the constructor.
     */
    void updateOdometry();
    
    /**
     * Computes the signed speed of the wheel in radians per second.
     */
    float getAngularSpeed() const;
    
    /**
     * Sets the speed target for the PID.
     * @param speed Target angular speed in radians per second.
     */
    void setTargetSpeed(float speed);
    
    /**
     * Regulate the power applied to the motor by the PID.
     */
    void regulatePower();
};
