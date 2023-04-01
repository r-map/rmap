/**
  ******************************************************************************
  * @file    sd_task.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   sd_task source file (SD SPI StimaV4)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

#define TRACE_LEVEL     SD_TASK_TRACE_LEVEL
#define LOCAL_TASK_ID   SD_TASK_ID

#include "tasks/sd_task.h"
#include "date_time.h"

#if (ENABLE_SD)

using namespace cpp_freertos;

SdTask::SdTask(const char *taskName, uint16_t stackSize, uint8_t priority, SdParam_t sdParam) : Thread(taskName, stackSize, priority), param(sdParam)
{
  // Start WDT controller and TaskState Flags
  TaskWatchDog(WDT_STARTING_TASK_MS);
  TaskState(SD_STATE_CREATE, UNUSED_SUB_POSITION, task_flag::normal);

  // Init val
  sdFlashPtr = 0;
  sdFlashBlock = 0;

  state = SD_STATE_INIT;
  Start();
};

#if (ENABLE_STACK_USAGE)
/// @brief local stack Monitor (optional)
void SdTask::TaskMonitorStack()
{
  u_int16_t stackUsage = (u_int16_t)uxTaskGetStackHighWaterMark( NULL );
  if((stackUsage) && (stackUsage < param.system_status->tasks[LOCAL_TASK_ID].stack)) {
    param.systemStatusLock->Take();
    param.system_status->tasks[LOCAL_TASK_ID].stack = stackUsage;
    param.systemStatusLock->Give();
  }
}
#endif

/// @brief local watchDog and Sleep flag Task (optional)
/// @param status system_status_t Status STIMAV4
/// @param lock if used (!=NULL) Semaphore locking system status access
/// @param millis_standby time in ms to perfor check of WDT. If longer than WDT Reset, WDT is temporanly suspend
void SdTask::TaskWatchDog(uint32_t millis_standby)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Update WDT Signal (Direct or Long function Timered)
  if(millis_standby)  
  {
    // Check 1/2 Freq. controller ready to WDT only SET flag
    if((millis_standby) < WDT_CONTROLLER_MS / 2) {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
    } else {
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::timer;
      // Add security milimal Freq to check
      param.system_status->tasks[LOCAL_TASK_ID].watch_dog_ms = millis_standby + WDT_CONTROLLER_MS;
    }
  }
  else
    param.system_status->tasks[LOCAL_TASK_ID].watch_dog = wdt_flag::set;
  param.systemStatusLock->Give();
}

/// @brief local suspend flag and positor running state Task (optional)
/// @param state_position Sw_Position (Local STATE)
/// @param state_subposition Sw_SubPosition (Optional Local SUB_STATE Position Monitor)
/// @param state_operation operative mode flag status for this task
void SdTask::TaskState(uint8_t state_position, uint8_t state_subposition, task_flag state_operation)
{
  // Local TaskWatchDog update
  param.systemStatusLock->Take();
  // Signal Task sleep/disabled mode from request (Auto SET WDT on Resume)
  if((param.system_status->tasks[LOCAL_TASK_ID].state == task_flag::suspended)&&
     (state_operation==task_flag::normal))
     param.system_status->tasks->watch_dog = wdt_flag::set;
  param.system_status->tasks[LOCAL_TASK_ID].state = state_operation;
  param.system_status->tasks[LOCAL_TASK_ID].running_pos = state_position;
  param.system_status->tasks[LOCAL_TASK_ID].running_sub = state_subposition;
  param.systemStatusLock->Give();
}

/// @brief Return a STIMA's name file data for archive value starting by current block of data time in epoch style (uint32)
/// @param time IN uint32 epoch datetime (in format RMAP of data to archive).
/// @param dirPrefix IN directory prefix (add to filename to create complete path and fileName).
/// @param nameFile OUT Complete name of file with path.
void SdTask::namingFileData(uint32_t time, char *dirPrefix, char* nameFile)
{
  uint32_t dayno = time / SECS_DAY;
  int year = EPOCH_YR;
  uint8_t month = 0;

  while (dayno >= YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  while (dayno >= _ytab[LEAPYEAR(year)][month]) {
    dayno -= _ytab[LEAPYEAR(year)][month];
    month++;
  }

  sprintf(nameFile, "%s/%04d_%02d_%02d.dat", dirPrefix, year, ++month, ++dayno);
}

void SdTask::Run()
{

  // Generic retry
  uint8_t retry;
  bool message_traced = false;
  bool is_getted_rtc;
  // Queue buffer for logging
  char logBuffer[LOG_PUT_DATA_ELEMENT_SIZE];
  char logIntest[23] = {0};
  // Data buffer for RMAP queue
  rmap_archive_data_t rmap_put_archive_data;
  rmap_get_request_t rmap_get_request;
  rmap_get_response_t rmap_get_response;
  // Name file for data append es. /data/2023_01_30.dat (RMAP File data are stored by Day)
  char rmap_file_name_wr[DATA_FILENAME_LEN] = {0};  // Name current Write File Data RMAP
  char rmap_file_name_rd[DATA_FILENAME_LEN] = {0};  // Name Current Read File Data RMAP (Get queue from MQTT/Supervisor Request)
  char rmap_file_name_check[DATA_FILENAME_LEN] = {0}; // Check control Name VAR (RMAP Data Day changed?)
  uint32_t rmap_pointer_seek; // Seek Absolute Position Pointer Read in File RMAP Queue Out
  uint32_t rmap_pointer_datetime; // Date Time Pointer Read in File RMAP Queue Out
  // Queue file put and get from external Task
  // Put From extern task to card ( Es. Receive firmware from http to SD )
  file_put_request_t file_put_request;
  file_put_response_t file_put_response;
  // Get from card to extern task ( Es. Transmit firmware from SD to CAN module )
  file_get_request_t file_get_request;
  file_get_response_t file_get_response;
  char remote_file_name[FILE_NAME_MAX_LENGHT];
  // Local Firmware check and update
  char data_block[SD_FW_BLOCK_SIZE];
  char stima_name[STIMA_MODULE_NAME_LENGTH];
  char local_file_name[FILE_NAME_MAX_LENGHT];
  Module_Type module_type;
  uint8_t module_type_cast, fw_version, fw_revision;
  bool fw_found;
  File rmapWrFile, rmapRdFile;    // File (RMAP Write Data Append and Read Data from External Task request)
  File logFile, putFile;          // File Log and Firmware Write INTO SD (From Queue TASK Extern)
  File getFile[BOARDS_COUNT_MAX]; // File for remote boards Multi simultaneous file server Reading (For Queue Task Extern)
  File dir, entry, tmpFile;       // Only used for Temp(shared Open Close Single Operation) or Access directory List

  // Start Running Monitor and First WDT normal state
  #if (ENABLE_STACK_USAGE)
  TaskMonitorStack();
  #endif
  TaskState(state, UNUSED_SUB_POSITION, task_flag::normal);

  // SD-CARD Setup PIN CS SD UPIN27
  pinMode(PIN_SPI_SS, OUTPUT);
  digitalWrite(PIN_SPI_SS, HIGH);

  while (true)
  {

    switch (state)
    {
    case SD_STATE_INIT:
      // Check SD or Resynch after Error
      if (SD.begin(PIN_SPI_SS, SPI_SPEED)) {
        TRACE_VERBOSE_F(F("SD Card slot ready -> SD_STATE_CHECK_SD\r\n"));
        state = SD_STATE_CHECK_SD;
        message_traced = false;
      } else {
        // Only one TRACE message... SD Not present
        if(!message_traced) {
          // SD Was NOT Ready... for System
          param.systemStatusLock->Take();
          param.system_status->flags.sd_card_ready = false;
          param.systemStatusLock->Give();
          TRACE_VERBOSE_F(F("SD Card waiting to begin\r\n"));
          message_traced = true;
        }
      }
      break;

    case SD_STATE_CHECK_SD:
      // Optional Trace Type of CARD... and Size
      // Check or create directory Structure...
      if(!SD.exists("firmware")) SD.mkdir("firmware");
      if(!SD.exists("log")) SD.mkdir("log");
      if(!SD.exists("data")) SD.mkdir("data");

      // Open/Create File data pointer... and check if SD Starting OK
      // Check data Pointer and create if not exist
      if(SD.exists("/data/pointer.dat")) {
        tmpFile = SD.open("/data/pointer.dat", O_RDONLY);
        if(tmpFile) {
          tmpFile.read(&rmap_pointer_datetime, sizeof(rmap_pointer_datetime));
          tmpFile.read(&rmap_pointer_seek, sizeof(rmap_pointer_seek));
          tmpFile.close();
          // At First Get Data Set Sync Pointer position with loaded param
          namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_rd);
          // Not opened? Open... in append (Normally close But ReOpen if Full Resync SD)
          if(rmapRdFile) rmapRdFile.close();
          // Set Current Pointer Position
          rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
          rmapRdFile.seek(rmap_pointer_seek);
        } else {
          // SD Pointer Error, general Openon first File...
          // Error. Send to system_stae and retry OPEN INIT SD
          state = SD_STATE_INIT;
          break;
        }
      } else {
        tmpFile = SD.open("/data/pointer.dat", O_RDWR | O_CREAT);
        if(tmpFile) {
          rmap_pointer_seek = 0;
          rmap_pointer_datetime = rtc.getEpoch();  // Init to Current Epoch
          // System status enter in data not ready for SENT (no data present)
          param.systemStatusLock->Take();
          param.system_status->flags.new_data_to_send = false;
          param.systemStatusLock->Give();
          tmpFile.write(&rmap_pointer_datetime, sizeof(rmap_pointer_datetime));
          tmpFile.write(&rmap_pointer_seek, sizeof(rmap_pointer_seek));
          tmpFile.close();
        } else {
          // SD Pointer Error, general Openon first File...
          // Error. Send to system_stae and retry OPEN INIT SD
          state = SD_STATE_INIT;
          break;
        }
      }

      // ***************************************************
      // SD Was Ready... for System Structure and Pointer OK
      // ***************************************************
      param.systemStatusLock->Take();
      param.system_status->flags.sd_card_ready = true;
      param.systemStatusLock->Give();

      // **********************************************************************************
      // Check firmware file present Type, model and version from list file in firmware dir
      // Full system_status inform struct with firmware present on SD CARD
      // **********************************************************************************
      dir = SD.open("/firmware");
      while(true) {
        entry = dir.openNextFile();
        if(!entry) break;
        // Found firmware file?
        entry.getName(local_file_name, FILE_NAME_MAX_LENGHT);
        if(checkStimaFirmwareType(local_file_name, &module_type_cast, &fw_version, &fw_revision)) {
          module_type = static_cast<Module_Type>(module_type_cast);
          getStimaNameByType(stima_name, module_type);
          // Update info Fw File module array (system_status)
          TRACE_INFO_F(F("SD: found firmware type: %s Ver %u.%u\r\n"), stima_name, fw_version, fw_revision);
          // Check if already module file are detected and processed (first occurance or update if version major found)
          fw_found = false;
          for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
            if(param.system_status->boards_update_avaiable[brd].module_type == module_type) {
              fw_found = true;
              break;
            }
          }
          // If Version> last file version or Version== and Revision >... Update module version/revision firmware avaiable struct
          for(uint8_t brd=0; brd<STIMA_MODULE_TYPE_MAX_AVAIABLE; brd++) {
            // First occurance or Found...            
            if( ((!fw_found)&&(param.system_status->boards_update_avaiable[brd].module_type == 0)) ||
                (param.system_status->boards_update_avaiable[brd].module_type==module_type)) {
              if((fw_version>param.system_status->boards_update_avaiable[brd].version) ||
                ((fw_version==param.system_status->boards_update_avaiable[brd].version)&&(fw_revision>param.system_status->boards_update_avaiable[brd].revision)))
                param.systemStatusLock->Take();
                param.system_status->boards_update_avaiable[brd].module_type = module_type;
                param.system_status->boards_update_avaiable[brd].version = fw_version;
                param.system_status->boards_update_avaiable[brd].revision = fw_revision;
                // Is this module (check direct if fw upgrade is avaiable with last version file present)
                if(param.configuration->module_type == module_type) {
                  if((fw_version > param.configuration->module_main_version) ||
                    ((fw_version == param.configuration->module_main_version) && (fw_revision > param.configuration->module_minor_version))) {
                    param.system_status->data_master.fw_upgradable = true;
                  }
                }
                param.systemStatusLock->Give();
              break;
            }
          }
        }
        entry.close();
      }
      dir.close();
      // **********************************************************************************

      TRACE_VERBOSE_F(F("SD Card init complete -> SD_STATE_WAITING_EVENT\r\n"));

      state = SD_STATE_WAITING_EVENT;
      break;

    case SD_STATE_WAITING_EVENT:

      // If System SLEEP...  Long WAIT
      // Else check all queue input
      // Go to response/action

      // TEST FW UPGRADE
      // Starting from LCD or Remote RPC Request
      if(0)
      {
        retry = 0;
        state = SD_UPLOAD_FIRMWARE_TO_FLASH;
        break;
      }

      // TEST CREATE DATA FILE TO GET/SET Pointer and GET DATA
      // Create file with fake data over 15 min. For Test SET Pointer, GET Data, Send MQTT, Queue Get Req/Resp
      if(0)
      {
        uint32_t dateTimePtrCreate = 1671494400ul; // Set create file from 20/12/2022... To Now()
        uint32_t dateTimeEpoch = rtc.getEpoch();
        // Fake data with all 0
        memset(&rmap_put_archive_data, 0, sizeof(rmap_put_archive_data));
        rmap_put_archive_data.module_type = Module_Type::th;
        while(dateTimePtrCreate<dateTimeEpoch) {
          namingFileData(dateTimePtrCreate, "/data", rmap_file_name_wr);
          if(strcmp(rmap_file_name_wr, rmap_file_name_check)) {
            if(rmapWrFile) rmapWrFile.close();
            // Create NEW File
            strcpy(rmap_file_name_check, rmap_file_name_wr);
            rmapWrFile = SD.open(rmap_file_name_wr, O_WRONLY | O_CREAT);
          }
          // Change data of block archive
          rmap_put_archive_data.date_time = dateTimePtrCreate;
          dateTimePtrCreate += 900; // Add 15 min.
          rmapWrFile.write(&rmap_put_archive_data, sizeof(rmap_put_archive_data));
          // WDT and Delay
          TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
          Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
        }
        rmapWrFile.close();
      }

      // *********************************************************
      //             Perform LOG WRITE append message 
      // *********************************************************
      // If element get all element from the queue and Put to SD
      // Typical Put of Logging are Time controlled from TASK (If queue are free into reasonable time LOG is pushed)
      // Log queue element is reasonable sized to avoid problems
      // File are always opened if Append for fast Access Operation
      // File can be opened simultaneously also readonly mode by another function es.Read/Print/Send INFO LOG
      is_getted_rtc = false;
      while(!param.dataLogPutQueue->IsEmpty()) {
        if(!is_getted_rtc) {
          // Get date time to Intest string to PUT (for this message session)
          is_getted_rtc = true;
          if(param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
            sprintf(logIntest, "%02d/%02d/%02d %02d:%02d:%02d.%03d ",
              rtc.getDay(), rtc.getMonth(), rtc.getYear(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
            param.rtcLock->Give();
          }
        }
        // Get message from queue
        if(param.dataLogPutQueue->Dequeue(logBuffer)) {
          // Put to SD ( APPEND File Always Opened with Flush Data )
          if(!logFile) logFile = SD.open("log/log.txt", O_RDWR | O_APPEND);
          if(logFile) {          
            logFile.print(logIntest);
            logFile.write(logBuffer, strlen(logBuffer) < LOG_PUT_DATA_ELEMENT_SIZE ? strlen(logBuffer) : LOG_PUT_DATA_ELEMENT_SIZE);
            logFile.println();
            logFile.flush();
          }
        }
      }
      // *********************************************************
      //             End OF perform LOG append message
      // *********************************************************

      // *********************************************************
      //       Perform RMAP Write Data append get message
      // *********************************************************
      // If element get all element from the queue and Put to MMC
      // rmap_put_archive_data.date_time is epoch_style dateTime Archive record field
      // Check if data must be added into current day_file. If Day cahnged (nameing!=)
      // File data will be closed and reopened with the new name with control
      // File have to be opened and flushed for fast write access operation
      // Reading file ReadOnly can work simultaneously with opendFile.
      // Pointer Read from other TASK (Read Archive Data) work reading file when is WR/Open
      while(!param.dataRmapPutQueue->IsEmpty()) {
        // Get message from queue
        if(param.dataRmapPutQueue->Dequeue(&rmap_put_archive_data)) {
          // Put to MMC ( APPEND to File in Native Format. Check naming file )
          namingFileData(rmap_put_archive_data.date_time, "/data", rmap_file_name_check);
          // Day Name File Changed (Data is to save in New File?)
          if(strcmp(rmap_file_name_wr, rmap_file_name_check)) {
            // Save new file_name for next control
            strcpy(rmap_file_name_wr, rmap_file_name_check);
            // Not opened? Open... in append
            if(rmapWrFile) rmapWrFile.close();
            rmapWrFile = SD.open(rmap_file_name_wr, O_RDWR | O_APPEND);
          }
          // All correct... Write Block of data
          if(rmapWrFile) {
            rmapWrFile.write(&rmap_put_archive_data, sizeof(rmap_put_archive_data));
            rmapWrFile.flush();
            // System status enter in data ready for SENT (new data present)
            param.systemStatusLock->Take();
            param.system_status->flags.new_data_to_send = true;
            param.systemStatusLock->Give();
          }
        }
      }
      // *********************************************************
      //         End OF perform RMAP Write append message
      // *********************************************************

      // *********************************************************
      //         Perform FILE (DATA RMAP) READ data block
      // *********************************************************
      // External request RMAP data block (Cypal DSDL Format)
      // Get Block and send with queue to REQUEST (MQTT Task) Get Data Update command
      // Set request with 2 options standard do_get_data (get next data avaiable if exist)
      // Option 2 do_synch_ptr, search file_name and pointer from request to now()
      // Each operation modify current pointer stored in a file for prepare standard do_get_data
      while(!param.dataRmapGetRequestQueue->IsEmpty()) {
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        // Request Pointer SET Modify rmap_file_name_rd and current Opened File for Reading Data from External QUEUE Request
        // Get Pointer, Get Data from File Opened. If Data Change File Day Archive (Data is Next Day From last request)
        // rmap_file_name_rd automatic close and reopen with New Day Archive. Data Are Opened in ReadOnlyMode
        // Resynch file are security made when New Data avaiable In Write File (system_status->new data avaiable)
        if(param.dataRmapGetRequestQueue->Dequeue(&rmap_get_request)) {
          // Locking data session (Get Request Operation)
          memset(&rmap_get_response, 0, sizeof(rmap_get_response));
          // ******************************************************************
          //           Request is set pointer to date/time?
          // ******************************************************************
          if(rmap_get_request.command.do_synch_ptr) {
            bool is_found = false;
            char rmap_file_name_new[DATA_FILENAME_LEN]; // Work with temp Name file (SET in Pointer only if all right)
            uint32_t dateTimeSearch = rmap_get_request.param;
            // Trace INFO Queue Request SET Pointer TO->
            DateTime rmap_date_time_val;
            convertUnixTimeToDate(dateTimeSearch, &rmap_date_time_val);
            TRACE_INFO_F(F("Data RMAP requested search pointer date/time at [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
            // Check name File            
            namingFileData(dateTimeSearch, "/data", rmap_file_name_new);
            // If Exist, search pointer (correct position) into file
            // Search block dateTime to synch pointer requested
            if(SD.exists(rmap_file_name_new)) {
              // Request Name File EXIST
              // Found OK
              is_found = true;
              // Reset current dateTime control position
              uint32_t currReadDateTimeFile = 0;
              // Search pointer into file... (Open as a Temp File)
              tmpFile = SD.open(rmap_file_name_new, O_RDONLY);
              // Correctly opened..
              if(tmpFile) {
                // Search wile dateTime block into file are >= to requested dateTime block
                while(true) {
                  // Operation perform non blocking TASK
                  TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                  Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                  // Save position before read dateTime (set back)
                  uint32_t peek_rmap_pointer = tmpFile.curPosition();
                  // No more data avaiable?... Not Found
                  if(!tmpFile.available()) {
                    // EOF Not found, but searching procedure are correct 
                    // Pointer requested is over last data. Set Value to CurrentPosition (DateTime to Request)
                    rmap_pointer_seek = peek_rmap_pointer;
                    rmap_pointer_datetime = dateTimeSearch;
                    // Procedure can go right (response... no more data avaiable)
                    break;
                  }
                  // Read block RMAP (to check dateTime)
                  int bytes_readed = tmpFile.read(&rmap_get_response.rmap_data, sizeof(rmap_get_response.rmap_data));
                  #if (ENABLE_STACK_USAGE)
                  TaskMonitorStack();
                  #endif
                  // Block read size is correct
                  if(bytes_readed == sizeof(rmap_get_response.rmap_data)) {
                    // Get dateTime of block
                    currReadDateTimeFile = rmap_get_response.rmap_data.date_time;
                    // Check if block DateTime is found
                    convertUnixTimeToDate(currReadDateTimeFile, &rmap_date_time_val);
                    TRACE_VERBOSE_F(F("Data RMAP current searching date/time (Readed) [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
                    if(currReadDateTimeFile >= dateTimeSearch) {
                      // Found first dateTime block compilant with initial position read (peek...)
                      rmap_pointer_seek = peek_rmap_pointer;
                      rmap_pointer_datetime = currReadDateTimeFile;
                      break;
                    }
                  } else {
                    // Error readed block not correctly dimensioned
                    file_get_response.error_operation = true;
                    break;
                  }
                }
                tmpFile.close();
              } else {
                // Error opening file
                file_get_response.error_operation = true;
              }
            } else {
              // Request Name File NOT EXIST (Search another file in date sequence)
              // Reading Current epoch to STOP Searching (No data avaiable in the future)
              // Stop on first data found over requested date pointer
              uint32_t currEpochLimitCheck;
              if (param.rtcLock->Take(Ticks::MsToTicks(RTC_WAIT_DELAY_MS))) {
                currEpochLimitCheck = rtc.getEpoch();
                param.rtcLock->Give();
              }
              char rmap_file_name_new[DATA_FILENAME_LEN]; // Work with temp Name file (SET in Pointer only if all right)
              dateTimeSearch = (dateTimeSearch / SECS_DAY) * SECS_DAY; // Around to DataeTime Hour 00:00:00
              while(true) {
                // Operation perform non blocking TASK
                TaskWatchDog(TASK_WAIT_REALTIME_DELAY_MS);
                Delay(Ticks::MsToTicks(TASK_WAIT_REALTIME_DELAY_MS));
                #if (ENABLE_STACK_USAGE)
                TaskMonitorStack();
                #endif
                // Add time second day -> set Next Epoch Day
                // If found, seek pointer are set to first block of data
                // because the requested date is necessarily higher
                dateTimeSearch += SECS_DAY;                
                convertUnixTimeToDate(dateTimeSearch, &rmap_date_time_val);
                TRACE_VERBOSE_F(F("Data RMAP current searching date/time (Not readed) [ %s ]\r\n"), formatDate(&rmap_date_time_val, NULL));
                namingFileData(dateTimeSearch, "/data", rmap_file_name_new);
                // Exist?
                if(SD.exists(rmap_file_name_new)) {
                  // FOUND FILE NEXT DATE
                  is_found = true;
                  // Real DateTime Pointer will be set on First GetData. DataPtr is setted to Day_00:00:00
                  rmap_pointer_datetime = dateTimeSearch;
                  rmap_pointer_seek = 0;
                  break;
                } else {
                  // Exit when date_time is > now()
                  // No data found...
                  // No modify date_time pointer RMAP
                  if(dateTimeSearch >= currEpochLimitCheck) break;
                }
              }
            }
            // Found file and position correct?...
            if((!is_found)||(rmap_get_response.result.event_error)) {
              // Error procedure... or Not Found
              TRACE_VERBOSE_F(F("Data RMAP current searching date/time FOUND [ %s ]\r\n"), ERROR_STRING);
              rmap_get_response.result.event_error = true;
            } else {
              // Responding data pointer Setted
              TRACE_VERBOSE_F(F("Data RMAP current searching date/time FOUND [ %s ]\r\n"), OK_STRING);
              rmap_get_response.result.done_synch = true;
            }
            // All OK?
            if(rmap_get_response.result.done_synch) {
              // System status enter in data ready for SENT (new data present)
              param.systemStatusLock->Take();
              param.system_status->flags.new_data_to_send = true;
              param.systemStatusLock->Give();
            }
            // ***** Send response to request *****
            param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response, 0);
          }
          // ******************************************************************
          // Request next avaiable data? ( N.B. Standard Request for GET DATA )
          // ******************************************************************
          else if(rmap_get_request.command.do_get_data) {
            namingFileData(rmap_pointer_datetime, "/data", rmap_file_name_check);
            // Day Name File Changed (Data is to save in New File?)
            if(strcmp(rmap_file_name_rd, rmap_file_name_check)) {
              // Save new file_name for next control
              strcpy(rmap_file_name_rd, rmap_file_name_check);
              // Not opened? Open... in append
              if(rmapRdFile) rmapRdFile.close();
              rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
            }
            memset(&rmap_get_response, 0, sizeof(rmap_get_response));
            if(rmapRdFile) {
              // Not avaiable, EOF...
              if(rmapRdFile.available()) {
                // Not read size correct block?... Error
                int bytes_readed = rmapRdFile.read(&rmap_get_response.rmap_data, sizeof(rmap_get_response.rmap_data));
                if(bytes_readed == sizeof(rmap_get_response.rmap_data)) {
                  // CurPosition Check assert(bytes_readed+=sizeof(rmap_get_response.rmap_data))
                  rmap_pointer_seek = rmapRdFile.curPosition();
                  rmap_get_response.result.done_get_data = true;
                  // Set DateTime Local Pointer correct
                  rmap_pointer_datetime = rmap_get_response.rmap_data.date_time;
                  // Send an EOF with a block data if last block
                  if(!rmapRdFile.available()) {
                    // Check if another Day (Next) is present before sending End Of Data
                    // If Exist The Seek Pointer Have to be resetted to Init Value (First Data of New File)
                    namingFileData(rmap_pointer_datetime + SECS_DAY, "/data", rmap_file_name_check);
                    // Not Exist? End Of Data, Otherwise next request in New Day Direct open Day File without other operation
                    if(SD.exists(rmap_file_name_check)) {
                      // Reopen Operation can be Start Immediatly.
                      // Set SEEK Position to Start File and DateTime to hh:nn:ss at 0.0.0 Begin of Day
                      rmap_pointer_seek = 0;
                      rmap_pointer_datetime = ((rmap_pointer_datetime + SECS_DAY) / SECS_DAY) * SECS_DAY;
                      // Save new file_name for next control
                      strcpy(rmap_file_name_rd, rmap_file_name_check);
                      // Not opened? Open... in append
                      if(rmapRdFile) rmapRdFile.close();
                      rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
                    } else {
                      rmap_get_response.result.end_of_data = true;
                      // No more data avaiable
                      param.systemStatusLock->Take();
                      param.system_status->flags.new_data_to_send = false;
                      param.systemStatusLock->Give();
                    }
                  }
                } else {
                  // Error readed block not correctly dimensioned
                  rmap_get_response.result.event_error = true;
                }
              } else {
                  // Check if another Day (Next) is present before sending End Of Data
                  // If Exist The Seek Pointer Have to be resetted to Init Value (First Data of New File)
                  namingFileData(rmap_pointer_datetime + SECS_DAY, "/data", rmap_file_name_check);
                  // Not Exist? End Of Data, Otherwise next request in New Day Direct open Day File without other operation
                  if(SD.exists(rmap_file_name_check)) {
                    // Reopen Operation can be Start Immediatly.
                    rmap_pointer_seek = 0;
                    // Save new file_name for next control
                    strcpy(rmap_file_name_rd, rmap_file_name_check);
                    // Not opened? Open... in append
                    if(rmapRdFile) rmapRdFile.close();
                    rmapRdFile = SD.open(rmap_file_name_rd, O_RDONLY);
                  } else {
                    rmap_get_response.result.end_of_data = true;
                    // No more data avaiable
                    param.systemStatusLock->Take();
                    param.system_status->flags.new_data_to_send = false;
                    param.systemStatusLock->Give();
                  }
              }
            } else {
              // Error on open file
              rmap_get_response.result.event_error = true;
            }
            // ***** Send response to request *****
            param.dataRmapGetResponseQueue->Enqueue(&rmap_get_response, 0);
          }
          // ******************************************************************
          //        Request is save current Seek and DateTime pointer
          // ******************************************************************
          // Non esclusive command (Not else_if) Save PTR Can Be executed all request
          // But for Fast Speed we can Call this function on End of Data Transmit
          // Or if call down and data cannot end process upload (From extern)
          if(rmap_get_request.command.do_save_ptr) {
            // Rewrite Pointer Data File (Open only at startup for Set Position)
            tmpFile = SD.open("/data/pointer.dat", O_RDWR | O_CREAT);
            if(tmpFile) {
              tmpFile.write(&rmap_pointer_datetime, sizeof(rmap_pointer_datetime));
              tmpFile.write(&rmap_pointer_seek, sizeof(rmap_pointer_seek));
              tmpFile.close();
            }
          }
        }
      }
      // *********************************************************
      //       END Perform FILE (DATA RMAP) READ data block
      // *********************************************************

      // *********************************************************
      //       Perform FILE (FIRMWARE) WRITE append message
      // *********************************************************
      // If element get all element from the queue and Put to SD N.B. If session running (Uploading...) Name File != NULL
      // If remote_file_name != NULL remote_file_name is not ready to system (Put firmware into module local and remote)
      // Any request for file (es. Cypal Request firmware are blocked)
      while(!param.dataFilePutRequestQueue->IsEmpty()) {
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        if(param.dataFilePutRequestQueue->Dequeue(&file_put_request)) {
          // Put to SD ( CREATE / APPEND Firmware Block File session )
          if(file_put_request.block_type == file_block_type::file_name) {
            // Get File name set file name Upload (session current START)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            strcpy(remote_file_name, "/firmware/");
            memcpy(remote_file_name + strlen(remote_file_name), file_put_request.block, file_put_request.block_lenght);
            // Create File in ReWrite Mode
            // Locking file session (uploading...)
            memset(&file_put_response, 0, sizeof(file_put_response));
            // Open Put File
            putFile = SD.open(remote_file_name, O_RDWR | O_CREAT | O_TRUNC);
            if(!putFile)
              file_put_response.done_operation = false;
            else
              file_put_response.done_operation = true;
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&file_put_response, 0);
          } else if(file_put_request.block_type == file_block_type::data_chunck) {
            memset(&file_put_response, 0, sizeof(file_put_response));
            if(putFile) {
              uint16_t writtenBytes;
              writtenBytes = putFile.write(file_put_request.block, file_put_request.block_lenght);
              // Bytes written is ok)
              if(writtenBytes == file_put_request.block_lenght)
                file_put_response.done_operation = true;
              else
                file_put_response.error_operation = true;
            } else {
              file_put_response.error_operation = true;
            }
            // Send response to caller
            param.dataFilePutResponseQueue->Enqueue(&file_put_response, 0);
          } else if(file_put_request.block_type == file_block_type::end_of_file) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system (without integrity control)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // Send response to caller ... OK done
            putFile.close();
            memset(&file_put_response, 0, sizeof(file_put_response));
            file_put_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&file_put_response, 0);
          } else if(file_put_request.block_type == file_block_type::ctrl_checksum) {
            // Remove file name Upload (session current END)
            // Unlock session. File is ready for the system
            // Need to control checksum (if any error file have to delete)
            memset(remote_file_name, 0, sizeof(remote_file_name));
            // TODO: checksum... other control file
            // Send response to caller ... OK done
            memset(&file_put_response, 0, sizeof(file_put_response));
            file_put_response.done_operation = true;
            param.dataFilePutResponseQueue->Enqueue(&file_put_response, 0);
          }
        }
      }
      // *********************************************************
      //        End OF FILE (FIRMWARE) WRITE append message
      // *********************************************************

      // *********************************************************
      //        Perform FILE (FIRMWARE) READ block message
      // *********************************************************
      // External request Firmware data block (Cypal...)
      // Get Block and send with queue to CAN and CAN Upload remote module... FW/File Update command
      // Flag Firmware (Receive from Network) toBe Completed before request to upload can start
      while(!param.dataFileGetRequestQueue->IsEmpty()) {
        // Try Get message from queue (Start, progress session download fron NETWORK TASK and push to SD CARD)
        // Send response -> system_reesponse generic mode to request
        if(param.dataFileGetRequestQueue->Dequeue(&file_get_request)) {
          // Locking file session (uploading...)
          memset(&file_get_response, 0, sizeof(file_get_response));
          // Put to SD ( Start Read Firmware Block File session )
          if(file_get_request.block_id==0) {
            // Closing if file already used for other Session (preserve memoryLeak error)
            if(getFile[file_get_request.board_id]) getFile[file_get_request.board_id].close();
            // Get File name set file name Upload (session current START)
            memset(local_file_name, 0, sizeof(local_file_name));
            strcpy(local_file_name, "/firmware/");
            strcat(local_file_name + strlen(local_file_name), file_get_request.file_name);
            // Create File in ReWrite Mode
            // Open Get File (for reading)
            getFile[file_get_request.board_id] = SD.open(local_file_name, O_RDONLY);
            if(getFile[file_get_request.board_id]) {
              // Read the first block data (return number of bytes read)
              file_get_response.done_operation = true;
              uint32_t avaiables = getFile[file_get_request.board_id].available();
              if(avaiables > FILE_GET_DATA_BLOCK_SIZE) avaiables = FILE_GET_DATA_BLOCK_SIZE;
              file_get_response.block_lenght = getFile[file_get_request.board_id].readBytes(file_get_response.block, avaiables);
            } else
              file_get_response.done_operation = false;
            // Send response to caller
            // Closing automatic of file if block not completed (EOF)
            if(file_get_response.block_lenght != FILE_GET_DATA_BLOCK_SIZE) {
              // Rapid return, before closing file
              param.dataFileGetResponseQueue->Enqueue(&file_get_response, 0);
              getFile[file_get_request.board_id].close();
              // Unlock file (clear name)
              memset(local_file_name, 0, sizeof(local_file_name));
            } else
              param.dataFileGetResponseQueue->Enqueue(&file_get_response, 0);
          } else {
            // Set Seek Position or Reading Next Block (only if block_read_next not called)
            if(!file_get_request.block_read_next) 
              getFile[file_get_request.board_id].seek(FILE_GET_DATA_BLOCK_SIZE * file_get_request.block_id);
            // Read the first block data (return number of bytes read)
            file_get_response.done_operation = true;
            uint32_t avaiables = getFile[file_get_request.board_id].available();
            if(avaiables > FILE_GET_DATA_BLOCK_SIZE) avaiables = FILE_GET_DATA_BLOCK_SIZE;
            file_get_response.block_lenght = getFile[file_get_request.board_id].readBytes(file_get_response.block, avaiables);
            // Closing automatic of file if block not completed (EOF)
            if(file_get_response.block_lenght != FILE_GET_DATA_BLOCK_SIZE) {
              // Rapid return before closing file
              param.dataFileGetResponseQueue->Enqueue(&file_get_response, 0);
              getFile[file_get_request.board_id].close();
              // Unlock file (clear name)
              memset(local_file_name, 0, sizeof(local_file_name));
            } else
              // Send response to caller
              param.dataFileGetResponseQueue->Enqueue(&file_get_response, 0);
          }
        }
      }
      // *********************************************************
      //        End OF FILE (FIRMWARE) WRITE append message
      // *********************************************************

      break;

    case SD_UPLOAD_FIRMWARE_TO_FLASH:
      state = SD_STATE_WAITING_EVENT;
      break;

    case SD_STATE_ERROR:
      // Gest Error... Resynch SD
      TRACE_VERBOSE_F(F("SD_STATE_ERROR -> SD_STATE_INIT\r\n"));
      state = SD_STATE_INIT;
      break;
    }

    #if (ENABLE_STACK_USAGE)
    TaskMonitorStack();
    #endif

    // One step base non blocking switch
    TaskWatchDog(SD_TASK_WAIT_DELAY_MS);
    Delay(Ticks::MsToTicks(SD_TASK_WAIT_DELAY_MS));

  }
}

#endif