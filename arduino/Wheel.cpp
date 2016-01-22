#include <math.h>

#include <Arduino.h>

#include "Wheel.h"

#define ENCODER_INCREMENTS_PER_TURN 3072
#define POSITION_DELTA_COUNT (sizeof(mPositionDeltas) / sizeof(mPositionDeltas[0]))

Wheel::Wheel(Motor& motor, RotaryEncoder& encoder, float updateFrequency):
    mMotor(motor),
    mEncoder(encoder),
    mUpdateFrequency(updateFrequency),
    mKP(50), mKI(10), mKD(50),
    mPositionDeltas{0},
    mPositionDeltaIndex(0),
    mPositionDeltaSum(0),
    mPositionDeltaSumTarget(0),
    mPositionDeltaSumIntegratedError(0) {}

void Wheel::updateOdometry() {
    mPositionDeltaIndex = (mPositionDeltaIndex + 1) % POSITION_DELTA_COUNT;
    mPositionDeltaSum -= mPositionDeltas[mPositionDeltaIndex];
    mPositionDeltaSum += mPositionDeltas[mPositionDeltaIndex] = mEncoder.mPosition;
}

float Wheel::getAngularSpeed() const {
    return mPositionDeltaSum
        * mUpdateFrequency
        * (2 * M_PI / ENCODER_INCREMENTS_PER_TURN)
        / POSITION_DELTA_COUNT;
}

void Wheel::setTargetSpeed(float speed) {
    mPositionDeltaSumTarget =
        speed
        / mUpdateFrequency
        / (2 * M_PI / ENCODER_INCREMENTS_PER_TURN)
        * POSITION_DELTA_COUNT;
    mPositionDeltaSumIntegratedError = 0;
}

void Wheel::regulatePower() {
    const int error = mPositionDeltaSumTarget - mPositionDeltaSum;
    mPositionDeltaSumIntegratedError += error;
    mPositionDeltaSumIntegratedError =
        mPositionDeltaSumIntegratedError > 12700 ? 12700 :
        mPositionDeltaSumIntegratedError < -12700 ? -12700 :
        mPositionDeltaSumIntegratedError;
    const int p = mKP * error;
    const int i = mKI * mPositionDeltaSumIntegratedError;
    const int d = mKD * mPositionDeltas[mPositionDeltaIndex];
    const int output = -(p + i - d) / 100;
    mMotor.setPower(
        output > 127 ? 127 :
        output < -127 ? -127 :
        output
    );
}
