// This software is distributed under the terms of the MIT License.
// Progetto RMAP - STIMA V4
// Hardware Config, STIMAV4 SLAVE Board - Rev.1.00
// Copyright (C) 2022 Digiteco s.r.l.
// Author: Gasperini Moreno <m.gasperini@digiteco.it>

// Arduino
#include <Arduino.h>
// Private HW Configuration
#include "module_slave_hal.hpp"

#define CAN_INIT        0
#define CAN_NORMAL      1
#define CAN_LISTEN_ONLY 2
#define CAN_SLEEP       3

// *********************************************************************************************
//                                       SETUP AMBIENTE
// *********************************************************************************************
void setup(void) {

    // *****************************************************
    //   STARTUP PRIVATE BASIC HARDWARE CONFIG AND ISTANCE
    // *****************************************************

    // MX_GPIO_Init();
    // MX_CAN1_Init();
    // MX_CRC_Init();
    // MX_I2C2_Init();
    // MX_QUADSPI_Init();
    // MX_RTC_Init();
    // MX_UART4_Init();
    // MX_USART1_UART_Init();
    // MX_LPTIM1_Init();
    // MX_RNG_Init();
    // MX_USART2_UART_Init();
    // MX_I2C1_Init();
    // MX_SPI1_Init();

    // Serial2.begin(9600);

    Serial.begin(115200);
    // Wait for serial port to connect
    while (!Serial) {
    }
    Serial.println(F("Start RS232 Monitor"));

    // // *****************************************************
    // //            STARTUP LED E PIN DIAGNOSTICI
    // // *****************************************************
    // // Output mode for LED BLINK SW LOOP (High per Setup)
    // // Input mode for test button
    
    // Serial.println(F("Initialization HW Base done"));

    // // Config PIN ->
    // // CAN:
    // // PE14 -> Abilita
    // // PE15 -> Disabilita
    pinMode(PIN_CAN_EN, OUTPUT);
    pinMode(PIN_CAN_STB, OUTPUT);
    CAN_Power(CAN_INIT);

}

// *************************************************************************************************
//                                          MAIN LOOP
// *************************************************************************************************

/// @brief Enable Power CAN_Circuit TJA1443
/// @param ModeCan (Mode TYPE CAN_BUS)
void CAN_Power(uint8_t ModeCan) {
    // INIT State o setup from WakeUP
    if(ModeCan == CAN_INIT) {
        // A LOW-to-HIGH transition on pin STB_N will clear the UVNOM flag
        digitalWrite(PIN_CAN_EN, LOW);
        digitalWrite(PIN_CAN_STB, LOW);
        // Wakeup security, unknown state Min 5 uS from Sleep to Listen
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_EN, HIGH);
        digitalWrite(PIN_CAN_STB, HIGH);
    }
    // Normal Mode (TX/RX Full functionally)
    if(ModeCan == CAN_NORMAL) {
        digitalWrite(PIN_CAN_EN, HIGH);
        digitalWrite(PIN_CAN_STB, HIGH);
    }
    // Listen Mode (Only RX circuit enabled)
    if(ModeCan == CAN_LISTEN_ONLY) {
        digitalWrite(PIN_CAN_EN, HIGH);
        digitalWrite(PIN_CAN_STB, HIGH);
        // Wakeup minimo 5 uS from Sleep to Listen
        delayMicroseconds(10);
        digitalWrite(PIN_CAN_EN, LOW);
        digitalWrite(PIN_CAN_STB, LOW);
    }
    // Sleep (Turn OFF HW and enter sleep mode TJA1443)
    if(ModeCan == CAN_SLEEP) {
        digitalWrite(PIN_CAN_EN, HIGH);
        digitalWrite(PIN_CAN_STB, LOW);
    }
}

void loop(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    uint8_t last_ss=100;
    uint8_t evo_ss=0;

    // RESET Power
    CAN_Power(CAN_INIT);

    do {
         if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN)!= HAL_OK) {
            delay(300);
             Serial.println(F("Error!!!,"));
         }
         if(sTime.Seconds!=last_ss) {
            evo_ss++;
            if(evo_ss==3) {
                Serial.println(F("NORMAL"));
                CAN_Power(CAN_NORMAL);
            }
            if(evo_ss==6) {
                Serial.println(F("LISTEN"));
                CAN_Power(CAN_LISTEN_ONLY);
            }
            if(evo_ss==9) {
                Serial.println(F("SLEEP"));
                CAN_Power(CAN_SLEEP);
            }
            last_ss=sTime.Seconds;
            Serial.print(F("Event second... BIN -> "));
            Serial.println(sTime.Seconds);
            Serial2.println("Evento");
        } else {
            delay(50);
            // Subsecond
            Serial.println((float)1 -((float)sTime.SubSeconds / (float)sTime.SecondFraction));             
        }

    } while (1);

}