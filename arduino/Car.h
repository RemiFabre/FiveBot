#pragma once

#include "Wheel.h"

ISR(PCINT0_vect);
ISR(PCINT1_vect);
ISR(PCINT2_vect);
ISR(TIMER2_COMPB_vect);

class Car {
    Motor mMotors[4];
    RotaryEncoder mEncoders[4];
    Wheel mWheels[4];
    float mPosition[2];
    float mOrientation;
    
    /**
     * Sets up the serial com.
     */
    void setupSerial();
    
    /**
     * Sets up the timer used to trigger calls to #update().
     */
    void setupUpdateTimer();
    
    /**
     * Updates the encoders and the odometry of the car.
     * Loop function called at a regular interval from a timer interrupt vector.
     * Must be quick.
     */
    void updateOdometry();
    
    /**
     * Sets the speed of each wheel to make the car move.
     * @param vx speed forward/backward in rad/s
     * @param vy speed sideways in rad/s
     * @param w rotation speed in rad/s
     */
    void setSpeed(float vx, float vy, float w);
    
    /**
     * Sends the start of a command to the serial port.
     * @param cmd Id of the command.
     */
    void sendStart(char cmd) const;
    
    /**
     * Sends escaped data to the serial Port.
     * @param data Byte to send.
     */
    void send(char data) const;
    
    /**
     * Sends escaped data to the serial Port.
     * @param data Data to send.
     */
    template<typename T>
    void send(const T& data) const {
        for (size_t i = 0; i < sizeof(data); ++i)
            send(((const char*)&data)[i]);
    }
    
    /**
     * Sends the command delimiter to the serial port.
     */
    void sendEnd() const;
    
    /**
     * Tells if input is available on the serial com.
     */
    bool canRead() const;
    
    /**
     * Reads the start byte of a command and its id on the serial com.
     */
    char readStart() const;
    
    /**
     * Reads an escaped byte from the serial com.
     */
    char read() const;
    
    /**
     * Reads escaped data from the serial com.
     * @param Block of data to save input into.
     */
    template<typename T>
    void read(T& data) const {
        for (size_t i = 0; i < sizeof(data); ++i)
            ((char*)&data)[i] = Serial.read();
    }
    
    /**
     * Reads the end byte of a command on the serial com.
     */
    void readEnd() const;
    
    public:
    
    Car();
    
    /**
     * Loop function called by the main program.
     * Can be slow and can use the serial com.
     */
    void loop();
    
    /**
     * Publishes the position and orientation of the car.
     */
    void publishOdometry() const;
    
    /**
     * Publishes the speed of the wheels.
     */
    void publishWheels() const;
    
    /**
     * Interprets the commands given on the serial com.
     */
    void readCommand();
    
    /**
     * Interrupt vectors need to call private functions.
     */
    friend void PCINT0_vect();
    friend void PCINT1_vect();
    friend void PCINT2_vect();
    friend void TIMER2_COMPB_vect();
};
