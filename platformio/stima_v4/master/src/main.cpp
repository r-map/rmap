/**
* @file main.cpp
* @brief Main
*
* @section License
*
* SPDX-License-Identifier: GPL-2.0-or-later
*
* Copyright (C) 2022 Marco Baldinetti. All rights reserved.
*
* This file is part of CycloneTCP Open.
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
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* @author Marco Baldinetti <marco.baldinetti@alling.it>
* @version 0.1
**/

#define TRACE_LEVEL STIMA_TRACE_LEVEL

#include "main.h"

//Client's certificate
const char_t clientCert[] =
"-----BEGIN CERTIFICATE-----"
"MIICmzCCAYOgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix"
"FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE"
"CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y"
"ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMjAxMTQxNDMy"
"MTRaFw0yMjA0MTQxNDMyMTRaMEAxCzAJBgNVBAYTAkZSMRYwFAYDVQQKDA1Pcnl4"
"IEVtYmVkZGVkMRkwFwYDVQQDDBBtcXR0LWNsaWVudC1kZW1vMFkwEwYHKoZIzj0C"
"AQYIKoZIzj0DAQcDQgAEWT/enOkLuY+9NzUQPOuNVFARl5Y3bc4lLt3TyVwWG0Ez"
"IIk8Wll5Ljjrv+buPSKBVQtOwF9VgyW4QuQ1uYSAIaMaMBgwCQYDVR0TBAIwADAL"
"BgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADggEBAExFyxS1GTiVGmSPUCkVkvzn"
"WN4WdUjeWm8tN2PmFXMyV1wIlmBPeOlYk5Analbmp5mxoJhVP1cOtJMnqjfl6nSF"
"iX8ZSQTCW6bEyxOTPcJkjYIDXcmM0bB5fSNn2WwuOSkCgkncL//qA9q6suKZ3xen"
"Jl39vRyDxIzVnhtuZhg6Q7iSDNsDOv+j4sWh0Z6Zb45so/uWn6mwEaiWVQU38lPp"
"MeNFjkfaVAwmnaO/1Qfc4yroWcoI5NHCYbTkbTf6cVbQ67GdCZtCzFEJdmuBNS8H"
"NoJ20mL2D28AFnVsYu2w2bJAjYJ1uDk94Qrgi5p36OPMwp+AKXpDRGsH3H7hmjs="
"-----END CERTIFICATE-----";

//Client's private key
const char_t clientKey[] =
"-----BEGIN EC PRIVATE KEY-----"
"MHcCAQEEICYULY0KQ6nDAXFl5tgK9ljqAZyb14JQmI3iT7tdScDloAoGCCqGSM49"
"AwEHoUQDQgAEWT/enOkLuY+9NzUQPOuNVFARl5Y3bc4lLt3TyVwWG0EzIIk8Wll5"
"Ljjrv+buPSKBVQtOwF9VgyW4QuQ1uYSAIQ=="
"-----END EC PRIVATE KEY-----";

//List of trusted CA certificates
const char_t trustedCaList[] =
"-----BEGIN CERTIFICATE-----"
"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL"
"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG"
"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU"
"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv"
"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE"
"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES"
"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp"
"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg"
"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW"
"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA"
"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH"
"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo"
"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT"
"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV"
"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL"
"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC"
"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf"
"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK"
"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839"
"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE"
"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU="
"-----END CERTIFICATE-----"
"-----BEGIN CERTIFICATE-----"
"MIIC8DCCAlmgAwIBAgIJAOD63PlXjJi8MA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD"
"VQQGEwJHQjEXMBUGA1UECAwOVW5pdGVkIEtpbmdkb20xDjAMBgNVBAcMBURlcmJ5"
"MRIwEAYDVQQKDAlNb3NxdWl0dG8xCzAJBgNVBAsMAkNBMRYwFAYDVQQDDA1tb3Nx"
"dWl0dG8ub3JnMR8wHQYJKoZIhvcNAQkBFhByb2dlckBhdGNob28ub3JnMB4XDTEy"
"MDYyOTIyMTE1OVoXDTIyMDYyNzIyMTE1OVowgZAxCzAJBgNVBAYTAkdCMRcwFQYD"
"VQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwGA1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1v"
"c3F1aXR0bzELMAkGA1UECwwCQ0ExFjAUBgNVBAMMDW1vc3F1aXR0by5vcmcxHzAd"
"BgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hvby5vcmcwgZ8wDQYJKoZIhvcNAQEBBQAD"
"gY0AMIGJAoGBAMYkLmX7SqOT/jJCZoQ1NWdCrr/pq47m3xxyXcI+FLEmwbE3R9vM"
"rE6sRbP2S89pfrCt7iuITXPKycpUcIU0mtcT1OqxGBV2lb6RaOT2gC5pxyGaFJ+h"
"A+GIbdYKO3JprPxSBoRponZJvDGEZuM3N7p3S/lRoi7G5wG5mvUmaE5RAgMBAAGj"
"UDBOMB0GA1UdDgQWBBTad2QneVztIPQzRRGj6ZHKqJTv5jAfBgNVHSMEGDAWgBTa"
"d2QneVztIPQzRRGj6ZHKqJTv5jAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUA"
"A4GBAAqw1rK4NlRUCUBLhEFUQasjP7xfFqlVbE2cRy0Rs4o3KS0JwzQVBwG85xge"
"REyPOFdGdhBY2P1FNRy0MDr6xr+D2ZOwxs63dG1nnAnWZg7qwoLgpZ4fESPD3PkA"
"1ZgKJc2zbSQ9fCPxt2W3mdVav66c6fsb7els2W2Iz7gERJSX"
"-----END CERTIFICATE-----"
"-----BEGIN CERTIFICATE-----"
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT"
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow"
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT"
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC"
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF"
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8"
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0"
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA"
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj"
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T"
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG"
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv"
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k"
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw"
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC"
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz"
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu"
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF"
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo"
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/"
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu"
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG"
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6"
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg=="
"-----END CERTIFICATE-----";

void setup() {
  osInitKernel();
  SerialDebugInit(115200);

  error_t error = NO_ERROR;

  error = initCPRNG();
  if (error) {
    TRACE_ERROR("Failed to initialize Cryptographic Pseudo Random Number Generator!\r\n");
  }
  // TCP/IP stack initialization
  error = netInit();
  if (error) {
    TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
  }

  LedParam_t ledParam1 = {LED1_PIN, 100, 900};
  LedParam_t ledParam2 = {LED2_PIN, 200, 800};
  LedParam_t ledParam3 = {LED3_PIN, 300, 700};
  // HardwareParam_t hardwareParam;
  EthernetParam_t ethernetParam;
  MqttParam_t mqttParam;

  // hardwareParam.interface = &netInterface[0];
  // hardwareParam.tickHandlerMs = APP_ETHERNET_TICK_EVENT_HANDLER_MS;

  ethernetParam.interface = &netInterface[0];
  ethernetParam.tickHandlerMs = APP_ETHERNET_TICK_EVENT_HANDLER_MS;

  mqttParam.interface = &netInterface[0];
  mqttParam.timeoutMs = APP_MQTT_TIMEOUT_MS;
  mqttParam.attemptDelayMs = APP_MQTT_ATTEMPT_DELAY_MS;
  strncpy(mqttParam.server, APP_SERVER_NAME, APP_MQTT_SERVER_NAME_LENGTH);
  mqttParam.port = APP_SERVER_PORT;
  mqttParam.version = MQTT_VERSION_3_1_1;
  mqttParam.transportProtocol = MQTT_TRANSPORT_PROTOCOL_TLS;
  mqttParam.qos = MQTT_QOS_LEVEL_1;
  mqttParam.keepAliveS = APP_MQTT_KEEP_ALIVE_S;
  memset(mqttParam.username, 0, APP_MQTT_USERNAME_LENGTH);
  memset(mqttParam.password, 0, APP_MQTT_USERNAME_LENGTH);
  // strncpy(mqttParam.username, "username", APP_MQTT_USERNAME_LENGTH);
  // strncpy(mqttParam.password, "password", APP_MQTT_PASSWORD_LENGTH);
  // memset(mqttParam.willTopic, 0, APP_MQTT_WILL_TOPIC_LENGTH);
  strncpy(mqttParam.willTopic, "board/status", APP_MQTT_WILL_TOPIC_LENGTH);
  // memset(mqttParam.willMsg, 0, APP_MQTT_WILL_MSG_LENGTH);
  strncpy(mqttParam.willMsg, "offline", APP_MQTT_WILL_MSG_LENGTH);
  mqttParam.isWillMsgRetain = false;
  mqttParam.isCleanSession = true;
  mqttParam.isPublishRetain = false;
  mqttParam.yarrowContext = &yarrowContext;
  mqttParam.trustedCaList = (char *)trustedCaList;

  static LedTask led_1_task("LED 1 TASK", 100, OS_TASK_PRIORITY_01, ledParam1);
  static LedTask led_2_task("LED 2 TASK", 100, OS_TASK_PRIORITY_01, ledParam2);
  static LedTask led_3_task("LED 3 TASK", 100, OS_TASK_PRIORITY_01, ledParam3);
  // static HardwareTask hw_task("HW TASK", 100, OS_TASK_PRIORITY_11, hardwareParam);
  static EthernetTask eth_task("ETH TASK", 100, OS_TASK_PRIORITY_03, ethernetParam);
  static MqttTask mqtt_task("MQTT TASK", 1024, OS_TASK_PRIORITY_02, mqttParam);

  cpp_freertos::Thread::StartScheduler();
}

void loop() {}

error_t initCPRNG () {
  // Global variables
  error_t error;

  // Initialize hardware cryptographic accelerator
  error = stm32l4xxCryptoInit();
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize hardware crypto accelerator!\r\n");
  }

  // Generate a random seed
  error = trngGetRandomData(seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to generate random data!\r\n");
  }

  // PRNG initialization
  error = yarrowInit(&yarrowContext);
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize PRNG!\r\n");
  }

  // Properly seed the PRNG
  error = yarrowSeed(&yarrowContext, seed, sizeof(seed));
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to seed PRNG!\r\n");
  }

  return error;
}
