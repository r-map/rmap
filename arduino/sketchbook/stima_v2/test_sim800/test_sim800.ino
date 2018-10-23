/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include "sim800b.h"

SIM800 s800;
char buf[200];

void setup()
{

  //Serial connection.
  Serial.begin(9600);
  Serial.println("SIM800 Shield testing.");

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

#ifdef HARDWARESERIAL
    if (s800.init( 7, 6)) break;
#else
    if (s800.init(&Serial1 , 7, 6)) break;
#endif

  }

  Serial.println("try to setup sim800");
  s800.setup();

  // at command examples
  //s800.ATcommand("+CLTS=1",buf);
  //s800.ATcommand("+CENG=3",buf);
  //s800.ATcommand("+CCLK=\"yy/MM/dd,hh:mm:ss±zz\"",buf);
  //s800.ATcommand("+CCLK=\"15/04/27,16:12:00+00\"",buf);

};

void loop()
{  
  Serial.println("null loop");

  #define RESULTL 512
  char result[RESULTL];

  Serial.println(millis());

  if(s800.httpGET("rmap.cc", 80, "/http2mqtt/?topic=mobile/user/test&payload=ciao&user=user&password=password", result, RESULTL)){

    Serial.println("-----------------<>-------------------");
    Serial.println(result);
    Serial.println("-----------------<>-------------------");
    Serial.println(millis());

  }else{
    Serial.println(">>>>>>>>>>>>>  errore httpGET");

    if (!s800.checkNetwork()){
      Serial.println("#GSM try to restart network");
      s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
    }
    if (!s800.checkNetwork()){
      Serial.println("#GSM try to restart sim800");

      //s800.switchOn();

      // fast restart
      if (s800.init_onceautobaud()){
	if (s800.setup()){
	  s800.ATcommand("+CLTS=1",buf);
	  s800.ATcommand("+CENG=3",buf);
	  s800.stopNetwork();
	  s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
	}
      }
    }

    s800.init_onceautobaud();
    s800.setup();
    s800.stopNetwork();
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  }

  delay(10000);

};
