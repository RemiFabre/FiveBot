#include <math.h>

#include "Wheel.h"

#define ENCODER_INCREMENTS_PER_TURN 3072
#define POSITION_DELTA_COUNT (sizeof(mPositionDeltas) / sizeof(mPositionDeltas[0]))

Wheel::Wheel(Motor& motor, RotaryEncoder& encoder): mMotor(motor), mEncoder(encoder), mPositionDeltas{0}, mPositionDeltaIndex(0), mPositionDeltaSum(0) {}

void Wheel::update() {
    mPositionDeltaSum -= mPositionDeltas[mPositionDeltaIndex];
    mPositionDeltaSum += mPositionDeltas[mPositionDeltaIndex] = mEncoder.mPosition;
    if (++mPositionDeltaIndex == POSITION_DELTA_COUNT)
        mPositionDeltaIndex = 0;
}

float Wheel::getAngularSpeed(float updateFrequency) const {
    return mPositionDeltaSum * updateFrequency * (2 * M_PI / ENCODER_INCREMENTS_PER_TURN) / POSITION_DELTA_COUNT;
}
