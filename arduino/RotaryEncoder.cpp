#include <avr/io.h>

#include <Arduino.h>

#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(volatile unsigned char& ddr, unsigned char pinA, unsigned char pinB, unsigned char pcie, volatile unsigned char& pcmsk, unsigned char pcintA, unsigned char pcintB): mPinA(pinA), mPinB(pinB), mPhase(0), mDirection(0), mErrors(0), mPosition(0) {
    ddr &= ~_BV(pinA) & ~_BV(pinB);
    PCICR |= _BV(pcie);
    pcmsk |= _BV(pcintA) | _BV(pcintB);
}

void RotaryEncoder::update(unsigned char port) {
    const char
        a = (port >> mPinA) & 1,
        b = (port >> mPinB) & 1,
        phase = (a ^ b) | b << 1,
        direction = ((phase - mPhase + 4 + 1) & 3) - 1;
    mPhase = phase;
    if (direction) {
        if(direction != 2)
            mDirection = direction;
        else
            ++mErrors;
        mPosition += mDirection;
    }
}
