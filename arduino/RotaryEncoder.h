#pragma once

class RotaryEncoder {
  /**
   * Pin inside the port used to read channel A.
   */
  unsigned char mPinA;

  /**
   * Pin inside the port used to read channel B.
   */
  unsigned char mPinB;

  /**
   * Current phase of the encoder.
   */
  char mPhase;

  /**
   * Last known direction of the encoder.
   */
  char mDirection;

  /**
   * Error count.
   */
  unsigned int mErrors;

 public:
  /**
   * Computed absolute position of the encoder.
   * Can overflow if not reset externally.
   */
  int mPosition;

  /**
   * Sets up the pins used by the encoder.
   * @param ddr Data direction register for the encoder's port.
   * @param pinA Bit inside the port used to read channel A.
   * @param pinB Bit inside the port used to read channel B.
   * @param pcie Bit of the pin change interrupt enable register to set.
   * @param pcmsk Pin change interrupt mask register for the encoder's port.
   * @param pcintA Bit inside PCMSK to set to have interrupts on channel A.
   * @param pcintB Bit inside PCMSK to set to have interrupts on channel B.
   */
  RotaryEncoder(volatile unsigned char& ddr, unsigned char pinA,
                unsigned char pinB, unsigned char pcie,
                volatile unsigned char& pcmsk, unsigned char pcintA,
                unsigned char pcintB);

  /**
   * Reads the encoder phase and updates the state.
   * @param port Pointer to the port to read.
   */
  void update(unsigned char port);

  /**
   * Returns the errors encountered by the encoder.
   */
  inline unsigned int getErrorCount() const { return mErrors; }
};
