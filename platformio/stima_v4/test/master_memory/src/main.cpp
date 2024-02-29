/**
  ******************************************************************************
  * @file    main.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Bootloader Application for StimaV4 Slave & MPPT
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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

  Descrizione Funzionamento del BOOT Loader:

  In avvio parte il bootLoader (questo applicativo) Se non necessario upload
  viene avviato l'applicativo standard APP all' indirizzo conosciuto con Jump

  Descrizione:
  All'avvio viene controllato un Bytes Flag della EEProm (settato dall'applicazione CAN)
  e un secondo flag che contiene le informazioni di firmware first boot OK

  Se il primo flag contiene un valore di aggiornamento allora viene controllato
  la presenza del file di firmware con il nome e versione e spazio consentito...
  Se OK continua, altrimenti esce aggiornando i flag di BOOT Loader annullando l'operazione

  se tutto OK, parte il backup del firmware corrente salvato in zona Flash conosciuta
  Se OK continua, altrimenti esce aggiornando i flag di BOOT Loader annullando l'operazione
  A questo punto viene resettato il flag di APP_EXECUTED_OK (applicazione run OK)
  che indica se il programma si è avviato correttamente. Questo flag unito ai precedenti
  comunica al bootloader in caso di riavvio anomalo che qualcosa è andato KO durante
  il flashing o che il programma caricato non ha effettuato un avvio corretto
  In questa condizione parte il rollback alla versione precedente salvata prima
  del caricamento del nuovo firmware

  Il boot finisce e parte l'applicativo come sopra jump + vectorTable setup...

  Dall'applicativo principale se l'avvio è corretto viene resettato il flag
  di first bootloader, altrimenti se tutto non parte correttamente e si riavvia
  il boot loader, boot loader troverà questo flag attivo ed effetuerà l'operazione di
  rollback del firmware precedente... (Descritto in precedenza)

*/

#include <Arduino.h>
// HW Configuration and module driver
#include "drivers/module_master_hal.h"
#include "drivers/flash.h"
#include "drivers/eeprom.h"

// HW Class Variables access
Flash       memFlash;
EEprom      memEprom;

// Setup Wire I2C Interface
void init_wire()
{
#if (ENABLE_I2C1)
  Wire.begin();
  Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
  Wire2.begin();
  Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
}

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

    uint8_t data[100];

    bool test_fail = false;

    // STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
    SetupSystemPeripheral();
    init_wire();
    delay(100);

    // Setup EEprom and Flash memory Access
    memEprom=EEprom(&Wire2);
    memFlash=Flash(&hqspi);

    // Write 0..99 to element and Write to EEProm 
    for (int iCnt = 0; iCnt<100; iCnt++) {
        data[iCnt] = iCnt;
    }

    // Write EEProm
    memEprom.Write(0, data, 100);

    // Reset 0 to element and read from EEProm before compare 
    for (int iCnt = 0; iCnt<100; iCnt++) {
        data[iCnt] = 0;
    }

    // Read EEProm
    memEprom.Read(0, data, 100);

    // Check comapre is OK
    for (int iCnt = 0; iCnt<100; iCnt++) {
        if(data[iCnt] != iCnt) test_fail = true;
    }

    if (test_fail) {
        Serial.print("TEST FAIL");
    }

    // *************** FLASH ********************

    // Init if required (DeInit after if required PowerDown Module)
    if(memFlash.BSP_QSPI_Init() != Flash::QSPI_OK) {
        Serial.print("INIT FLASH TEST FAIL");
    }
    // Check Status Flash OK
    if (memFlash.BSP_QSPI_GetStatus()) {
        Serial.print("STATUS FLASH NOT READY TEST FAIL");
    }

    if(memFlash.BSP_QSPI_Erase_Block(0) != Flash::QSPI_OK) {
        Serial.print("STATUS FLASH NOT ERASED TEST FAIL");
    }

    // Write 0..99 to element and Write to EEProm 
    for (int iCnt = 0; iCnt<100; iCnt++) {
        data[iCnt] = iCnt;
    }
    // Read Name file, Version and Info
    memFlash.BSP_QSPI_Write(data, 0, 100);

    // Reset 0 to element and read from EEProm before compare 
    for (int iCnt = 0; iCnt<100; iCnt++) {
        data[iCnt] = 0;
    }

    memFlash.BSP_QSPI_Read(data, 0, 100);

    test_fail = false;

    // Check comapre is OK
    for (int iCnt = 0; iCnt<100; iCnt++) {
        if(data[iCnt] != iCnt) test_fail = true;
    }

    if (test_fail) {
        Serial.print("TEST FAIL");
    }

}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************
void loop(void)
{
}