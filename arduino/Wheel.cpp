#include <Arduino.h>

#include "Wheel.h"

#define ENCODER_INCREMENTS_PER_TURN 3072

Wheel::Wheel(Motor& motor, RotaryEncoder& encoder): mMotor(motor), mEncoder(encoder), mPositionDeltas{0}, mPositionDeltaIndex(0), mPositionDeltaSum(0) {}

void Wheel::update() {
    mPositionDeltaSum -= mPositionDeltas[mPositionDeltaIndex];
    mPositionDeltaSum += mPositionDeltas[mPositionDeltaIndex] = mEncoder.mPosition;
    mEncoder.mPosition = 0;
    if (++mPositionDeltaIndex == sizeof(mPositionDeltas) / sizeof(mPositionDeltas[0]))
        mPositionDeltaIndex = 0;
}

void Wheel::dump() const {
    Serial.print("speed: ");
    Serial.print(mPositionDeltaSum);
    Serial.print(' ');
    mEncoder.dump();
}
