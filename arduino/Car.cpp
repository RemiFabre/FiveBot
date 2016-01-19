#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <Arduino.h>

#include "Car.h"

static Car* car;

Car::Car(): mMotors {
    Motor( 4,  5),
    Motor( 8,  9),
    Motor(11, 10),
    Motor( 7,  6),
}, mEncoders {
    RotaryEncoder(PIND, PD2, PD3, PCIE2, PCMSK2, PCINT18, PCINT19), // Pins 2 & 3.
    RotaryEncoder(PINC, PC0, PC1, PCIE1, PCMSK1, PCINT8, PCINT9), // Pins A0 & A1.
    RotaryEncoder(PINC, PC2, PC3, PCIE1, PCMSK1, PCINT10, PCINT11), // Pins A2 & A3.
    RotaryEncoder(PINB, PB4, PB5, PCIE0, PCMSK0, PCINT4, PCINT5), // Pins 12 & 13.
}, mWheels {
    Wheel(mMotors[0], mEncoders[0]),
    Wheel(mMotors[1], mEncoders[1]),
    Wheel(mMotors[2], mEncoders[2]),
    Wheel(mMotors[3], mEncoders[3]),
} {
    setupSerial();
    setupUpdateTimer();
}

void Car::setupSerial() {
    Serial.begin(2000000);
    Serial.println(__FUNCTION__);
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

void Car::updateWheels() {
    mWheels[0].update();
    mWheels[1].update();
    mWheels[2].update();
    mWheels[3].update();
}

void Car::dump() const {
    mWheels[0].dump();
    mWheels[1].dump();
    mWheels[2].dump();
    mWheels[3].dump();
    Serial.println();
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
    car->updateWheels();
}

// Arduino
void setup() { car = new Car; }
void loop() { car->dump(); sleep_mode(); }
