#include <PID.h>



///*********************  DEFINITIONS   *********************///  

#define MVT 0x01
#define ENC 0x02

#define MOTOR_NB 4

struct {
    unsigned char direction;
    unsigned char speed;
    unsigned char enc_a;
    unsigned char enc_b;
} 
MOTOR_PIN[MOTOR_NB] = {
    {
        4, 5, 2, 3        }
    ,
    {
        7, 6, A5, A4        }
    , 
    {
        8, 9, A0, A1        }
    ,
    {
        11, 10, 12, 13        }
};


struct {
    double position;
    double target_position;
    double speed;
    double target_speed;
    int direction;
    int encoder_phase;
    double error; 
    double cmd;
} 
MOTOR_STATE[MOTOR_NB] = {
    0};


PID MOTOR_PID[MOTOR_NB] = {
    PID(&MOTOR_STATE[0].speed, &MOTOR_STATE[0].cmd, &MOTOR_STATE[0].target_speed, 
        &MOTOR_STATE[0].error,2,1,3,DIRECT),
    PID(&MOTOR_STATE[1].speed, &MOTOR_STATE[1].cmd, &MOTOR_STATE[1].target_speed,
        &MOTOR_STATE[1].error,0.5,0,0,DIRECT),
    PID(&MOTOR_STATE[2].speed, &MOTOR_STATE[2].cmd, &MOTOR_STATE[2].target_speed,
        &MOTOR_STATE[2].error,0.5,0,0,DIRECT),
    PID(&MOTOR_STATE[3].speed, &MOTOR_STATE[3].cmd, &MOTOR_STATE[3].target_speed,
        &MOTOR_STATE[0].error,0.5,0,0,DIRECT)
    };

long t1 = 0;
long t2 = 0;
long t = 0;
boolean bench = false;
int ct = 0;

long t_read, t_set, t_enc;

const unsigned short ROTARY_ENCODER_PHASE[2][2] = {
    {
        0,1          }
    ,{
        3,2          }
}; 

void poll_encoders() {

    t_read = micros();
    t_enc = t_read-t_set;

    for (int i = 0; i < 4; ++i) {

        const int a = digitalRead(MOTOR_PIN[i].enc_a);
        const int b = digitalRead(MOTOR_PIN[i].enc_b);
        const int phase = ROTARY_ENCODER_PHASE[a][b];

        if (phase == ((MOTOR_STATE[i].encoder_phase + 1) % 4))
            MOTOR_STATE[i].direction = 1;
        else if (((phase + 1) % 4) == MOTOR_STATE[i].encoder_phase)
            MOTOR_STATE[i].direction = -1;
        else if (phase == MOTOR_STATE[i].encoder_phase)
            MOTOR_STATE[i].direction = 0;
        else {
            //read error ? should not happen, and therefore will ;)
            /*Serial.print("Encoder read error, phase ");
             		  Serial.print(MOTOR_STATE[i].encoder_phase);
             		  Serial.print(" --> ");
             		  Serial.print(phase);
             		  Serial.print(" wheel: ");
             		  Serial.println(i);*/
            MOTOR_STATE[i].error++;
            MOTOR_STATE[i].direction = 0;
        }
        MOTOR_STATE[i].encoder_phase = phase;
        MOTOR_STATE[i].position += MOTOR_STATE[i].direction;
        MOTOR_STATE[i].speed = MOTOR_STATE[i].position / t_enc;
    }

    t_set = micros();

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

    /*
  t1 = micros();
     
     poll_encoders();
     
     if(Serial.available()){
     String cmd = Serial.readStringUntil('\n'); 
     if(cmd == "move"){
     long v[4]= {
     0      };
     for(int i=0; i < 4; i++){
     v[i] = Serial.parseInt();
     //Serial.println(v[i]);        
     }
     
     for (int i = 0; i < 4; i++)
     {
     analogWrite(MOTOR_PIN[i].speed,abs(v[i]));
     digitalWrite(MOTOR_PIN[i].direction, (v[i] < 0) ? LOW : HIGH);
     }
     }
     else if(cmd == "encoders"){
     for(int i=0; i < 4; i++){
     Serial.println(MOTOR_STATE[i].position);        
     }    
     }
     else if(cmd == "errors"){
     for(int i=0; i < 4; i++){
     Serial.println(MOTOR_STATE[i].error);        
     }    
     }
     else if(cmd == "bench"){
     bench = !bench; 
     }
     else if (cmd == "odometry") {
     float w_r[] = {
     MOTOR_STATE[0].position/3072.0,
     MOTOR_STATE[1].position/3072.0,
     MOTOR_STATE[2].position/3072.0,
     MOTOR_STATE[3].position/3072.0,
     };
     float xyw[3];
     get_vx_vy_w(w_r, xyw);
     Serial.println(xyw[0]);
     Serial.println(xyw[1]);
     Serial.println(xyw[2]);
     }
     }
     
     t2 = micros();
     t = max(t2-t1, t);
     ct++;
     if(bench && (ct > 10000)){
     Serial.println(t);
     ct = 0;
     t = 0;
     }
     
     */
    poll_encoders();
    set_speed(0,10);
  
}



