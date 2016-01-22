#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>
#include <PID.h>
// #include <mapper.h>


/***** Constants definitions *****/

#define BAUDRATE 2000000

#define SPEED_HIST_SIZE (1 << 7)
#define NEXT_SPEED_INDEX(i) ((i + 1) & (SPEED_HIST_SIZE - 1))
#define ENC_RATIO 3072
#define FACTOR 100

#define PID_OFF


/***** Structures definitions *****/

enum { M1, M2, M3, M4, MOTOR_COUNT };

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
    int direction_hist[SPEED_HIST_SIZE];
    int direction_sum, direction_index;
} MOTOR_STATE[MOTOR_COUNT] = {0};

PID MOTOR_PID[MOTOR_COUNT] = {
    PID(
        &MOTOR_STATE[0].speed,
        &MOTOR_STATE[0].cmd,
        &MOTOR_STATE[0].target_speed,
        &MOTOR_STATE[0].error,
        2, 1, 3,
        DIRECT, 
        FACTOR
    ),
    PID(
        &MOTOR_STATE[1].speed,
        &MOTOR_STATE[1].cmd,
        &MOTOR_STATE[1].target_speed,
        &MOTOR_STATE[1].error,
        2, 1, 3,
        DIRECT,
        FACTOR
    ),
    PID(
        &MOTOR_STATE[2].speed,
        &MOTOR_STATE[2].cmd,
        &MOTOR_STATE[2].target_speed,
        &MOTOR_STATE[2].error,
        2, 1, 3,
        DIRECT,
        FACTOR
    ),
    PID(
        &MOTOR_STATE[3].speed,
        &MOTOR_STATE[3].cmd,
        &MOTOR_STATE[3].target_speed,
        &MOTOR_STATE[0].error,
        2, 1, 3,
        DIRECT,
        FACTOR
    ),
};


inline void readEncoder(int motor, char port) {
    const char a = true and port & _BV(MOTOR_PIN[motor].enc_a);
    const char b = true and port & _BV(MOTOR_PIN[motor].enc_b);
    const char phase = (a ^ b) | b << 1;
    const int direction = ((phase - MOTOR_STATE[motor].encoder_phase + 5) & 3) - 1;
    if(direction == 0) // For encoders 2 & 3 on the same interrupt port  
        return;
    if(direction != 2)
        MOTOR_STATE[motor].direction = direction;
    else
        ++MOTOR_STATE[motor].err_nb;
    MOTOR_STATE[motor].encoder_phase = phase;
    MOTOR_STATE[motor].position += MOTOR_STATE[motor].direction;
}

void computeEncSpeed(int motor){
    auto& m(MOTOR_STATE[motor]);
    m.direction_sum -= m.direction_hist[m.direction_index];
    m.direction_sum += (m.direction_hist[m.direction_index] = m.direction);
    m.direction_index = NEXT_SPEED_INDEX(m.direction_index);
    m.speed =  m.direction_sum ;
}

ISR(PCINT0_vect) {
    readEncoder(M4, PINB);
}

ISR(PCINT1_vect) {
    auto port = PINC;
    readEncoder(M2, port);
    readEncoder(M3, port);
}

ISR(PCINT2_vect) {
    readEncoder(M1, PIND);
}

ISR(TIMER2_COMPB_vect){
    for(int motor = 0; motor < 4; ++motor) {  
        computeEncSpeed(motor);
        MOTOR_STATE[motor].direction = 0;
    }
}

void setMotorSpeed(int motor, int v) {
    MOTOR_STATE[motor].target_speed = v;
    #ifdef PID_ON
    MOTOR_PID[motor].Compute();
    #else
    MOTOR_STATE[motor].cmd = v;
    #endif  
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
    // Enable Pin Change Interrupt
    PCICR = _BV(PCIE0)|_BV(PCIE1)|_BV(PCIE2);

    // Enable interrupt on pins 12-13
    PCMSK0 = _BV(PCINT4)|_BV(PCINT5);
    // Enable interrupt on pins A0-A3
    PCMSK1 = _BV(PCINT8)|_BV(PCINT9)|_BV(PCINT10)|_BV(PCINT11);
    // Enable interrupt on pins 2-3
    PCMSK2 = _BV(PCINT18)|_BV(PCINT19);

    // Enable interrupts globally
    sei();
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

void setupTimer(){
    //Set interrupt on compare match
    // Mode 4, CTC on OCR1A
    // set prescaler to 1024 and start the timer
    TIMSK2 = _BV(OCIE2B);
    TCCR2A = _BV(WGM21);
    TCCR2B = /*_BV(WGM12) | */_BV(CS22) | _BV(CS21) | _BV(CS20);
    //Timer interrupt every 12.8ms
    OCR2A = 200;
}

void debugSetMotorsForward() {
    const char speed = 127;
    setMotorSpeed(M1, speed);
    setMotorSpeed(M2, speed);
    setMotorSpeed(M3, -speed);
    setMotorSpeed(M4, -speed);
}

void setup() {
    setupSerial();
    setupMotorPins();
    setupPID();
    setupTimer();
    setupInterrupts();
    
    debugSetMotorsForward();
}

void printEncoders() {
    Serial.print("wheels");
    for(int i = 0; i < 4; ++i) {
        Serial.print(" ");
        Serial.print(MOTOR_STATE[i].position);
    }
    Serial.println();
}

void printEncoderErrors() {
    Serial.print("errors");
    for(int i = 0; i < 4; ++i) {
        Serial.print(" ");
        Serial.print(MOTOR_STATE[i].err_nb);
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

void printSpeeds() {
    Serial.print("speeds");
    for(int i = 0; i < 4; ++i) {    
        Serial.print(" ");
        Serial.print(MOTOR_STATE[i].speed);
    }
    Serial.println();
}


void send_Serial(){

    unsigned short head_1 = 0xAA;
    unsigned short head_2 = 0xFF;


}

void loop() {
    static int skip = 0;
    skip = (skip + 1) % 1000;
    if (!skip) {
        printEncoders();
        printEncoderErrors();
        printOdometry();
        printSpeeds();
        Serial.println();
    }
}
