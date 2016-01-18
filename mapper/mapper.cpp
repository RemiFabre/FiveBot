#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>
#include <PID.h>

enum { M1, M2, M3, M4, MOTOR_COUNT };

#define BAUDRATE 2000000

#define SPEED_HIST_SIZE (1 << 5)
#define NEXT_SPEED_INDEX(i) ((i + 1) & (SPEED_HIST_SIZE - 1))
#define ENC_RATIO 3072

float xyw[] = { 0, 0, 0 };

struct MotorPin {
    unsigned char direction, speed, enc_a, enc_b;
} MOTOR_PIN[MOTOR_COUNT] = {
    {  4,   5, PD2, PD3 }, /*  2,  3 */
    {  8,   9, PC0, PC1 }, /* A0, A1 */
    { 11,  10, PC2, PC3 }, /* A2, A3 */
    {  7,   6, PB4, PB5 }, /* 12, 13 */
};


struct MotorState {
    int position, target_position, speed, target_speed,
        direction, encoder_phase, err_nb,
        error, cmd;
    long time_hist[SPEED_HIST_SIZE];
    int position_hist[SPEED_HIST_SIZE];
    int index;
} MOTOR_STATE[MOTOR_COUNT] = {0};


PID MOTOR_PID[MOTOR_COUNT] = {
    PID(
        &MOTOR_STATE[0].speed,
        &MOTOR_STATE[0].cmd,
        &MOTOR_STATE[0].target_speed,
        &MOTOR_STATE[0].error,
        2, 1, 3,
        DIRECT
    ),
    PID(
        &MOTOR_STATE[1].speed,
        &MOTOR_STATE[1].cmd,
        &MOTOR_STATE[1].target_speed,
        &MOTOR_STATE[1].error,
        2, 1, 3,
        DIRECT
    ),
    PID(
        &MOTOR_STATE[2].speed,
        &MOTOR_STATE[2].cmd,
        &MOTOR_STATE[2].target_speed,
        &MOTOR_STATE[2].error,
        2, 1, 3,
        DIRECT
    ),
    PID(
        &MOTOR_STATE[3].speed,
        &MOTOR_STATE[3].cmd,
        &MOTOR_STATE[3].target_speed,
        &MOTOR_STATE[0].error,
        2, 1, 3,
        DIRECT
    ),
};

inline void readEncoder(int motor, char port) {
    const char a = true and port & _BV(MOTOR_PIN[motor].enc_a);
    const char b = true and port & _BV(MOTOR_PIN[motor].enc_b);
    const char phase = (a ^ b) | b << 1;
    const int direction = ((phase - MOTOR_STATE[motor].encoder_phase + 5) & 3) - 1;
    if(direction != 2)
        MOTOR_STATE[motor].direction = direction;
    else
        ++MOTOR_STATE[motor].err_nb;
    MOTOR_STATE[motor].encoder_phase = phase;
    MOTOR_STATE[motor].position += MOTOR_STATE[motor].direction;
}

ISR(PCINT0_vect){
    readEncoder(M4, PINB);
}

ISR(PCINT1_vect) {
    readEncoder(M2, PINC);
    readEncoder(M3, PINC);
}

ISR(PCINT2_vect) {
    readEncoder(M1, PIND);
}

void setMotorSpeed(int motor, int v) {
    MOTOR_STATE[motor].target_speed = v;
    MOTOR_STATE[motor].cmd = v; // MOTOR_PID[motor].Compute();
    analogWrite(MOTOR_PIN[motor].speed, abs(MOTOR_STATE[motor].cmd));
    digitalWrite(MOTOR_PIN[motor].direction, v < 0 ? LOW : HIGH);
}

void updateOdometryFromEncoders() {
    static float w[] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4; ++i)
        w[i] += MOTOR_STATE[i].position;
    const float R = 9.4 / 2, L1 = 15, L2 = 15;
    xyw[0] += R / 4 * (w[0] + w[1] - w[2] - w[3]);
    xyw[1] += R / 4 * (w[0] - w[1] - w[2] + w[3]);
    xyw[2] += R / 4 / (L1 + L2) * (-w[0] + w[1] - w[2] + w[3]);
    for (int i = 0; i < 4; ++i) {
        int& pos(MOTOR_STATE[i].position);
        const int sign = pos < 0 ? -1 : 1;
        pos = sign * ((sign * pos) % ENC_RATIO);
        w[i] = -pos;
    }
}

void setupMotorPins() {
    // 0 -> input
    // 1 -> output
    DDRC = 0;
    DDRB = 0;
    DDRD |= (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
    DDRD &= ~((1 << DDD2) | (1 << DDD3));
}

void setupInterrupts() {
    // Enable interrupts globally
    sei();
    // Enable Pin Change Interrupt
    PCICR = _BV(PCIE0)|_BV(PCIE1)|_BV(PCIE2);

    // Enable interrupt on pins 12-13
    PCMSK0 = _BV(PCINT4)|_BV(PCINT5);
    // Enable interrupt on pins A0-A3
    PCMSK1 = _BV(PCINT8)|_BV(PCINT9)|_BV(PCINT10)|_BV(PCINT11);
    // Enable interrupt on pins 2-3
    PCMSK2 = _BV(PCINT18)|_BV(PCINT19);
}

void setupSerial() {
    Serial.begin(BAUDRATE);
    Serial.setTimeout(10000);
    Serial.println("SETUP");
}

void setupPID() {
    for(int i = 0; i < MOTOR_COUNT; ++i)
        MOTOR_PID[i].SetMode(AUTOMATIC);
}

void debugSetMotorsForward() {
    const char speed = 50;
    setMotorSpeed(M1, speed);
    setMotorSpeed(M2, speed);
    setMotorSpeed(M3, -speed);
    setMotorSpeed(M4, -speed);
}

void setup() {
    setupSerial();
    setupMotorPins();
    //setupPID();
    setupInterrupts();

    debugSetMotorsForward();
}

void printWheelPositions() {
    Serial.print("wheels");
    for(int i = 0; i < 4; ++i) {
        Serial.print(" ");
        Serial.print(MOTOR_STATE[i].position);
    }
    Serial.println();
}

void printOdometry() {
    updateOdometryFromEncoders();
    Serial.print("xyw");
    for (int i = 0; i < 3; ++i) {
        Serial.print(" ");
        Serial.print(xyw[i]);
    }
    Serial.println();
}

void loop() {
    static int skip = 0;
    skip = (skip + 1) % 10000;
    if (!skip)
        printOdometry();
}
