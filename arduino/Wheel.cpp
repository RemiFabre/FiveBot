#include <math.h>

#include <Arduino.h>

#include "Wheel.h"

#define ENCODER_INCREMENTS_PER_TURN 3072
#define POSITION_DELTA_COUNT (sizeof(mPositionDeltas) / sizeof(mPositionDeltas[0]))
#define INCREMENTS_TO_RADIANS (2 * M_PI / ENCODER_INCREMENTS_PER_TURN)
#define FIXED_POINT_SHIFT 7

Wheel::Wheel(Motor& motor, RotaryEncoder& encoder, float updateFrequency):
    mMotor(motor),
    mEncoder(encoder),
    mUpdateFrequency(updateFrequency),
    mKP(15), mKI(1), mKD(0),
    mPositionDeltas{0},
    mPositionDeltaIndex(0),
    mPositionDeltaSum(0),
    mPositionDeltaSumTarget(0),
    mPositionDeltaSumIntegratedError(0),
    mBypassPID(false) {}

void Wheel::updateOdometry() {
    mPositionDeltaIndex = (mPositionDeltaIndex + 1) % POSITION_DELTA_COUNT;
    mPositionDeltaSum -= mPositionDeltas[mPositionDeltaIndex];
    mPositionDeltaSum += mPositionDeltas[mPositionDeltaIndex] = mEncoder.mPosition;
}

float Wheel::getAngularSpeed() const {
    return mPositionDeltaSum
        * mUpdateFrequency
        * INCREMENTS_TO_RADIANS
        / POSITION_DELTA_COUNT;
}

void Wheel::setTargetSpeed(float speed) {
    mPositionDeltaSumTarget =
        speed
        * POSITION_DELTA_COUNT
        / INCREMENTS_TO_RADIANS
        / mUpdateFrequency;
    mPositionDeltaSumIntegratedError = 0;
    if (mBypassPID)
        mMotor.setPower(speed);
}

void Wheel::regulatePower() {
    if (mBypassPID)
        return;
    const int error = mPositionDeltaSum - mPositionDeltaSumTarget;
    mPositionDeltaSumIntegratedError += error;
    mPositionDeltaSumIntegratedError =
        mPositionDeltaSumIntegratedError > (127 << FIXED_POINT_SHIFT) ? (127 << FIXED_POINT_SHIFT) :
        mPositionDeltaSumIntegratedError < (-127 << FIXED_POINT_SHIFT) ? (-127 << FIXED_POINT_SHIFT) :
        mPositionDeltaSumIntegratedError;
    const int p = mKP * error;
    const int i = mKI * mPositionDeltaSumIntegratedError;
    const int d = mKD * mPositionDeltas[mPositionDeltaIndex];
    const int output = (p + i + d) >> FIXED_POINT_SHIFT;
    mMotor.setPower(
        output > 127 ? 127 :
        output < -127 ? -127 :
        output
    );
}
