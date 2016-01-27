#pragma once

const char STX = 2, ETX = 3, ESC = 27;

class Protocol {
    /**
     * Reads an escaped byte from the serial com.
     */
    char readEscaped();
    
    /**
     * Sends escaped data to the serial Port.
     * @param data Byte to send.
     */
    void sendEscaped(char data);
    
    public:
    
    Protocol();
    
    /**
     * Tells if input is available on the serial com.
     */
    bool canRead() const;
    
    /**
     * Reads the start byte of a command and its id on the serial com.
     * @returns The id of the packet's command.
     */
    char readStart();
    
    /**
     * Reads escaped data from the serial com.
     * @param Block of data to save input into.
     */
    template<typename T>
    void read(T& data) {
        for (size_t i = 0; i < sizeof(data); ++i)
            ((char*)&data)[i] = readEscaped();
    }
    
    /**
     * Reads the end byte of a command on the serial com.
     */
    void readEnd();
    
    /**
     * Sends the start of a command to the serial port.
     * @param cmd Id of the command.
     */
    void sendStart(char cmd);
    
    /**
     * Sends escaped data to the serial Port.
     * @param data Data to send.
     */
    template<typename T>
    void send(const T& data) {
        for (size_t i = 0; i < sizeof(data); ++i)
            sendEscaped(((const char*)&data)[i]);
    }
    
    /**
     * Sends the command delimiter to the serial port.
     */
    void sendEnd();
    
    /**
     * Sends debug info as ASCII to the serial port.
     */
    template<typename T>
    void sendASCII(const T& data) {
        sendStart('i');
        Serial.print(data);
        sendEnd();
    }
};
