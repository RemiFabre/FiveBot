#pragma once

class Motor {
    const unsigned char
        mDirectionPin, ///< Pin used to choose between forward and reverse.
        mPowerPin; ///< Pin used to give power to the motor.
    unsigned char
        mDirection, ///< State of the direction pin
        mPower; ///< Current power between 0 and 255.
    
    public:
    
    /**
     * Initializes a motor and sets its power to zero.
     * @param directionPin Pin used to choose between forward and reverse.
     * @param powerPin Pin used to give power to the motor.
     */
    Motor(unsigned char directionPin, unsigned char powerPin);
    
    /**
     * Sets the power applied to the motor.
     * @param power Power to apply to the motor between -127 and +127 (controls direction too).
     */
    void setPower(signed char power);
    
    /**
     * Returns the power currently given to the motor.
     */
    inline signed char getPower() const {
        return (mDirection * 2 - 1) * mPower / 2;
    }
};
