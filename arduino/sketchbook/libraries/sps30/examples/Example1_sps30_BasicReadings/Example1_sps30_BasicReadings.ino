/************************************************************************************
 *  Copyright (c) January 2019, version 1.0     Paul van Haastrecht
 *
 *  Version 1.1 Paul van Haastrecht
 *  - Changed the I2C information / setup.
 *
 *  =========================  Highlevel description ================================
 *
 *  This basic reading example sketch will connect to an SPS30 for getting data and
 *  display the available data
 *
 *  =========================  Hardware connections =================================
 *  /////////////////////////////////////////////////////////////////////////////////
 *  ## UART UART UART UART UART UART UART UART UART UART UART UART UART UART UART  ##
 *  /////////////////////////////////////////////////////////////////////////////////
 *
 *  Successfully tested on ATMEGA2560
 *  Used SerialPort2. No need to set/change RX or TX pin
 *  SPS30 pin     ATMEGA
 *  1 VCC -------- 5V
 *  2 RX  -------- TX2  pin 16
 *  3 TX  -------- RX2  pin 17
 *  4 Select      (NOT CONNECTED)
 *  5 GND -------- GND
 *
 *  Also tested on SerialPort1 and Serialport3 successfully
 *  .........................................................
 *  Failed testing on UNO
 *  Had to use softserial as there is not a separate serialport. But as the SPS30
 *  is only working on 115K the connection failed all the time with CRC errors.
 *
 *  Not tested ESP8266
 *  As the power is only 3V3 (the SPS30 needs 5V)and one has to use softserial
 *
 *  //////////////////////////////////////////////////////////////////////////////////
 *  ## I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C  I2C I2C I2C ##
 *  //////////////////////////////////////////////////////////////////////////////////
 *  NOTE 1:
 *  Depending on the Wire / I2C buffer size we might not be able to read all the values.
 *  The buffer size needed is at least 60 while on many boards this is set to 32. The driver
 *  will determine the buffer size and if less than 64 only the MASS values are returned.
 *  You can manually edit the Wire.h of your board to increase (if you memory is larg enough)
 *  One can check the expected number of bytes with the I2C_expect() call as in this example
 *  see detail document.
 *
 *  NOTE 2:
 *  As documented in the datasheet, make sure to use external 10K pull-up resistor on
 *  both the SDA and SCL lines. Otherwise the communication with the sensor will fail random.
 *
 *  ..........................................................
 *  Successfully tested on ESP32
 *
 *  SPS30 pin     ESP32
 *  1 VCC -------- VUSB
 *  2 SDA -------- SDA (pin 21)
 *  3 SCL -------- SCL (pin 22)
 *  4 Select ----- GND (select I2c)
 *  5 GND -------- GND
 *
 *  The pull-up resistors should be to 3V3
 *  ..........................................................
 *  Successfully tested on ATMEGA2560
 *
 *  SPS30 pin     ATMEGA
 *  1 VCC -------- 5V
 *  2 SDA -------- SDA
 *  3 SCL -------- SCL
 *  4 Select ----- GND  (select I2c)
 *  5 GND -------- GND
 *
 *  ..........................................................
 *  Successfully tested on UNO R3
 *
 *  SPS30 pin     UNO
 *  1 VCC -------- 5V
 *  2 SDA -------- A4
 *  3 SCL -------- A5
 *  4 Select ----- GND  (select I2c)
 *  5 GND -------- GND
 *
 *  When UNO-board is detected the UART code is excluded as that
 *  does not work on UNO and will save memory. Also some buffers
 *  reduced and the call to GetErrDescription() is removed to allow
 *  enough memory.
 *  ..........................................................
 *  Successfully tested on ESP8266
 *
 *  SPS30 pin     External     ESP8266
 *  1 VCC -------- 5V
 *  2 SDA -----------------------SDA
 *  3 SCL -----------------------SCL
 *  4 Select ----- GND --------- GND  (select I2c)
 *  5 GND -------- GND --------- GND
 *
 *  The pull-up resistors should be to 3V3 from the ESP8266.
 *
 *  ================================= PARAMETERS =====================================
 *
 *  From line 139 there are configuration parameters for the program
 *
 *  ================================== SOFTWARE ======================================
 *  Sparkfun ESP32
 *
 *    Make sure :
 *      - To select the Sparkfun ESP32 thing board before compiling
 *      - The serial monitor is NOT active (will cause upload errors)
 *      - Press GPIO 0 switch during connecting after compile to start upload to the board
 *
 *  ================================ Disclaimer ======================================
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  ===================================================================================
 *
 *  NO support, delivered as is, have fun, good luck !!
 */

#include "sps30.h"

/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid options:
 *   I2C_COMMS              use I2C communication
 *   SERIALPORT             ONLY IF there is NO monitor attached

 * NOTE: Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on any device. */
/////////////////////////////////////////////////////////////

#define SP30_COMMS I2C_COMMS

///////////////////////////////////////////////////////////////
/////////// NO CHANGES BEYOND THIS POINT NEEDED ///////////////
///////////////////////////////////////////////////////////////

// create constructor
SPS30 sps30;

void setup() {

  Serial.begin(115200);
  
  Wire.begin();
  
  Serial.println(F("Trying to connect\n"));

  // Begin communication channel;
  if (sps30.begin(SP30_COMMS) == false) {
    ErrtoMess("could not initialize communication channel.", 0);
  }

  // check for SPS30 connection
  if (sps30.probe() == false) {
    ErrtoMess("could not probe / connect with SPS30.", 0);
  }
  else
    Serial.println(F("Detected SPS30."));

  // reset SPS30 connection
  if (sps30.reset() == false) {
    ErrtoMess("could not reset.", 0);
  }

  // read device info
  GetDeviceInfo();

  if (SP30_COMMS == I2C_COMMS) {
    if (sps30.I2C_expect() == 4)
      Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
}

void loop() {
  // start measurement
  if (sps30.start() == true)
    Serial.println(F("Measurement started\n"));
  else
    ErrtoMess("Could NOT start measurement", 0);

  // 8 sec to start the fan  and 1 sec to measure
  
  delay(10000);
  read_all();

  if (sps30.stop() == true)
    Serial.println(F("Measurement stopped\n"));
  else
    ErrtoMess("Could NOT stop measurement", 0);

  delay(30000);
}

/**
 * @brief : read and display device info
 */
void GetDeviceInfo()
{
  char buf[32];
  uint8_t ret;

  //try to read serial number
  ret = sps30.GetSerialNumber(buf, 32);
  if (ret == ERR_OK) {
    Serial.print(F("Serial number : "));
    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess("could not get serial number", ret);

  // try to get product name
  ret = sps30.GetProductName(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Product name  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess("could not get product name.", ret);

  // try to get article code
  ret = sps30.GetArticleCode(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Article code  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess("could not get Article code .", ret);
}

/**
 * @brief : read and display all values
 */
bool read_all()
{
  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  ret = sps30.GetValues(&val);

  // data might not have been ready
  if (ret == ERR_DATALENGTH){

    ErrtoMess("Error during reading values: ",ret);
    return(false);
  }
  
  // if other error
  else if(ret != ERR_OK) {
    ErrtoMess("Error during reading values: ",ret);
    return(false);
  }
  
  // only print header first time
  if (header) {
    Serial.println(F("-------------Mass -----------    ------------- Number --------------   -Average-"));
    Serial.println(F("     Concentration [μg/m3]             Concentration [#/cm3]             [μm]"));
    Serial.println(F("P1.0\tP2.5\tP4.0\tP10\tP0.5\tP1.0\tP2.5\tP4.0\tP10\tPartSize\n"));
    header = false;
  }

  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(val.NumPM0);
  Serial.print(F("\t"));
  Serial.print(val.NumPM1);
  Serial.print(F("\t"));
  Serial.print(val.NumPM2);
  Serial.print(F("\t"));
  Serial.print(val.NumPM4);
  Serial.print(F("\t"));
  Serial.print(val.NumPM10);
  Serial.print(F("\t"));
  Serial.print(val.PartSize);
  Serial.print(F("\n"));
}

/**
 *  @brief : display error message
 *  @param mess : message to display
 *  @param r : error code
 *
 */
void ErrtoMess(char *mess, uint8_t r)
{
  char buf[80];

  Serial.print(mess);

  sps30.GetErrDescription(r, buf, 80);
  Serial.println(buf);
}

