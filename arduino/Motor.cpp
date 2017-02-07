#include <Arduino.h>

#include "Motor.h"

Motor::Motor(unsigned char directionPin, unsigned char powerPin, unsigned char motorID): mDirectionPin(directionPin), mPowerPin(powerPin), mMotorID(motorID), mDirection(-1), mPower(0) {
    pinMode(directionPin, OUTPUT);
    pinMode(powerPin, OUTPUT);
    setPower(0);
}

void Motor::setPower(signed char power) {
    const unsigned char direction = power >= 0;
    if (direction != mDirection)
        digitalWrite(mDirectionPin, mDirection = direction);
    // Simple approach :
    //analogWrite(mPowerPin, mPower = (direction * 2 - 1) * 2 * power);

    power = abs(power);
    // OCR0* is 8bit, OCR1* is 16 bit... Union?
    if (mMotorID == 1) {
      OCR0A = power;
    } else if (mMotorID == 2) {
      OCR1B = power;
    } else if (mMotorID == 3) {
      OCR1A = power;
    } else if (mMotorID == 4) {
      OCR0B = power;
    }
}
