#include <debug_config.h>

#define SERIAL_TRACE_LEVEL                (SERIAL_TRACE_LEVEL_OFF)
#define SIM800_SERIAL_TRACE_LEVEL         (SERIAL_TRACE_LEVEL_OFF)

#include <debug.h>
#include <gsm_config.h>
#include <sim800Client.h>

#define SIM800_ON_OFF_PIN     (5)
#define SIM800_GSM_APN        (GSM_APN_WIND)
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
      break;

      case GSM_SWITCH_ON:
         sim800_status = s800.switchOn();

         // success
         if (sim800_status == SIM800_OK) {
            gsm_state = GSM_END;
         }
         else if (sim800_status == SIM800_ERROR) {
            gsm_state = GSM_END;
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
      break;
   }
}

void setup() {
   SERIAL_BEGIN(115200);
   s800.init(SIM800_ON_OFF_PIN);
   is_event_gsm = true;
   ready_tasks_count = 1;
   gsm_state = GSM_INIT;
}

void loop() {
   if (is_event_gsm) {
      gsm_task();
   }

   // if (!is_event_gsm) {
   //    is_event_gsm = true;
   // }
}
