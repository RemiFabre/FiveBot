#include <PID.h>



///*********************  DEFINITIONS   *********************///  

#define MVT 0x01
#define ENC 0x02

#define M1 0
#define M2 1
#define M3 2
#define M4 4
 
#define MOTOR_NB 4
#define SPEED_MEASURE_COUNT 5 

#define CALC_SEQ(a,b) ((a^b)|b<<1)
#define NEXT_INDEX(i) ((i+1)%SPEED_MEASURE_COUNT)

struct {
    unsigned char direction;
    unsigned char speed;
    unsigned char enc_a;
    unsigned char enc_b;
} 
MOTOR_PIN[MOTOR_NB] = {
    {
        4, 5, 12, 13    }
    ,
    {
        7, 6, 2, 3      }
    , 
    {
        0, 0, 8, 9      }
    ,
    {
        0, 0, 10, 11    }
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


// OBSOLETE
/*const unsigned short ROTARY_ENCODER_PHASE[2][2] = {
    {
        0,1          }
    ,{
        3,2          }
};*/ 

void poll_encoders() {

    for (int i = 0; i < 4; ++i) {

        const int a = digitalRead(MOTOR_PIN[i].enc_a);
        const int b = digitalRead(MOTOR_PIN[i].enc_b);
        const int phase = CALC_SEQ(a,b);

        // Calcul de la direction
        const int direction = (phase - MOTOR_STATE[i].encoder_phase + 4) % 4;
        if(direction != 2)
            MOTOR_STATE[i].direction = direction;
        else
            ++MOTOR_STATE[i].err_nb;
        MOTOR_STATE[i].encoder_phase = phase;
        MOTOR_STATE[i].position += MOTOR_STATE[i].direction;
        
        // Calcul de la vitesse
        MOTOR_STATE[i].time_hist[MOTOR_STATE[i].index] = micros();
        MOTOR_STATE[i].position_hist[MOTOR_STATE[i].index] = MOTOR_STATE[i].position;
        delta_t = MOTOR_STATE[i].time_hist[MOTOR_STATE[i].index] - MOTOR_STATE[i].time_hist[NEXT_INDEX(MOTOR_STATE[i].index)];
        delta_p = MOTOR_STATE[i].position_hist[MOTOR_STATE[i].index] - MOTOR_STATE[i].position_hist[NEXT_INDEX(MOTOR_STATE[i].index)];
        MOTOR_STATE[i].speed = delta_p / delta_t;
        
        MOTOR_STATE[i].index = NEXT_INDEX(MOTOR_STATE[i].index);
    }
}


// Works with PID in POSITION !
// void go_to_angle(int angle){
//     MOTOR_STATE[1].target_position = angle;
//     MOTOR_PID[1].Compute();
//     analogWrite(MOTOR_PIN[1].speed, abs(MOTOR_STATE[1].cmd));
//     digitalWrite(MOTOR_PIN[1].direction, (MOTOR_STATE[1].error < 0) ? LOW : HIGH);
    
//     // if(MOTOR_STATE[1].error !=0) Serial.println(MOTOR_STATE[1].error);
// }

// Set speed with PID, v in tr/s
void set_speed(int motor, float v){
    MOTOR_STATE[motor].target_speed = v;
    MOTOR_PID[motor].Compute();
    // MOTOR_STATE[motor].cmd = v;
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
    for (int i = 0; i < 4; ++i) {
        pinMode(MOTOR_PIN[i].direction, OUTPUT);
        pinMode(MOTOR_PIN[i].speed, OUTPUT);
        pinMode(MOTOR_PIN[i].enc_a, INPUT);
        pinMode(MOTOR_PIN[i].enc_b, INPUT);
    }
}

void setup(){
    setup_motor_pins();
    Serial.begin(115200);
    Serial.setTimeout(10000);  

    for(int i=0; i<MOTOR_NB; i++){
        MOTOR_PID[i].SetMode(AUTOMATIC);
    }
}



void loop(){

    poll_encoders();
    set_speed(M2,50);
  
}



