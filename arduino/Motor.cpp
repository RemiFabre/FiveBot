#include <Arduino.h>

#include "Motor.h"

Motor::Motor(unsigned char directionPin, unsigned char powerPin): mDirectionPin(directionPin), mPowerPin(powerPin), mDirection(-1), mPower(0) {
    setPower(0);
}

void Motor::setPower(signed char power) {
    const unsigned char direction = power >= 0;
    if (direction != mDirection) {
        mDirection = direction;
        digitalWrite(mDirectionPin, mDirection);
    }
    mPower = ((unsigned char)direction & 127) << 1;
    analogWrite(mPowerPin, mPower);
}
