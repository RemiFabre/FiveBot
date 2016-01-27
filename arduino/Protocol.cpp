#include <Arduino.h>

#include "Protocol.h"

Protocol::Protocol() {
    Serial.begin(115200);
    sendASCII("FiveBot started");
}

bool Protocol::canRead() const {
    return Serial.available() > 0;
}

char Protocol::readEscaped() {
    while (not canRead()) {}
    char c = Serial.read();
    if (c == ESC) {
        while (not canRead()) {}
        c = Serial.read();
        c ^= ESC;
    }
    return c;
}

char Protocol::readStart() {
    while (Serial.read() != STX) {}
    return readEscaped();
}

void Protocol::readEnd() {
    while (Serial.read() != ETX) {}
}

void Protocol::sendStart(char c) {
    Serial.write(STX);
    sendEscaped(c);
}

void Protocol::sendEscaped(char c) {
    if (c == STX || c == ETX || c == ESC) {
        Serial.write(ESC);
        c ^= ESC;
    }
    Serial.write(c);
}

void Protocol::sendEnd() {
    Serial.write(ETX);
}
