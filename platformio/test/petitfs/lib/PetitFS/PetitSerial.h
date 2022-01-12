// A small Serial class in the spirit of Petit FatFs.
#ifndef PetitSerial_h
#define PetitSerial_h
#include <Arduino.h>

// Set zero if you only need single character I/O.
#define PETIT_SERIAL_USE_PRINT 1

#if defined(UDR0) || defined(DOXYGEN)
const uint16_t MIN_2X_BAUD = F_CPU/(4*(2*0XFFF + 1)) + 1;
//------------------------------------------------------------------------------
/**
 * \class PetitSerial
 * \brief mini serial class for the %SdFat library.
 */
#if PETIT_SERIAL_USE_PRINT
class PetitSerial : public Print {
 public:
  using Print::write;
#else   // PETIT_SERIAL_USE_PRINT
class PetitSerial : public Print {
 public:
#endif  // PETIT_SERIAL_USE_PRINT
  /**
   * Set baud rate for serial port zero and enable in non interrupt mode.
   * Do not call this function if you use another serial library.
   * \param[in] baud rate
   */
  void begin(uint32_t baud) {
    uint16_t baud_setting;
    // don't worry, the compiler will squeeze out F_CPU != 16000000UL
    if ((F_CPU != 16000000UL || baud != 57600) && baud > MIN_2X_BAUD) {
      // Double the USART Transmission Speed
      UCSR0A = 1 << U2X0;
      baud_setting = (F_CPU / 4 / baud - 1) / 2;
    } else {
      // hardcoded exception for compatibility with the bootloader shipped
      // with the Duemilanove and previous boards and the firmware on the 8U2
      // on the Uno and Mega 2560.
      UCSR0A = 0;
      baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }
    // assign the baud_setting
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;
    // enable transmit and receive
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
  }    
  /**
   *  Unbuffered read
   *  \return -1 if no character is available or an available character.
   */
  int read() {
    if (UCSR0A & (1 << RXC0)) {
      return UDR0;
    }
    return -1;
  }
  /**
   * Unbuffered write
   *
   * \param[in] b byte to write.
   * \return 1
   */
  size_t write(uint8_t b) {
    while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}
    UDR0 = b;
    return 1;
  }
};
#endif defined(UDR0) || defined(DOXYGEN)
#endif  // PetitSerial_h

