/*
 * Copyright (c) 2019, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_i2c.h"
#include "sensirion_arch_config.h"

// needed for delay() routine
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif



#ifdef SPS30_USE_ALT_I2C
#include "i2c_master_lib.h"

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication. After this function has been called, the functions
 * i2c_read() and i2c_write() must succeed.
 */
void sensirion_i2c_init()
{
   I2c.begin();
}

s8 sensirion_i2c_read(u8 address, u8* data, u16 count)
{
    return I2c.read(address, count, data);
}

s8 sensirion_i2c_write(u8 address, const u8* data, u16 count)
{
    // the API doesn't forsee calls without register, so we'll use the first
    // byte as "register", and pass the rest as data argument
    if (count == 0) {
      return 0;
    }
    return I2c.write(address, data[0], data + 1, count - 1);
}

#else /* SPS30_USE_ALT_I2C */

#include <Wire.h>
/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication. After this function has been called, the functions
 * i2c_read() and i2c_write() must succeed.
 */
void sensirion_i2c_init()
{
   Wire.begin();
}

s8 sensirion_i2c_read(u8 address, u8* data, u16 count)
{
    u8 readData[count];
    u8 rxByteCount=0;

    // 2 bytes RH, 1 CRC, 2 bytes T, 1 CRC
    Wire.requestFrom((uint8_t)address, (uint8_t)count);

    while (Wire.available()) { // wait till all arrive
      readData[rxByteCount++] = Wire.read();
      if(rxByteCount >= count)
        break;
    }

    memcpy(data, readData, count);

    return 0;
}

s8 sensirion_i2c_write(u8 address, const u8* data, u16 count)
{
    Wire.beginTransmission(address);
    Wire.write(data, count);
    Wire.endTransmission();

    return 0;
}
#endif /* SPS30_USE_ALT_I2C */


/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(u32 useconds) {
    delay((useconds / 1000) + 1);
}

#ifdef __cplusplus
} // extern "C"
#endif
