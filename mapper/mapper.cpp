#include <avr/io.h>
#include <avr/interrupt.h>

#include <PID.h>
#include <Arduino.h>



///*********************  DEFINITIONS   *********************///  

#define MVT 0x01
#define ENC 0x02

#define M1 0
#define M2 1
#define M3 2
#define M4 3
 
#define MOTOR_NB 4
#define SPEED_MEASURE_COUNT 30
#define ENC_RATIO 3072

#define NEXT_INDEX(i) ((i+1)%SPEED_MEASURE_COUNT)

#define BAUDRATE 2000000

struct {
    unsigned char direction;
    unsigned char speed;
    unsigned char enc_a;
    unsigned char enc_b;
} 
MOTOR_PIN[MOTOR_NB] = {
    {
        4, 5, PD2, PD3 /*2, 3*/     }
    ,
    {
        8, 9, PC0, PC1 /*A0, A1*/      }
    , 
    {
        11, 10, PC2, PC3/*A2, A3*/      }
    ,
    {
        7, 6, PB4, PB5 /*12, 13*/     }
};


struct {
    int position;
    int target_position;
    int speed;
    int target_speed;
    int direction;
    int encoder_phase;
    int err_nb;
    int error;
    int cmd;
    long time_hist[SPEED_MEASURE_COUNT] = {0};
    int position_hist[SPEED_MEASURE_COUNT] = {0};
    int index = 0;
} 
MOTOR_STATE[MOTOR_NB];


PID MOTOR_PID[MOTOR_NB] = {
    PID(&MOTOR_STATE[0].speed, &MOTOR_STATE[0].cmd, &MOTOR_STATE[0].target_speed, 
        &MOTOR_STATE[0].error,2,1,3,DIRECT),
    PID(&MOTOR_STATE[1].speed, &MOTOR_STATE[1].cmd, &MOTOR_STATE[1].target_speed,
        &MOTOR_STATE[1].error,2,1,3,DIRECT),
    PID(&MOTOR_STATE[2].speed, &MOTOR_STATE[2].cmd, &MOTOR_STATE[2].target_speed,
        &MOTOR_STATE[2].error,2,1,3,DIRECT),
    PID(&MOTOR_STATE[3].speed, &MOTOR_STATE[3].cmd, &MOTOR_STATE[3].target_speed,
        &MOTOR_STATE[0].error,2,1,3,DIRECT)
    };

long delta_t,delta_p;

// PORT B
ISR(PCINT0_vect){ 

    const unsigned char port = PINB;

    int i = M4;

    const unsigned char a = (port & _BV(MOTOR_PIN[i].enc_a) || 0); //digitalRead(MOTOR_PIN[i].enc_a);
    const unsigned char b = (port & _BV(MOTOR_PIN[i].enc_b) || 0); //digitalRead(MOTOR_PIN[i].enc_b);

    const unsigned char phase = (a ^ b) | b << 1;

    // Calcul de la direction
    const int direction = ((phase - MOTOR_STATE[i].encoder_phase + 5) & 3) -1;
    if(direction != 2){
        MOTOR_STATE[i].direction = direction;
    }
    else{
        ++MOTOR_STATE[i].err_nb;
    }
    MOTOR_STATE[i].encoder_phase = phase;
    MOTOR_STATE[i].position += MOTOR_STATE[i].direction;

}

// PORT C
ISR(PCINT1_vect){ 

    const unsigned char port = PINC;

    for (int i = M2; i <= M3; ++i) {

        const unsigned char a = (port & _BV(MOTOR_PIN[i].enc_a) || 0); //digitalRead(MOTOR_PIN[i].enc_a);
        const unsigned char b = (port & _BV(MOTOR_PIN[i].enc_b) || 0); //digitalRead(MOTOR_PIN[i].enc_b);

        const unsigned char phase = (a ^ b) | b << 1;

        // Calcul de la direction
        const int direction = ((phase - MOTOR_STATE[i].encoder_phase + 5) & 3) -1;
        if(direction != 2){
            MOTOR_STATE[i].direction = direction;
        }
        else{
            ++MOTOR_STATE[i].err_nb;
        }
        MOTOR_STATE[i].encoder_phase = phase;
        MOTOR_STATE[i].position += MOTOR_STATE[i].direction;
    }

}

// PORT D
ISR(PCINT2_vect){ 

    int i = M1;

    const unsigned char port = PIND;

    const unsigned char a = (port & _BV(MOTOR_PIN[i].enc_a) || 0); //digitalRead(MOTOR_PIN[i].enc_a);
    const unsigned char b = (port & _BV(MOTOR_PIN[i].enc_b) || 0); //digitalRead(MOTOR_PIN[i].enc_b);

    const unsigned char phase = (a ^ b) | b << 1;

    // Calcul de la direction
    const int direction = ((phase - MOTOR_STATE[i].encoder_phase + 5) & 3) - 1;
    if(direction != 2){
        MOTOR_STATE[i].direction = direction;
    }
    else{
        ++MOTOR_STATE[i].err_nb;
    }
    MOTOR_STATE[i].encoder_phase = phase;
    MOTOR_STATE[i].position += MOTOR_STATE[i].direction;

}


void poll_encoders() {
    static int skip = 0;
    skip = (skip + 1) % 10000;

    for (int i = 0; i < 1; ++i) {

        const int a = (PINB & _BV(MOTOR_PIN[i].enc_a) || 0); //digitalRead(MOTOR_PIN[i].enc_a);
        const int b = (PINB & _BV(MOTOR_PIN[i].enc_b) || 0); //digitalRead(MOTOR_PIN[i].enc_b);

        const int phase = (a ^ b) | b << 1;

        // Calcul de la direction
        const int direction = ((phase - MOTOR_STATE[i].encoder_phase + 5) & 3) -1;
        if(direction != 2){
            MOTOR_STATE[i].direction = direction;
        }
        else{
            ++MOTOR_STATE[i].err_nb;
        }
        MOTOR_STATE[i].encoder_phase = phase;
        MOTOR_STATE[i].position += MOTOR_STATE[i].direction;

        if (!skip) {
            Serial.print(" dir=");
            Serial.print(MOTOR_STATE[i].direction);
            Serial.print(" pos=");
            Serial.print(MOTOR_STATE[i].position);
            Serial.print(" err=");
            Serial.print(MOTOR_STATE[i].err_nb);
            // if (i < 3)
            //     Serial.print(" ");
            // else
                Serial.println();
        }
        
        // Calcul de la vitesse
        /*
        MOTOR_STATE[i].time_hist[MOTOR_STATE[i].index] = micros();
        MOTOR_STATE[i].position_hist[MOTOR_STATE[i].index] = MOTOR_STATE[i].position;
        delta_t = MOTOR_STATE[i].time_hist[MOTOR_STATE[i].index] - MOTOR_STATE[i].time_hist[NEXT_INDEX(MOTOR_STATE[i].index)];
        delta_p = MOTOR_STATE[i].position_hist[MOTOR_STATE[i].index] - MOTOR_STATE[i].position_hist[NEXT_INDEX(MOTOR_STATE[i].index)];
        
        long dp = (((long)delta_p)*1000000*100);
        long dt = (((long)delta_t)*ENC_RATIO);
        MOTOR_STATE[i].speed =  dp / dt;

        // Serial.print("dp : ");
        // Serial.println(dp);
        // Serial.print("dt : ");
        // Serial.println(dt);
        // Serial.print("speed : ");
        if(MOTOR_STATE[M1].speed != 0) Serial.println(MOTOR_STATE[M1].speed);

        
        MOTOR_STATE[i].index = NEXT_INDEX(MOTOR_STATE[i].index);
        */
    }

}


// // Works with PID in POSITION !
// void go_to_angle(int angle){
//     MOTOR_STATE[M1].target_position = angle;
//     MOTOR_PID[M1].Compute();
//     analogWrite(MOTOR_PIN[M1].speed, abs(MOTOR_STATE[M1].cmd));
//     digitalWrite(MOTOR_PIN[M1].direction, (MOTOR_STATE[M1].error < 0) ? LOW : HIGH);

//     // if(MOTOR_STATE[1].error !=0) Serial.println(MOTOR_STATE[M1].error);
// }


// Set speed with PID, v in 10^(-2)tr/s
void set_speed(int motor, int v){
    MOTOR_STATE[motor].target_speed = v;
    // MOTOR_PID[motor].Compute();
    MOTOR_STATE[motor].cmd = v;
   
    analogWrite(MOTOR_PIN[motor].speed, abs(MOTOR_STATE[motor].cmd));
    digitalWrite(MOTOR_PIN[motor].direction, (v < 0) ? LOW : HIGH);

}


void get_vx_vy_w(float w[4], float xyw[4]) {
    const float R = 9.4 / 2, L1 = 15, L2 = 15;
    xyw[0] = R / 4 * (w[0] + w[1] - w[2] - w[3]);
    xyw[1] = R / 4 * (w[0] - w[1] - w[2] + w[3]);
    xyw[2] = R / 4 / (L1 + L2) * (-w[0] + w[1] - w[2] + w[3]);
}

///*********************    SETUP    *********************///  
void setup_motor_pins() {

    // 1 -> output
    // 0 -> input
    DDRB = 0;
    DDRD |= (1 << DDD4)|(1 << DDD5)|(1 << DDD6)|(1 << DDD7);
    DDRD &= ~((1 << DDD2)|(1 << DDD3));
    DDRC = 0;

    //Deactivate ADC
    ADCSRA &= ~(1 << ADEN);

}

void Interrupt_Init(){
    sei();                      // Enable interruptions globally

    PCICR = _BV(PCIE0)|_BV(PCIE1)|_BV(PCIE2);         // Enable Pin Change Interrupt
    // Enable interrupt on pins 12-13 
    PCMSK0 = _BV(PCINT4)|_BV(PCINT5); 
    // Enable interrupt on pins A0-A3 
    PCMSK1 = _BV(PCINT8)|_BV(PCINT9)|_BV(PCINT10)|_BV(PCINT11);
    // Enable interrupt on pins 2-3
    PCMSK2 = _BV(PCINT18)|_BV(PCINT19);
}

void setup(){
    setup_motor_pins();
    Interrupt_Init();

    Serial.begin(BAUDRATE);
    Serial.setTimeout(10000);  
    Serial.println("SETUP");

    for(int i=0; i<MOTOR_NB; i++){
        MOTOR_PID[i].SetMode(AUTOMATIC);
    }

    set_speed(M1,50);
    set_speed(M2,50);
    set_speed(M3,-50);
    set_speed(M4,-50);

}


static int skip = 0;

void loop(){
    skip = (skip + 1) % 10000;
    
    if (!skip) {
        for(int i = 0; i < 4; i++){
            Serial.print(" dir=");
            Serial.print(MOTOR_STATE[i].direction);
            Serial.print(" pos=");
            Serial.print(MOTOR_STATE[i].position);
            Serial.print(" err=");
            Serial.print(MOTOR_STATE[i].err_nb);
            if (i < 3){
                Serial.println();
            }
            else{
                Serial.println();
                Serial.println();
            }
        }
    }

}



