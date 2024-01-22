/**
 * sps-30 Library file
 *
 * Copyright (c) January 2019, Paul van Haastrecht
 *
 * All rights reserved.
 *
 * The library can communicated over different communication channels
 * with the SPS-30 to get and set information. It works with either
 * UART or I2c communication. The I2C link has a number of restrictions.
 * See detailed document
 *
 * Development environment specifics:
 * Arduino IDE 1.9
 *
 * ================ Disclaimer ===================================
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************
 */

#include "sps30.h"

#if !defined INCLUDE_I2C && !defined INCLUDE_UART
#error you must enable either I2C or UART communication
#endif


/**
 * @brief constructor and initialize variables
 */
#if defined INCLUDE_UART
SPS30::SPS30(Stream *serial)
#else
SPS30::SPS30(void)
#endif
{
#if defined INCLUDE_UART
  _serial = &serial;
#endif
  _Send_BUF_Length = 0;
  _Receive_BUF_Length = 0;
  _Sensor_Comms = NONE_COMMS;
  _started = false;

#if defined INCLUDE_I2C                      // added with version 1.3.0
  if (I2C_LENGTH >= 64)  I2C_Max_bytes = 40; // total length
  else I2C_Max_bytes = 20;                   // only Mass
#endif
}

/**
 * @brief Initialize the communication port
 *
 * @param port : communication channel to be used (see sps30.h)
 */
bool SPS30::begin(serial_port port)
{
    _Sensor_Comms = port;

    if (port == I2C_COMMS)  // initialize I2C
    {

#if defined INCLUDE_I2C
      //I2C_init();
#else
      SPS30LOGD  (F("I2C communication not enabled\n"));
        return(false);
#endif // INCLUDE_I2C

    }
    else     // setup serial communication
    {

#if defined INCLUDE_UART

    //_serial->begin(115200);       // no other speed is allowed for SPS30

#else
      SPS30LOGD (F("UART communication not enabled\n"));
        return(false);
#endif // INCLUDE_UART

    }

    return true;
}

/**
 * @brief check if SPS30 sensor is available (read serial number)
 *
 * Return:
 *   true on success else false
 */
bool SPS30::probe() {
    char buf[32];

    if (GetSerialNumber(buf, 32) == SPS30ERR_OK) return(true);
    return(false);
}

/**
 * @brief Instruct SPS30 sensor
 * @param type
 *  SER_START_MEASUREMENT   Start measurement
 *  SER_STOP_MEASUREMENT    Stop measurement
 *  SER_RESET               Perform reset
 *  SER_START_FAN_CLEANING  start cleaning
 *
 * Return
 *  true = ok
 *  false = error
 */
bool SPS30::Instruct(uint8_t type)
{
    uint8_t ret;

    if (type == SER_START_FAN_CLEANING)
    {
        if(_started == false)
        {
	  SPS30LOGD  (F("ERROR : Sensor is not in measurement mode\n"));
	  return(false);
        }
    }
#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        if (type == SER_START_MEASUREMENT)
            I2C_fill_buffer(I2C_START_MEASUREMENT);
        else if(type == SER_STOP_MEASUREMENT)
            I2C_fill_buffer(I2C_STOP_MEASUREMENT);
        else if(type == SER_RESET)
            I2C_fill_buffer(I2C_RESET);
        else if(type == SER_START_FAN_CLEANING)
            I2C_fill_buffer(I2C_START_FAN_CLEANING);
        else
            return(SPS30ERR_PARAMETER);

        ret = I2C_SetPointer();

    }
    else // if serial communication
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {    // fill buffer to send
        if (SHDLC_fill_buffer(type) != true) return(SPS30ERR_PARAMETER);

        ret = ReadFromSerial();
    }
#else
    {}
#endif // INCLUDE_UART
    if (ret == SPS30ERR_OK){

        if (type == SER_START_MEASUREMENT) {
            _started = true;
            delay(1000);
        }
        else if (type == SER_STOP_MEASUREMENT)
            _started = false;

        else if (type == SER_RESET){
            _started = false;
            delay(2000);
        }

        return(true);
    }
    return(false);
}

/**
 * @brief General Read device info
 *
 * @param type:
 *  Product Name  : SER_READ_DEVICE_PRODUCT_NAME
 *  Article Code  : SER_READ_DEVICE_ARTICLE_CODE
 *  Serial Number : SER_READ_DEVICE_SERIAL_NUMBER
 *
 * @param ser     : buffer to hold the read result
 * @param len     : length of the buffer
 *
 * return
 *  SPS30ERR_OK = ok
 *  else error
 */
uint8_t SPS30::Get_Device_info(uint8_t type, char *ser, uint8_t len)
{
    uint8_t ret,i, offset;

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        // Serial or article code
        if (type == SER_READ_DEVICE_SERIAL_NUMBER)
            I2C_fill_buffer(I2C_READ_SERIAL_NUMBER);

        else if(type == SER_READ_DEVICE_ARTICLE_CODE)
            I2C_fill_buffer(I2C_READ_ARTICLE_CODE);

        // product name is not available on I2C
        else {
            for (i = 0; i < len ; i++) ser[i] = 0x0;
            return(SPS30ERR_OK);
        }

        // true = check zero termination
        ret =  I2C_SetPointer_Read(len,true);
        offset = 0;
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(type) != true) return(SPS30ERR_PARAMETER);

        ret = ReadFromSerial();

        offset = 5;
    }
#else
    {}
#endif // INCLUDE_UART

    if (ret != SPS30ERR_OK) return (ret);

    // get data
    for (i = 0; i < len ; i++) {
        ser[i] = _Receive_BUF[i+offset];
        if (ser[i] == 0x0) break;
    }

    return(SPS30ERR_OK);
}

/**
 * @brief : read the auto clean interval
 * @param val : pointer to return the interval value
 *
 * The default cleaning interval is set to 604’800 seconds (i.e., 168 hours or 1 week).
 *
 * Return:
 *  OK = SPS30ERR_OK
 *  else error
 */
uint8_t SPS30::GetAutoCleanInt(uint32_t *val)
{
    uint8_t ret, offset;

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(I2C_AUTO_CLEANING_INTERVAL);

        ret = I2C_SetPointer_Read(4);

        offset = 0;
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(SER_READ_AUTO_CLEANING) != true) return(SPS30ERR_PARAMETER);

        ret = ReadFromSerial();

        offset = 5;
    }
#else
    {}
#endif // INCLUDE_UART

    // get data
    *val = byte_to_U32(offset);

    return(ret);
}

/**
 * @brief : SET the auto clean interval
 * @param val : pointer for the interval value
 *
 *
 * Return:
 *  OK = SPS30ERR_OK
 *  else error
 */

uint8_t SPS30::SetAutoCleanInt(uint32_t val)
{

#if defined INCLUDE_I2C
    bool save_started, r;

    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(I2C_SET_AUTO_CLEANING_INTERVAL, val);

        if (I2C_SetPointer() == SPS30ERR_OK)
        {
            /* Datasheet page 15: Note that after writing a new interval, this will be activated immediately.
             * However, if the interval register is read out after setting the new value, the previous value
             * is returned until the next start/reset of the sensor module.
             *
             * A reset() alone will NOT do the job. It will continue to show the old value. The only way is to perform
             * a low level I2C line reset first and then perform a reset()*/
            save_started = _started;

            // flush and release lines
            Wire.~TwoWire();
            delay(1000);
            I2C_init();

            r = reset();

            // do we need to restart ?
            if (r) {if (save_started) r = start();}

            if (r) return(SPS30ERR_OK);
        }

        return(SPS30ERR_PROTOCOL);
    }
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    // fill buffer to send
    if (SHDLC_fill_buffer(SER_WRITE_AUTO_CLEANING, val) != true) return(SPS30ERR_PARAMETER);

    return(ReadFromSerial());
#else
    return (SPS30ERR_PARAMETER);
#endif //INCLUDE_UART
}

/**
 * @brief : get error description
 * @param code : error code
 * @param buf  : buffer to store the description
 * @param len  : length of buffer
 */
void SPS30::GetErrDescription(uint8_t code, char *buf, int len)
{

#if defined SMALLFOOTPRINT
    strncpy(buf, "Error-info disabled", len);
#else
    int i=0;

    while (ERR_desc[i].code != 0xff) {
        if(ERR_desc[i].code == code)  break;
        i++;
    }

    strncpy(buf, ERR_desc[i].desc, len);
#endif // SMALLFOOTPRINT
}

/**
 * @brief : read all values from the sensor and store in structure
 * @param : pointer to structure to store
 *
 * return
 *  SPS30ERR_OK = ok
 *  else error
 */
uint8_t SPS30::GetValues(struct sps_values *v)
{
  uint8_t ret;
  uint8_t offset;

  // measurement started already?
  if (_started == false) {
    return(SPS30ERR_CMDSTATE);
  }
#if defined INCLUDE_I2C
  if (_Sensor_Comms == I2C_COMMS) {
    
    offset = 0;

    // ATTENTION
    // this do not work; on esp8266 (sometimes) after I2C_Check_data_ready
    // READ_MEASURED_VALUE return a crc error !
    // if new data available
    if (I2C_Check_data_ready()) {
    //start();  // for the same I have to put this before read measured values
    //  delay(1);
      I2C_fill_buffer(I2C_READ_MEASURED_VALUE);
      
      // I2C will provide maximum data bytes depending on
      // the I2C read_buffer.

      ret = I2C_SetPointer_Read(I2C_Max_bytes);      
      
      if (ret != SPS30ERR_OK) return (ret);

    } else {	  
      return(SPS30ERR_TIMEOUT);
    }
  }  else
#endif // INCLUDE_I2C
#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send
        if (SHDLC_fill_buffer(SER_READ_MEASURED_VALUE) != true) return(SPS30ERR_PARAMETER);

        ret = ReadFromSerial();

        if (ret != SPS30ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
        // check length
        if (_Receive_BUF[4] != 0x28){
	  SPS30LOGD  (F("%d Not enough bytes for all values\n"), _Receive_BUF[4]);
            return(SPS30ERR_DATALENGTH);
        }
    }
#else
    {}
#endif // INCLUDE_UART

    memset(v,0xF,sizeof(struct sps_values));

    // get data
    v->MassPM1 = byte_to_float(offset);
    v->MassPM2 = byte_to_float(offset + 4);
    v->MassPM4 = byte_to_float(offset + 8);
    v->MassPM10 = byte_to_float(offset + 12);

    // I2C will only provide valid data bytes depending on I2C buffer
    // if I2C buffer is less than 64 we only provide MASS info (set in constructor)
    if (I2C_Max_bytes > 20) {
        v->NumPM0 = byte_to_float(offset + 16);
        v->NumPM1 = byte_to_float(offset + 20);
        v->NumPM2 = byte_to_float(offset + 24);
        v->NumPM4 = byte_to_float(offset + 28);
        v->NumPM10 = byte_to_float(offset + 32);
        v->PartSize = byte_to_float(offset + 36);
    }
    return(SPS30ERR_OK);
}

/**
 * @brief : translate 4 bytes to float IEEE754
 * @param x : offset in _Receive_BUF
 *
 * return : float number
 */
float SPS30::byte_to_float(int x)
{
    ByteToFloat conv;

    for (byte i = 0; i < 4; i++){
       conv.array[3-i] = _Receive_BUF[x+i]; //or conv.array[i] = _Receive_BUF[x+i]; depending on endianness
    }

    return conv.value;
}

/**
 * @brief : translate 4 bytes to Uint32
 * @param x : offset in _Receive_BUF
 *
 * Used for Get Auto Clean interval
 * return : Uint32 number
 */
uint32_t SPS30::byte_to_U32(int x)
{
    ByteToU32 conv;

    for (byte i = 0; i < 4; i++){
        conv.array[3-i] = _Receive_BUF[x+i]; //or conv.array[i] = _Receive_BUF[x+i]; depending on endianness
    }

    return conv.value;
}

/*******************************************************************
 *  UART ROUTINES
 *******************************************************************/
#if defined INCLUDE_UART

/**
 * @brief check and perform byte stuffing
 * @param b   : byte to send
 * @param off : current pointer in _Send_BUF
 *
 * Will store the value in _Send_BUF
 * return the new offset
 */
int SPS30::ByteStuff(uint8_t b, int off)
{
    uint8_t  x = 0;

    switch(b){
        case 0x11: {x = 0x31; break;}
        case 0x13: {x = 0x33; break;}
        case 0x7d: {x = 0x5d; break;}
        case 0x7e: {x = 0x5e; break;}
    }

    if (x == 0) _Send_BUF[off++] = b;
    else
    {
        _Send_BUF[off++] = 0x7D;
        _Send_BUF[off++] = x;
    }

    return(off);
}

/**
 * @brief : unstuff bytes
 * @param : received Stuff byte
 *
 * return
 *  byte to use or zero if error
 */
uint8_t SPS30::ByteUnStuff(uint8_t b)
{
    switch(b){
        case 0x31: return(0x11);
        case 0x33: return(0x13);
        case 0x5d: return(0x7d);
        case 0x5e: return(0x7e);

        default:
	  SPS30LOGD  (F("Incorrect byte Unstuffing. Got: %X\n"),b);
	  return(0);
    }
}

/**
 * @brief : create the SHDLC buffer to send
 * @param parameter: only used for setting auto clean interval
 *
 * return:
 *  true OK
 *  false ERROR
 */
bool SPS30::SHDLC_fill_buffer(uint8_t command, uint32_t parameter)
{
    memset(_Send_BUF,0x0,sizeof(_Send_BUF));
    _Send_BUF_Length = 0;

    int i = 0 ;
    uint8_t tmp;

    _Send_BUF[i++] = SHDLC_IND;
    _Send_BUF[i++] = 0x0;          // address SPS30 is zero
    _Send_BUF[i++] = command;

    switch(command) {

        case SER_START_MEASUREMENT:
            _Send_BUF[i++] = 2;     // length
            _Send_BUF[i++] = 0x1;
            _Send_BUF[i++] = 0x3;
            break;

        case SER_STOP_MEASUREMENT:
        case SER_READ_MEASURED_VALUE:
        case SER_START_FAN_CLEANING:
        case SER_RESET:
            _Send_BUF[i++] = 0;     // length
            break;

        case SER_READ_DEVICE_PRODUCT_NAME:
        case SER_READ_DEVICE_ARTICLE_CODE:
        case SER_READ_DEVICE_SERIAL_NUMBER:
            _Send_BUF[2] = SER_READ_DEVICE_INFO;
            _Send_BUF[i++] = 1;     // length
            _Send_BUF[i++] = command & 0x0f;
            break;

        case SER_READ_AUTO_CLEANING:
            _Send_BUF[2] = SER_AUTO_CLEANING_INTERVAL;
            _Send_BUF[i++] = 1;     // length
            _Send_BUF[i++] = 0;     // Subcommand, this value must be set to 0x00
            break;

        case SER_WRITE_AUTO_CLEANING:
            _Send_BUF[2] = SER_AUTO_CLEANING_INTERVAL;

            _Send_BUF[i++] = 5;  // length
            _Send_BUF[i++] = 0;  // Subcommand, this value must be set to 0x00
             tmp = parameter >> 24 & 0xff;  // change order depending on the indians...
             i = ByteStuff(tmp, i);
             tmp = parameter >> 16 & 0xff;
             i = ByteStuff(tmp, i);
             tmp = parameter >> 8 & 0xff;
             i = ByteStuff(tmp, i);
             tmp = parameter & 0xff;
             i = ByteStuff(tmp, i);
            break;

        default:
            return(false);
            break;

    }

    // add CRC and check for byte stuffing
    tmp = SHDLC_calc_CRC(_Send_BUF, 1, i);
    i = ByteStuff(tmp, i);

    _Send_BUF[i] = SHDLC_IND;
    _Send_BUF_Length = ++i;

    return(true);
}

/**
 * @brief calculate SHDLC CRC
 * @param buf   : buffer to calculated
 * @param first : first data byte to include
 * @param last  : last data byte to include
 *
 * return : calculated CRC
 */
uint8_t SPS30::SHDLC_calc_CRC(uint8_t *buf, uint8_t first, uint8_t last)
{
    uint8_t i;
    uint32_t ret = 0;

    for (i = first; i <= last ; i ++)   ret += buf[i];
    return(~(ret & 0xff));
}

/**
 * @brief send a filled buffer to the SPS30 over serial
 *
 * return
 *   Err_OK is OK
 *   else error
 */
uint8_t SPS30::SendToSerial()
{
    uint8_t i;

    if (_Send_BUF_Length == 0) return(SPS30ERR_DATALENGTH);

      SPS30LOGD  (F("Sending: "));
      for(i = 0; i < _Send_BUF_Length; i++)
	SPS30LOGD(F(" %X"), _Send_BUF[i]);
      SPS30LOGD  (F("\n"));

    for (i = 0 ; i <_Send_BUF_Length; i++)
        _serial->write(_Send_BUF[i]);

    // indicate that command has been sent
    _Send_BUF_Length = 0;

    return(SPS30ERR_OK);
}

/**
 * @brief send command, read response & check for errors
 *
 * return :
 *  Ok = SPS30ERR_OK
 *  else Error code
 */
uint8_t SPS30::ReadFromSerial()
{
    uint8_t ret;

    // write to serial
    ret = SendToSerial();
    if (ret != SPS30ERR_OK) return(ret);

    // wait
    delay(RX_DELAY_MS);

    // read serial
    ret = SerialToBuffer();
    if (ret != SPS30ERR_OK) return(ret);

    /**
     * check CRC.
     * CRC MIGHT have been byte stuffed as well but that is handled
     * in SerialtoBuffer !
     * buffer : hdr addr cmd state length data....data crc hdr
     *           0    1   2    3     4     5       -2   -1  -0
     */

    ret = SHDLC_calc_CRC(_Receive_BUF, 1,_Receive_BUF_Length-2);
    if (_Receive_BUF[_Receive_BUF_Length-1] != ret)
    {
      SPS30LOGD(F("CRC error. expected %X, got %X\n"),_Receive_BUF[_Receive_BUF_Length-1], ret);
      return(SPS30ERR_PROTOCOL);
    }

    // check status
    if (_Receive_BUF[3] != SPS30ERR_OK)
    {
      SPS30LOGD(F("%x : state error\n"),_Receive_BUF[3]);
    }

    return(_Receive_BUF[3]);
}

/**
 * @brief  read bytes into the receive buffer and perform byte (un)stuffing
 */
uint8_t SPS30::SerialToBuffer()
{
    uint32_t startTime;
    bool  byte_stuff = false;
    uint8_t i;
    _serial->flush();

    startTime = millis();
    i = 0;

    // read until last 0x7E
    while (true)
    {
        // prevent deadlock
        if (millis() - startTime > TIME_OUT)
        {
	  SPS30LOGD(F("TimeOut during reading byte %d\n"), i);
	  return(SPS30ERR_TIMEOUT);
        }

        if (_serial->available())
        {
            _Receive_BUF[i] = _serial->read();

            // check for good header
            if (i == 0) {

                if (_Receive_BUF[i] != SHDLC_IND){

		  SPS30LOGD(F("Incorrect Header. Expected 0x7E got %X\n"), _Receive_BUF[i]);
		  return(SPS30ERR_PROTOCOL);
                }
            }
            else {

                // detect Byte stuffing
                if (_Receive_BUF[i] == 0x7D ) {
                    i--;                // remove stuffing byte
                    byte_stuff = true;
                }

                // handle byte stuffing
                else if (byte_stuff) {
                    _Receive_BUF[i] = ByteUnStuff(_Receive_BUF[i]);
                    byte_stuff = false;
                }

                // check last byte received
                else if (_Receive_BUF[i] == SHDLC_IND) {

                    _Receive_BUF_Length = i;
		    SPS30LOGD(F("Received: "));
		    for(i = 0; i < _Receive_BUF_Length+1; i++) SPS30LOGD(F("%X "),_Receive_BUF[i]);
		    SPS30LOGD(F("length: %d\n\n"),_Receive_BUF_Length);
                    return(SPS30ERR_OK);
                }
            }

            i++;

            if(i > MAXRECVBUFLENGTH)
            {
	      SPS30LOGD(F("\nReceive buffer full\n"));
	      return(SPS30ERR_PROTOCOL);
            }
        }
    }
}

#endif  // INCLUDE_UART

/************************************************************
 * I2C routines
 *************************************************************/
#if defined INCLUDE_I2C

/**
 * @brief : Return the expected number of valid values read from device
 *
 * The I2C_Max_bytes is depending on the buffer defined in Wire.h
 *
 * Return
 *  4 = Valid Mass values only
 * 10 = All values are expected to be valid
 */
uint8_t SPS30::I2C_expect()
{
    if (I2C_Max_bytes == 20) return(4);
    return(10);
}

/**
 * @brief : Start I2C communication
 */
void SPS30::I2C_init()
{
    Wire.begin();
}

/**
 * @brief : Fill buffer to send over I2C communication
 * @param cmd: I2C commmand
 * @param interval : value to set for interval
 *
 */
void SPS30::I2C_fill_buffer(uint16_t cmd, uint32_t interval)
{
    memset(_Send_BUF,0xF,sizeof(_Send_BUF));
    _Send_BUF_Length = 0;

    int i = 0 ;

    // add command
    _Send_BUF[i++] = cmd >> 8 & 0xff;   //0 MSB
    _Send_BUF[i++] = cmd & 0xff;        //1 LSB

    switch(cmd) {

        case I2C_START_MEASUREMENT:
            _Send_BUF[i++] = 0x3;       //2 Measurement-Mode, this value must be set to 0x03
            _Send_BUF[i++] = 0x00;      //3 dummy byte
            _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]);
            break;

        case I2C_SET_AUTO_CLEANING_INTERVAL:
            i=0;
            _Send_BUF[i++] = I2C_AUTO_CLEANING_INTERVAL >> 8 & 0xff;   //0 MSB
            _Send_BUF[i++] = I2C_AUTO_CLEANING_INTERVAL & 0xff;        //1 LSB
            _Send_BUF[i++] = interval >> 24 & 0xff;       //2 MSB
            _Send_BUF[i++] = interval >> 16 & 0xff;       //3 MSB
            _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[2]); //4 CRC
            _Send_BUF[i++] = interval >>8 & 0xff;         //5 LSB
            _Send_BUF[i++] = interval & 0xff;             //6 LSB
            _Send_BUF[i++] = I2C_calc_CRC(&_Send_BUF[5]); //7 CRC
            break;
        }

     _Send_BUF_Length = i;
}

/**
 * @brief : SetPointer (and write if included) with I2C communication
 *
 * return:
 * Ok SPS30ERR_OK
 * else error
 */
uint8_t SPS30::I2C_SetPointer()
{
    if (_Send_BUF_Length == 0) return(SPS30ERR_DATALENGTH);

    SPS30LOGD(F("I2C Sending: "));
    for(byte i = 0; i < _Send_BUF_Length; i++)
      SPS30LOGD(F(" %X"), _Send_BUF[i]);
    SPS30LOGD(F("\n"));

    Wire.beginTransmission(SPS30_ADDRESS);
    Wire.write(_Send_BUF, _Send_BUF_Length);
    Wire.endTransmission();

    return(SPS30ERR_OK);
}

/**
 * @brief : read with I2C communication
 * @param cnt: number of data bytes to get
 * @param chk_zero : needed for read info buffer
 *  false : expect all the bytes
 *  true  : expect NULL termination and cnt is MAXIMUM byte
 *
 */
uint8_t SPS30::I2C_SetPointer_Read(uint8_t cnt, bool chk_zero)
{
    uint8_t ret;

    // set pointer
    ret = I2C_SetPointer();
    if (ret != SPS30ERR_OK) {
      SPS30LOGD(F("Can not set pointer\n"));
      return(ret);
    }

    // read from Sensor
    ret = I2C_ReadToBuffer(cnt, chk_zero);

    SPS30LOGD(F("I2C Received: "));
    for(byte i = 0; i < _Receive_BUF_Length; i++) SPS30LOGD(F("%X "),_Receive_BUF[i]);
    SPS30LOGD(F("length: %d\n\n"),_Receive_BUF_Length);

    if (ret != SPS30ERR_OK) {
      SPS30LOGD(F("Error during reading from I2C: %X\n"), ret);
    }
    return(ret);
}

/**
 * @brief       : receive from Sensor with I2C communication
 * @param count : number of data bytes to expect
 * @param chk_zero :  check for zero termination ( Serial and product code)
 *  false : expect and rea all the data bytes
 *  true  : expect NULL termination and count is MAXIMUM data bytes
 *
 * return :
 * OK   SPS30ERR_OK
 * else error
 */
uint8_t SPS30::I2C_ReadToBuffer(uint8_t count, bool chk_zero)
{
    uint8_t data[3];
    uint8_t i, j;

    j = i = _Receive_BUF_Length = 0;

    // 2 data bytes  + crc
    Wire.requestFrom((uint8_t) SPS30_ADDRESS, uint8_t (count / 2 * 3));

    while (Wire.available()) { // wait till all arrive

        data[i++] = Wire.read();

	//Serial.print(i-1);
	//Serial.print(" : ");
	//Serial.println(data[i-1],HEX);
	
        // 2 bytes RH, 1 CRC
        if( i == 3) {

            if (data[2] != I2C_calc_CRC(&data[0])){
	      SPS30LOGD(F("I2C CRC error: Expected %X, calculated %X\n"),data[2], I2C_calc_CRC(&data[0]));
	      return(SPS30ERR_PROTOCOL);
            }

            if(_Receive_BUF_Length >= MAXRECVBUFLENGTH)
            {
	      SPS30LOGD(F("\nReceive buffer full\n"));
	      return(SPS30ERR_PROTOCOL);
            }
	    
            _Receive_BUF[_Receive_BUF_Length++] = data[0];
            _Receive_BUF[_Receive_BUF_Length++] = data[1];

            i = 0;

            // check for zero termination (Serial and product code)
            if (chk_zero) {
                if (data[0] == 0 && data[1] == 0) return(SPS30ERR_OK);
            }

            if (_Receive_BUF_Length >= count) break;
        }
    }

    if (i != 0) {
      SPS30LOGD(F("Error: Data counter %d\n"),i);
      while (j < i) _Receive_BUF[_Receive_BUF_Length++] = data[j++];
    }

    if (_Receive_BUF_Length == 0) {
      SPS30LOGD(F("Error: Received NO bytes\n"));
      return(SPS30ERR_PROTOCOL);
    }

    if (_Receive_BUF_Length == count) return(SPS30ERR_OK);

    SPS30LOGD(F("Error: Expected bytes : %d, Received bytes %d\n"), count,_Receive_BUF_Length);
    return(SPS30ERR_DATALENGTH);
}

/**
 * @brief :check for data ready
 *
 * Return
 *  true  if available
 *  false if not
 */
bool SPS30::I2C_Check_data_ready()
{
   I2C_fill_buffer(I2C_READ_DATA_RDY_FLAG);

   if (I2C_SetPointer_Read(2) != SPS30ERR_OK) return(false);

   if (_Receive_BUF[1] == 1) return(true);
   return(false);
}

/**
 * @brief : calculate CRC for I2c comms
 * @param data : 2 databytes to calculate the CRC from
 *
 * Source : datasheet SPS30
 *
 * return CRC
 */
uint8_t SPS30::I2C_calc_CRC(uint8_t data[2])
{
    uint8_t crc = 0xFF;
    for(int i = 0; i < 2; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31u;
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

/*
// sample code by sensirion
uint8_t SHT3X_CalcCrc(uint8_t data[], uint8_t nbrOfBytes)
{
  uint8_t bit;        // bit mask
  uint8_t crc = 0xFF; // calculated checksum
  uint8_t byteCtr;    // byte counter
  
  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
  {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else           crc = (crc << 1);
    }
  }
  
  return crc;
}
*/


#endif // INCLUDE_I2C
