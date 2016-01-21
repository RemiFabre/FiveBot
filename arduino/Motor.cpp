#include <Arduino.h>

#include "Motor.h"

Motor::Motor(unsigned char directionPin, unsigned char powerPin): mDirectionPin(directionPin), mPowerPin(powerPin), mDirection(-1), mPower(0) {
    pinMode(directionPin, OUTPUT);
    pinMode(powerPin, OUTPUT);
    setPower(0);
}

void Motor::setPower(signed char power) {
    const unsigned char direction = power >= 0 ? HIGH : LOW;
    if (direction != mDirection)
        digitalWrite(mDirectionPin, mDirection = direction);
    analogWrite(mPowerPin, mPower = (direction * 2 - 1) * 2 * power);
}
