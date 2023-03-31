/**
  ******************************************************************************
  * @file    main.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Bootloader Application for StimaV4 Slave & MPPT
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
#include <IWatchdog.h>
#include <strings.h>
// HW Configuration and module driver
#include "drivers/eeprom.h"
#include "drivers/flash.h"
#include "drivers/module_slave_hal.h"
#include "unity.h"

#define DATA_LENGTH 100

// HW Class Variables access
Flash memFlash;
EEprom memEprom;

// Setup Wire I2C Interface
void init_wire() {
#if (ENABLE_I2C1)
    Wire.begin();
    Wire.setClock(I2C1_BUS_CLOCK_HZ);
#endif

#if (ENABLE_I2C2)
    Wire2.begin();
    Wire2.setClock(I2C2_BUS_CLOCK_HZ);
#endif
}

// Util variables
uint8_t data[DATA_LENGTH];

// Declaration test eeprom functions
void test_data_written_on_eeprom_is_correct(void);
void test_read_eeprom(void);
void test_write_eeprom(void);
// Declaration test flash functions
void test_data_written_on_flash_is_correct(void);
void test_erase_flash(void);
void test_init_qspi_hw(void);
void test_read_flash(void);
void test_write_flash(void);

/*** TEST EEPROM FUNCTION IMPLEMENTATIONS ***/

/**
 * @brief TEST Compare data
 *
 */
void test_data_written_on_eeprom_is_correct() {
    for (int iCnt = 0; iCnt < DATA_LENGTH; iCnt++) {
        if (data[iCnt] != iCnt) {
            TEST_ASSERT_TRUE(false);
            return;
        }
    }
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief TEST: Read 100 bytes from EEPROM
 *
 */
void test_read_eeprom() {
    // Reset 0 to element and read from EEProm before compare
    for (int iCnt = 0; iCnt < DATA_LENGTH; iCnt++) {
        data[iCnt] = 0;
    }
    TEST_ASSERT_TRUE(memEprom.Read(0, data, DATA_LENGTH));
}

/**
 * @brief TEST: Write 100 bytes to eeprom
 *
 */
void test_write_eeprom() {
    // Write 0..99 to element and Write to EEProm
    for (int iCnt = 0; iCnt < DATA_LENGTH; iCnt++) {
        data[iCnt] = iCnt;
    }
    TEST_ASSERT_TRUE(memEprom.Write(0, data, DATA_LENGTH));
}

/*** TEST FLASH FUNCTION IMPLEMENTATIONS ***/

/**
 * @brief TEST: Compare data
 *
 */
void test_data_written_on_flash_is_correct() {
    for (int iCnt = 0; iCnt < DATA_LENGTH; iCnt++) {
        if (data[iCnt] != iCnt) {
            TEST_ASSERT_TRUE(false);
            return;
        }
    }
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief TEST: Initialization of QSPI interface
 *
 */
void test_init_qspi_hw() {
    TEST_ASSERT_EQUAL(Flash::QSPI_OK, memFlash.BSP_QSPI_Init());
}

/**
 * @brief TEST: Erase block of flash memory
 *
 */
void test_erase_flash() {
    TEST_ASSERT_EQUAL(Flash::QSPI_OK, memFlash.BSP_QSPI_Erase_Block(0));
}

/**
 * @brief TEST: Write 100 bytes to flash
 *
 */
void test_read_flash() {
    // Reset 0 to element and read from EEProm before compare
    for (int iCnt = 0; iCnt < 100; iCnt++) {
        data[iCnt] = 0;
    }
    TEST_ASSERT_EQUAL(Flash::QSPI_OK, memFlash.BSP_QSPI_Read(data, 0, 100));
}

/**
 * @brief TEST: Read 100 bytes from EEPROM
 *
 */
void test_write_flash() {
    // Write 0..99 to element and Write to EEProm
    for (int iCnt = 0; iCnt < 100; iCnt++) {
        data[iCnt] = iCnt;
    }
    TEST_ASSERT_EQUAL(Flash::QSPI_OK, memFlash.BSP_QSPI_Write(data, 0, 100));
}

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {
    // STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
    SetupSystemPeripheral();
    init_wire();
    delay(100);

    // Setup EEprom and Flash memory Access
    memEprom = EEprom(&Wire);
    memFlash = Flash(&hqspi);

    UNITY_BEGIN();

    delay(1000);

    RUN_TEST(test_write_eeprom);
    RUN_TEST(test_read_eeprom);
    RUN_TEST(test_data_written_on_eeprom_is_correct);

    // *************** FLASH ********************

    RUN_TEST(test_init_qspi_hw);
    RUN_TEST(test_erase_flash);
    RUN_TEST(test_write_flash);
    RUN_TEST(test_read_flash);
    RUN_TEST(test_data_written_on_flash_is_correct);

    UNITY_END();
}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************
void loop(void) {
}