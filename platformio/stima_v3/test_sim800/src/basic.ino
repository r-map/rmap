
#define LOG_LEVEL LOG_LEVEL_TRACE

#include <gsm_config.h>
#include <sim800Client.h>
#include <ArduinoLog.h>

#define SIM800_ON_OFF_PIN     (5)
#define SIM800_GSM_APN        (GSM_APN_TIM)
#define SIM800_GSM_USERNAME   ("")
#define SIM800_GSM_PASSWORD   ("")

typedef enum {
   GSM_INIT,                  //!< init task variables
   GSM_SWITCH_ON,             //!< gsm power on
   GSM_AUTOBAUD,              //!< gsm autobaud procedure
   GSM_SETUP,                 //!< gsm setup
   GSM_START_CONNECTION,      //!< gsm open connection
   GSM_CHECK_OPERATION,       //!< check operations (ntp or mqtt)
   GSM_OPEN_UDP_SOCKET,       //!< open udp socket for ntp sync
   GSM_SUSPEND,               //!< wait other tasks for complete its operations with gsm
   GSM_STOP_CONNECTION,       //!< gsm close connection
   GSM_WAIT_FOR_SWITCH_OFF,   //!< wait gsm for power off
   GSM_SWITCH_OFF,            //!< gsm power off
   GSM_END,                   //!< performs end operations and deactivate task
   GSM_WAIT_STATE             //!< non-blocking waiting time
} gsm_state_t;

sim800Client s800;
bool is_event_gsm;
bool is_client_connected;
bool is_client_udp_socket_open;
bool is_event_client_executed;
uint8_t ready_tasks_count;
gsm_state_t gsm_state;

#ifndef ARDUINO_ARCH_AVR
HardwareSerial Serial1(PB11, PB10);
#endif


void gsm_task() {
   static gsm_state_t state_after_wait;
   static uint32_t delay_ms;
   static uint32_t start_time_ms;
   static bool is_error;
   sim800_status_t sim800_status;
   uint8_t sim800_connection_status;
   static uint8_t power_off_mode = SIM800_POWER_OFF_BY_SWITCH;

   switch (gsm_state) {
   case GSM_INIT:
     
     is_error = false;
     is_client_connected = false;
     sim800_connection_status = 0;
     state_after_wait = GSM_INIT;
     gsm_state = GSM_SWITCH_ON;
     LOGT(F("GSM_SWITCH_ON"));
     break;
     
   case GSM_SWITCH_ON:
     sim800_status = s800.switchOn();
     
     // success
     if (sim800_status == SIM800_OK) {
       gsm_state = GSM_AUTOBAUD;
       LOGT(F("GSM_AUTOBAUD"));     
     }
     else if (sim800_status == SIM800_ERROR) {
	  gsm_state = GSM_END;
	  LOGT(F("GSM_END"));     
     }
     // wait...
     break;
     
   case GSM_AUTOBAUD:
     sim800_status = s800.initAutobaud();
     
     // success
     if (sim800_status == SIM800_OK) {
       delay_ms = SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS;
       start_time_ms = millis();
       state_after_wait = GSM_SETUP;
       gsm_state = GSM_WAIT_STATE;
       LOGT(F("GSM_WAIT_STATE"));
     }
     // fail
     else if (sim800_status == SIM800_ERROR) {
       gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
       LOGT(F("GSM_WAIT_FOR_SWITCH_OFF"));
     }
     // wait...
     break;
     
   case GSM_SETUP:
     sim800_status = s800.setup();
     
     // success
     if (sim800_status == SIM800_OK) {
       gsm_state = GSM_START_CONNECTION;
       LOGT(F("GSM_START_CONNECTIO"));
     }
     // fail
     else if (sim800_status == SIM800_ERROR) {
       is_error = true;
       gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
       LOGT(F("GSM_WAIT_FOR_SWITCH_OFF"));
     }
     // wait...
     break;
     
   case GSM_START_CONNECTION:
     sim800_status = s800.startConnection(SIM800_GSM_APN, SIM800_GSM_USERNAME,SIM800_GSM_PASSWORD);
     
     // success
     if (sim800_status == SIM800_OK) {
       gsm_state = GSM_CHECK_OPERATION;
       LOGT(F("GSM_CHECK_OPERATION"));
     }
     // fail
     else if (sim800_status == SIM800_ERROR) {
       is_error = true;
       gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
       LOGT(F("GSM_WAIT_FOR_SWITCH_OFF"));       
     }
     // wait...
     break;
     
   case GSM_CHECK_OPERATION:
     // do nothing
     if (true) {
            gsm_state = GSM_OPEN_UDP_SOCKET;
	    LOGT(F("GSM_OPEN_UDP_SOCKET"));	    
     }
     // wait for mqtt send terminate
     else {
       gsm_state = GSM_SUSPEND;
       LOGT(F("GSM_SUSPEND"));
       state_after_wait = GSM_STOP_CONNECTION;
     }
     break;
     
   case GSM_OPEN_UDP_SOCKET:
     sim800_connection_status = s800.connection("UDP","it.pool.ntp.org" , 123);
     
     // success
     if (sim800_connection_status == 1) {
       is_client_udp_socket_open = true;
       is_client_connected = true;
       is_event_client_executed = true;
       state_after_wait = GSM_STOP_CONNECTION;
       gsm_state = GSM_SUSPEND;
       LOGT(F("GSM_SUSPEND"));
     }
     // fail
     else if (sim800_connection_status == 2) {
       is_client_connected = false;
       is_event_client_executed = true;
       is_error = true;
       gsm_state = GSM_WAIT_FOR_SWITCH_OFF;
       LOGT(F("GSM_WAIT_FOR_SWITCH_OFF"));
     }
     // wait
     break;
     
   case GSM_SUSPEND:
     is_client_connected = true;
     is_event_client_executed = true;
     gsm_state = state_after_wait;
     LOGT(F("GSM_%d"),gsm_state);
     noInterrupts();
     is_event_gsm = false;
     ready_tasks_count--;
     interrupts();
     break;
     
   case GSM_STOP_CONNECTION:
     sim800_status = s800.stopConnection();
     
     // success
     if (sim800_status == SIM800_OK) {
       gsm_state = GSM_SWITCH_OFF;
       LOGT(F("GSM_SWITCH_OFF"));
     }
     // fail
     else if (sim800_status == SIM800_ERROR) {
       is_error = true;
       gsm_state = GSM_SWITCH_OFF;
       LOGT(F("GSM_SWITCH_OFF"));
     }
     // wait
     break;
     
   case GSM_WAIT_FOR_SWITCH_OFF:
     delay_ms = SIM800_POWER_ON_TO_OFF_DELAY_MS;
     start_time_ms = millis();
     state_after_wait = GSM_SWITCH_OFF;
     gsm_state = GSM_WAIT_STATE;
     LOGT(F("GSM_WAIT_STATE"));
     break;
     
   case GSM_SWITCH_OFF:
     sim800_status = s800.switchOff(power_off_mode);
     
     // success
     if (sim800_status == SIM800_OK) {
       delay_ms = SIM800_WAIT_FOR_POWER_OFF_DELAY_MS;
       start_time_ms = millis();
       state_after_wait = GSM_END;
       gsm_state = GSM_WAIT_STATE;
       LOGT(F("GSM_WAIT_STATE"));
     }
     // fail
     else if (sim800_status == SIM800_ERROR) {
       if (power_off_mode == SIM800_POWER_OFF_BY_AT_COMMAND) {
	 power_off_mode = SIM800_POWER_OFF_BY_SWITCH;
       }
       else {
	 gsm_state = GSM_END;
	 LOGT(F("GSM_end"));
       }
     }
     // wait...
     break;
     
   case GSM_END:
     if (is_error) {
         }
     is_event_client_executed = true;
     is_client_connected = false;
     is_client_udp_socket_open = false;
     noInterrupts();
     is_event_gsm = false;
     ready_tasks_count--;
     interrupts();
     gsm_state = GSM_INIT;
     LOGT(F("GSM_INIT"));
     break;
     
   case GSM_WAIT_STATE:
     if (millis() - start_time_ms > delay_ms) {
       gsm_state = state_after_wait;
       LOGT(F("GSM_%d"),gsm_state);
     }
     break;
     
   }
}

void logPrefix(Print* _logOutput) {
  char m[12];
  sprintf(m, "%10lu ", millis());
  _logOutput->print("#");
  _logOutput->print(m);
  _logOutput->print(": ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL, &Serial);
  Log.setPrefix(logPrefix);
  Log.setSuffix(logSuffix);
  delay(10000);
  s800.init(SIM800_ON_OFF_PIN);
  //s800.setTimeout(1000);
  is_event_gsm = true;
  ready_tasks_count = 1;
  gsm_state = GSM_INIT;
  LOGT(F("GSM_INIT"));
}

void loop() {
  if (is_event_gsm) {
    gsm_task();
  }
}
