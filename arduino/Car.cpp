#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <Arduino.h>

#include "Car.h"

#define UPDATE_FREQUENCY (1000 / 12.8)


static Car* car;

// long t1,t;

Car::Car(): mMotors {
    Motor( 4,  5),
    Motor( 8,  9),
    Motor(11, 10),
    Motor( 7,  6),
}, mEncoders {
    RotaryEncoder(DDRD, PD2, PD3, PCIE2, PCMSK2, PCINT18, PCINT19), // Pins 2 & 3.
    RotaryEncoder(DDRC, PC0, PC1, PCIE1, PCMSK1, PCINT8, PCINT9), // Pins A0 & A1.
    RotaryEncoder(DDRC, PC2, PC3, PCIE1, PCMSK1, PCINT10, PCINT11), // Pins A2 & A3.
    RotaryEncoder(DDRB, PB4, PB5, PCIE0, PCMSK0, PCINT4, PCINT5), // Pins 12 & 13.
}, mWheels {
    Wheel(mMotors[0], mEncoders[0], UPDATE_FREQUENCY),
    Wheel(mMotors[1], mEncoders[1], UPDATE_FREQUENCY),
    Wheel(mMotors[2], mEncoders[2], UPDATE_FREQUENCY),
    Wheel(mMotors[3], mEncoders[3], UPDATE_FREQUENCY),
}, mPosition {
    0,
    0,
}, mOrientation(
    0
), mOdometryPublisher ("ard_odom", &mOdometryMessage)
{
    setupSerial();
    setupUpdateTimer();
    setSpeed(50, 0, 0);
}

void Car::setupSerial() {
    Serial.begin(2000000);
}

void Car::setupUpdateTimer() {
    // Enable timer 2 compare match B interrupts
    TIMSK2 = _BV(OCIE2B);
    // Set timer 2 to CTC mode (Clear Timer on Compare) with OCR2A
    TCCR2A = _BV(WGM21);
    // Enable the timer with prescaler set to 1024
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
    // Set the delay to 12.8ms (OCR2A * prescaler / F_CPU)
    OCR2A = 200;
}

void Car::timerLoop() {
    updateOdometry();
}


void Car::updateOdometry() {
    // t1 = micros();

    // Update wheel speeds from encoders
    mWheels[0].updateOdometry();
    mWheels[1].updateOdometry();
    mWheels[2].updateOdometry();
    mWheels[3].updateOdometry();
    // Save encoder positions and reset them
    int w[4];
    w[0] = mEncoders[0].mPosition;
    mEncoders[0].mPosition = 0;
    w[1] = mEncoders[1].mPosition;
    mEncoders[1].mPosition = 0;
    w[2] = mEncoders[2].mPosition;
    mEncoders[2].mPosition = 0;
    w[3] = mEncoders[3].mPosition;
    mEncoders[3].mPosition = 0;
    // Update odometry
    mPosition[0] += w[0] + w[1] - w[2] - w[3];
    mPosition[1] += w[0] - w[1] - w[2] + w[3];
    mOrientation += -w[0] + w[1] - w[2] + w[3];
    // Update motor speeds
    mWheels[0].regulatePower();
    mWheels[1].regulatePower();
    mWheels[2].regulatePower();
    mWheels[3].regulatePower();

    // t = micros() - t1;
}

void Car::setSpeed(float vx, float vy, float w) {
    mWheels[0].setTargetSpeed(-vx + vy + w);
    mWheels[1].setTargetSpeed(vx + vy - w);
    mWheels[2].setTargetSpeed(-vx + vy - w);
    mWheels[3].setTargetSpeed(vx + vy + w);
}

void Car::mainLoop() {
    // Transmit the state from time to time
    static int skip = 0;
    skip = (skip + 1) % 255;
    if (not skip) {
        // printEncoderPositions();
        // printEncoderErrors();
        // printMotorPower();
        // printWheelSpeeds();
        // printOdometry();
        // Serial.println();
        // Serial.println(t);
        publishOdometry();
    }
    // Let the ATmega sleep until the next interrupt
    sleep_mode();
}

void Car::printEncoderPositions() const {
    Serial.print("positions");
    for (int i = 0; i < 4; ++i) {
        Serial.print("|");
        Serial.print(mEncoders[i].mPosition);
    }
    Serial.println();
}

void Car::printEncoderErrors() const {
    Serial.print("errors");
    for (int i = 0; i < 4; ++i) {
        Serial.print("|");
        Serial.print(mEncoders[i].getErrorCount());
    }
    Serial.println();
}

void Car::printMotorPower() const {
    Serial.print("power");
    for (int i = 0; i < 4; ++i) {
        Serial.print("|");
        Serial.print(mMotors[i].getPower());
    }
    Serial.println();
}

void Car::printWheelSpeeds() const {
    Serial.print("speeds");
    for (int i = 0; i < 4; ++i) {
        Serial.print("|");
        Serial.print(mWheels[i].getAngularSpeed());
    }
    Serial.println();
}

void Car::printOdometry() const {
    Serial.print("odometry|");
    Serial.print(mPosition[0]);
    Serial.print("|");
    Serial.print(mPosition[1]);
    Serial.print("|");
    Serial.print(mOrientation);
    Serial.println();
}

void Car::publishOdometry(){
    mOdometryMessage.angular.z = mOrientation;
    mOdometryMessage.linear.x = mPosition[0];
    mOdometryMessage.linear.y = mPosition[1];

    mOdometryPublisher.publish(&mOdometryMessage);
}

ISR(PCINT0_vect) {
    const unsigned char port = PINB;
    car->mEncoders[3].update(port);
}

ISR(PCINT1_vect) {
    const unsigned char port = PINC;
    car->mEncoders[1].update(port);
    car->mEncoders[2].update(port);
}

ISR(PCINT2_vect) {
    const unsigned char port = PIND;
    car->mEncoders[0].update(port);
}

ISR(TIMER2_COMPB_vect) {
    car->timerLoop();
}

void setup() {
    car = new Car;
    Serial.println(__FUNCTION__);
}

void loop() {
    car->mainLoop();
}
