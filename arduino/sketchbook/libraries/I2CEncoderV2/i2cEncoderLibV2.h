//
//    FILE: i2cEncoderLibV2.h
// VERSION: 0.1..
// PURPOSE: Libreary for the i2c encoder board with arduinp
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET:
//
//     URL:
//
// AUTHOR:
// Simone Caron
//

#ifndef i2cEncoderLibV2_H
#define i2cEncoderLibV2_H

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <limits.h>
#include <float.h>

#define EC_INTERNAL_ERROR 1
#define EC_SUCCESS 0

/*Encoder register definition*/
#define REG_GCONF     0x00
#define REG_GP1CONF   0x01
#define REG_GP2CONF   0x02
#define REG_GP3CONF   0x03
#define REG_INTCONF   0x04
#define REG_ESTATUS   0x05
#define REG_I2STATUS  0x06
#define REG_FSTATUS   0x07
#define REG_CVALB4  0x08
#define REG_CVALB3  0x09
#define REG_CVALB2  0x0A
#define REG_CVALB1  0x0B
#define REG_CMAXB4  0x0C
#define REG_CMAXB3  0x0D
#define REG_CMAXB2  0x0E
#define REG_CMAXB1  0x0F
#define REG_CMINB4  0x10
#define REG_CMINB3  0x11
#define REG_CMINB2  0x12
#define REG_CMINB1  0x13
#define REG_ISTEPB4 0x14
#define REG_ISTEPB3 0x15
#define REG_ISTEPB2 0x16
#define REG_ISTEPB1 0x17
#define REG_RLED    0x18
#define REG_GLED  0x19
#define REG_BLED  0x1A
#define REG_GP1REG  0x1B
#define REG_GP2REG  0x1C
#define REG_GP3REG  0x1D
#define REG_ANTBOUNC  0x1E
#define REG_DPPERIOD  0x1F
#define REG_FADERGB   0x20
#define REG_FADEGP  0x21
#define REG_EEPROMS   0x80

/* Encoder configuration bit. Use with GCONF */
#define FLOAT_DATA		0x01
#define INT_DATA    	0x00

#define WRAP_ENABLE		0x02
#define WRAP_DISABLE	0x00

#define DIRE_LEFT		0x04
#define DIRE_RIGHT		0x00

#define IPUP_DISABLE  0x08
#define IPUP_ENABLE 	0x00

#define RMOD_X2			0x10
#define RMOD_X1			0x00

#define RGB_ENCODER          0x20
#define STD_ENCODER     0x00

#define EEPROM_BANK1     0x40
#define EEPROM_BANK2     0x00

#define RESET			0x80

/* Encoder status bits and setting. Use with: INTCONF for set and with ESTATUS for read the bits  */
#define PUSHR 0x01
#define PUSHP 0x02
#define PUSHD 0x04
#define RINC  0x08
#define RDEC  0x10
#define RMAX  0x20
#define RMIN  0x40
#define INT2  0x80

/* Encoder Int2 bits. Use to read the bits of I2STATUS  */
#define	GP1_POS  0x01
#define GP1_NEG  0x02
#define GP2_POS  0x04
#define GP2_NEG  0x08
#define GP3_POS  0x10
#define GP3_NEG  0x20
#define FADE_INT  0x40

/* Encoder Fade status bits. Use to read the bits of FSTATUS  */
#define FADE_R   0x01
#define FADE_G   0x02
#define FADE_B   0x04
#define FADE_GP1  0x08
#define FADE_GP2  0x10
#define FADE_GP3  0x20

/* GPIO Configuration. USe with GP1CONF,GP2CONF,GP3CONF */
#define GP_PWM 0x00
#define GP_OUT 0x01
#define GP_AN 0x02
#define GP_IN 0x03

#define GP_PULL_EN 0x04
#define GP_PULL_DI 0x00

#define GP_INT_DI 0x00
#define GP_INT_PE 0x08
#define GP_INT_NE 0x10
#define GP_INT_BE 0x18




union Data_v {
  float fval;
  int16_t val;
  int32_t lval;
  uint8_t bval[4];
};

class i2cEncoderLibV2
{
  public:
    i2cEncoderLibV2(uint8_t add);
    uint8_t begin( uint8_t conf);

    /**    Read functions   **/
    /** Configuration function **/
    uint8_t readGP1conf(void);
    uint8_t readGP2conf(void);
    uint8_t readGP3conf(void);
    uint8_t readInterruptConfig(void);

    /** Status function **/
    bool updateStatus(void);
    bool readStatus(uint8_t s);
    uint8_t readStatus(void);

    bool readInt2(uint8_t s);
    uint8_t readInt2(void);

    bool readFadeStatus(uint8_t s);
    uint8_t readFadeStatus(void);

    /** Encoder functions **/
    float readCounterFloat(void);
    int32_t readCounterLong(void);
    int16_t readCounterInt(void);
    int8_t readCounterByte(void);

    int32_t readMax(void);
    float readMaxFloat(void);

    int32_t readMin(void);
    float readMinFloat(void);

    int32_t readStep(void);
    float readStepFloat(void);

    /** RGB LED Functions **/
    uint8_t readLEDR(void);
    uint8_t readLEDG(void);
    uint8_t readLEDB(void);

    /** GP LED Functions **/
    uint8_t readGP1(void);
    uint8_t readGP2(void);
    uint8_t readGP3(void);

    /** Timing registers **/
    uint8_t readAntibouncingPeriod(void);
    uint8_t readDoublePushPeriod(void);
    uint8_t readFadeRGB(void);
    uint8_t readFadeGP(void);

    /** EEPROM register **/
    uint8_t readEEPROM(uint8_t add);


    uint8_t readEncoderByte(uint8_t reg);
    int16_t readEncoderInt(uint8_t reg);
    int32_t readEncoderLong(uint8_t reg);

    /******    Write functions   ********/
    uint8_t writeGP1conf(uint8_t gp1);
    uint8_t writeGP2conf(uint8_t gp2);
    uint8_t writeGP3conf(uint8_t gp3);
    uint8_t writeInterruptConfig(uint8_t interrupt);

    /** Encoder functions **/
    uint8_t writeCounter(int32_t counter);
    uint8_t writeCounter(float counter);

    uint8_t writeMax(int32_t max);
    uint8_t writeMax(float max);

    uint8_t writeMin(int32_t min);
    uint8_t writeMin(float min);

    uint8_t writeStep(int32_t step);
    uint8_t writeStep(float step);

    /** RGB LED Functions **/
    uint8_t writeLEDR(uint8_t rled);
    uint8_t writeLEDG(uint8_t gled);
    uint8_t writeLEDB(uint8_t bled);
    uint8_t writeRGBCode(uint32_t rgb);

    /** GP LED Functions **/
    uint8_t writeGP1(uint8_t gp1);
    uint8_t writeGP2(uint8_t gp2);
    uint8_t writeGP3(uint8_t gp3);

    /** Timing registers **/
    uint8_t writeAntibouncingPeriod(uint8_t bounc);
    uint8_t writeDoublePushPeriod(uint8_t dperiod);
    uint8_t writeFadeRGB(uint8_t fade);
    uint8_t writeFadeGP(uint8_t fade);

    /** EEPROM register **/
    uint8_t writeEEPROM(uint8_t add, uint8_t data);

  private:
    uint8_t _add = 0x00;
    uint8_t _stat = 0x00;
    uint8_t _stat2 = 0x00;
    uint8_t _gconf = 0x00;

    float readEncoderFloat(uint8_t reg);
    uint8_t writeEncoder(uint8_t reg, uint8_t data);
    uint8_t writeEncoder(uint8_t reg, int32_t data);
    uint8_t writeEncoder(uint8_t reg, float data);
    uint8_t writeEncoder24bit(uint8_t reg, uint32_t data);

};

#endif
