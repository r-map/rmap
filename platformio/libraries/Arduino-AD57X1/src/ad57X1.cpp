/*    
 *  The AD5781 library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the 
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ad57X1.h"

// Use CPOL = 0,  CPHA = 1 for ADI DACs
AD57X1::AD57X1(uint8_t _cs_pin, SPIClass* const _spi, const uint8_t _VALUE_OFFSET, uint32_t spiClockFrequency, uint8_t _ldac_pin, const bool cs_polarity) :
  VALUE_OFFSET(_VALUE_OFFSET),
  spi(_spi), PIN_CS(_cs_pin),
  PIN_LDAC(_ldac_pin),
  CS_POLARITY(cs_polarity),
  spi_settings(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE1)),
  InternalAmplifier(false){
}

void AD57X1::writeSPI(const uint32_t value) {
  digitalWrite (this->PIN_CS, this->CS_POLARITY);
  delayMicroseconds(5); // t4
  this->spi->beginTransaction(this->spi_settings);
  this->spi->transfer((value >> 16) & 0xFF);
  this->spi->transfer((value >> 8) & 0xFF);
  this->spi->transfer((value >> 0) & 0xFF);
  this->spi->endTransaction();
  delayMicroseconds(2); // t5
  digitalWrite(this->PIN_CS, !this->CS_POLARITY);
  
}

uint32_t AD57X1::readSPI(const uint32_t value) {

  uint32_t readvalue;
  this->writeSPI(value);
  delayMicroseconds(50); // t6
  
  digitalWrite (this->PIN_CS, this->CS_POLARITY);
  delayMicroseconds(5); // t4

  this->spi->beginTransaction(this->spi_settings);
  readvalue  = (uint32_t)this->spi->transfer(value) << 16;
  readvalue |= (uint32_t)this->spi->transfer(value) << 8;
  readvalue |= (uint32_t)this->spi->transfer(value);
  this->spi->endTransaction();

  delayMicroseconds(2); // t5
  digitalWrite(this->PIN_CS, !this->CS_POLARITY);

  return readvalue;
}


void AD57X1::updateControlRegister() {
  this->writeSPI(this->WRITE_REGISTERS | this->CONTROL_REGISTER | this->controlRegister);
}

void AD57X1::reset() {
  this->enableOutput();
}

// value is an 18 or 20 bit value
void AD57X1::setValue(const uint32_t value) {
  uint32_t command = this->WRITE_REGISTERS | this->DAC_REGISTER | ((value << this->VALUE_OFFSET) & 0xFFFFF);

  this->writeSPI(command);

  if (this->PIN_LDAC >= 0) {
    delayMicroseconds(25);  // t11
    digitalWrite(this->PIN_LDAC, HIGH);
    delayMicroseconds(20); // t12
    digitalWrite(this->PIN_LDAC, LOW);
  } else {
    this->writeSPI(this->SW_CONTROL_REGISTER | this->SW_CONTROL_LDAC);
  }  
}

// value is an 18 or 20 bit value
uint32_t AD57X1::readValue() {
  uint32_t command = this->READ_REGISTERS | this->DAC_REGISTER;

  return this->readSPI(command);
}


void AD57X1::setTension(const int32_t millivolt) {

  uint32_t value;  
  if (this->InternalAmplifier) {
    value=((millivolt+5000)/10000.D)*0XFFFFF;
  } else {
    value=(millivolt/5000.D)*0XFFFFF;
  }
  setValue(value);
}

void AD57X1::enableOutput() {
  this->setOutputClamp(false);
  this->setTristateMode(false);
  this->updateControlRegister();
}

void AD57X1::setInternalAmplifier(const bool enable) {
  // (1 << this->RBUF_REGISTER) : internal amplifier is disabled (default)
  // (0 << this->RBUF_REGISTER) : internal amplifier is enabled
  this->controlRegister = (this->controlRegister & ~(1 << this->RBUF_REGISTER)) | (!enable << this->RBUF_REGISTER);
  this->InternalAmplifier=enable;
}

// Setting this to enabled will overrule the tristate mode and clamp the output to GND
void AD57X1::setOutputClamp(const bool enable) {
  // (1 << this->OUTPUT_CLAMP_TO_GND_REGISTER) : the output is clamped to GND (default)
  // (0 << this->OUTPUT_CLAMP_TO_GND_REGISTER) : the dac is in normal mode
  this->controlRegister = (this->controlRegister & ~(1 << this->OUTPUT_CLAMP_TO_GND_REGISTER)) | (enable << this->OUTPUT_CLAMP_TO_GND_REGISTER);
}

void AD57X1::setTristateMode(const bool enable) {
  // (1 << this->OUTPUT_TRISTATE_REGISTER) : the dac output is in tristate mode (default)
  // (0 << this->OUTPUT_TRISTATE_REGISTER) : the dac is in normal mode
  this->controlRegister = (this->controlRegister & ~(1 << this->OUTPUT_TRISTATE_REGISTER)) | (enable << this->OUTPUT_TRISTATE_REGISTER);
}

void AD57X1::setOffsetBinaryEncoding(const bool enable) {
  // (1 << this->OFFSET_BINARY_REGISTER) : the dac uses offset binary encoding, should be used when writing unsigned ints
  // (0 << this->OFFSET_BINARY_REGISTER) : the dac uses 2s complement encoding, should be used when writing signed ints (default)
  this->controlRegister = (this->controlRegister & ~(1 << this->OFFSET_BINARY_REGISTER)) | (enable << this->OFFSET_BINARY_REGISTER);
}

/* Linearity error compensation
 * 
 */
// enable = 0 -> Range 0-10 V
// enable = 1 -> Range 0-20 V
void AD57X1::setReferenceInputRange(const bool enableCompensation) {
  this->controlRegister = (this->controlRegister & ~(0b1111 << this->LINEARITY_COMPENSATION_REGISTER)) | ((enableCompensation ? this->REFERENCE_RANGE_20V : this->REFERENCE_RANGE_10V) << this->LINEARITY_COMPENSATION_REGISTER);
}

void AD57X1::begin(const bool initSpi) {
  pinMode(this->PIN_CS, OUTPUT);
  digitalWrite(this->PIN_CS, !this->CS_POLARITY);

  if (this->PIN_LDAC >= 0) {
    pinMode(this->PIN_LDAC, OUTPUT);
    digitalWrite(this->PIN_LDAC, LOW);
  }

  if (initSpi) {
    this->spi->begin();
  }
  //this->reset();
}

