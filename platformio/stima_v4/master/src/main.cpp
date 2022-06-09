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

#define TRACE_LEVEL TRACE_LEVEL_VERBOSE

#include <stdlib.h>
#include "STM32FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
// #include "SdFat.h"

#include "tasks/led_task.h"
#include "tasks/ethernet_task.h"

using namespace cpp_freertos;

// Global variables
HttpClientContext httpClientContext;
YarrowContext yarrowContext;
uint8_t seed[32];
error_t error;

void setup() {
  SerialDebugInit(115200);

  // Start-up message
  TRACE_INFO("\r\n");
  TRACE_INFO("************************************\r\n");
  TRACE_INFO("*** CycloneTCP HTTPS Client Demo ***\r\n");
  TRACE_INFO("************************************\r\n");
  TRACE_INFO("Copyright: 2010-2022 Oryx Embedded SARL\r\n");
  TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
  TRACE_INFO("Target: STML496ZG\r\n");
  TRACE_INFO("\r\n");

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

  // TCP/IP stack initialization
  error = netInit();
  // Any error to report?
  if (error)
  {
    // Debug message
    TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
  }

  LedParam_t ledParam1 = {LED1_PIN, 100, 900};
  LedParam_t ledParam2 = {LED2_PIN, 200, 800};
  LedParam_t ledParam3 = {LED3_PIN, 300, 700};
  EthernetParam_t ethernetParam;
  ethernetParam.interface = &netInterface[0];

  static LedTask led_1_task(100, OS_TASK_PRIORITY_NORMAL, ledParam1);
  static LedTask led_2_task(100, OS_TASK_PRIORITY_NORMAL, ledParam2);
  static LedTask led_3_task(100, OS_TASK_PRIORITY_NORMAL, ledParam3);
  static EthernetTask eth_task(8192, OS_TASK_PRIORITY_NORMAL, ethernetParam);

  // httpClientTest();

  Thread::StartScheduler();
}

void loop() {}

// // List of preferred ciphersuites
// const uint16_t cipherSuites[] =
//     {
//         TLS_CHACHA20_POLY1305_SHA256,
//         TLS_AES_128_GCM_SHA256,
//         TLS_AES_256_GCM_SHA384,
//         TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
//         TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
//         TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
//         TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
//         TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
//         TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
//         TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
//         TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
//         TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
//         TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
//         TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
//         TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
//         TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
//         TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
//         TLS_RSA_WITH_AES_128_GCM_SHA256,
//         TLS_RSA_WITH_AES_256_GCM_SHA384,
//         TLS_RSA_WITH_AES_128_CBC_SHA,
//         TLS_RSA_WITH_AES_256_CBC_SHA,
//         TLS_RSA_WITH_3DES_EDE_CBC_SHA};
//
// // List of trusted CA certificates
// const char_t trustedCaList[] =
//     "-----BEGIN CERTIFICATE-----"
//     "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF"
//     "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6"
//     "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL"
//     "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv"
//     "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj"
//     "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM"
//     "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw"
//     "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6"
//     "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L"
//     "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm"
//     "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC"
//     "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA"
//     "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI"
//     "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs"
//     "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv"
//     "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU"
//     "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy"
//     "rqXRfboQnoZsG4q5WTP468SQvvG5"
//     "-----END CERTIFICATE-----";

//
// /**
//  * @brief TLS initialization callback
//  * @param[in] context Pointer to the HTTP client context
//  * @param[in] tlsContext Pointer to the TLS context
//  * @return Error code
//  **/
//
// error_t httpClientTlsInitCallback(HttpClientContext *context,
//                                   TlsContext *tlsContext)
// {
//   error_t error;
//
//   // Debug message
//   TRACE_INFO("HTTP Client: TLS initialization callback\r\n");
//
//   // Set the PRNG algorithm to be used
//   error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, &yarrowContext);
//   // Any error to report?
//   if (error)
//     return error;
//
// #if (APP_SET_CIPHER_SUITES == ENABLED)
//   // Preferred cipher suite list
//   error = tlsSetCipherSuites(tlsContext, cipherSuites, arraysize(cipherSuites));
//   // Any error to report?
//   if (error)
//     return error;
// #endif
//
// #if (APP_SET_SERVER_NAME == ENABLED)
//   // Set the fully qualified domain name of the server
//   error = tlsSetServerName(tlsContext, APP_HTTP_SERVER_NAME);
//   // Any error to report?
//   if (error)
//     return error;
// #endif
//
// #if (APP_SET_TRUSTED_CA_LIST == ENABLED)
//   // Import the list of trusted CA certificates
//   error = tlsSetTrustedCaList(tlsContext, trustedCaList, strlen(trustedCaList));
//   // Any error to report?
//   if (error)
//     return error;
// #endif
//
//   // Successful processing
//   return NO_ERROR;
// }
//
// /**
//  * @brief HTTP client test routine
//  * @return Error code
//  **/
//
// error_t httpClientTest(void)
// {
//   error_t error;
//   size_t length;
//   uint_t status;
//   const char_t *value;
//   IpAddr ipAddr;
//   char_t buffer[128];
//
//   // Initialize HTTP client context
//   httpClientInit(&httpClientContext);
//
//   // Start of exception handling block
//   do
//   {
//     // Debug message
//     TRACE_INFO("\r\n\r\nResolving server name...\r\n");
//
//     // Resolve HTTP server name
//     error = getHostByName(NULL, APP_HTTP_SERVER_NAME, &ipAddr, 0);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to resolve server name!\r\n");
//       break;
//     }
//
// #if (APP_HTTP_SERVER_PORT == 443)
//     // Register TLS initialization callback
//     error = httpClientRegisterTlsInitCallback(&httpClientContext,
//                                               httpClientTlsInitCallback);
//     // Any error to report?
//     if (error)
//       break;
// #endif
//
//     // Select HTTP protocol version
//     error = httpClientSetVersion(&httpClientContext, HTTP_VERSION_1_1);
//     // Any error to report?
//     if (error)
//       break;
//
//     // Set timeout value for blocking operations
//     error = httpClientSetTimeout(&httpClientContext, 20000);
//     // Any error to report?
//     if (error)
//       break;
//
//     // Debug message
//     TRACE_INFO("Connecting to HTTP server %s...\r\n",
//                ipAddrToString(&ipAddr, NULL));
//
//     // Connect to the HTTP server
//     error = httpClientConnect(&httpClientContext, &ipAddr,
//                               APP_HTTP_SERVER_PORT);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to connect to HTTP server!\r\n");
//       break;
//     }
//
//     // Create an HTTP request
//     httpClientCreateRequest(&httpClientContext);
//     httpClientSetMethod(&httpClientContext, "POST");
//     httpClientSetUri(&httpClientContext, APP_HTTP_URI);
//
//     // Set query string
//     httpClientAddQueryParam(&httpClientContext, "param1", "value1");
//     httpClientAddQueryParam(&httpClientContext, "param2", "value2");
//
//     // Add HTTP header fields
//     httpClientAddHeaderField(&httpClientContext, "Host", APP_HTTP_SERVER_NAME);
//     httpClientAddHeaderField(&httpClientContext, "User-Agent", "Mozilla/5.0");
//     httpClientAddHeaderField(&httpClientContext, "Content-Type", "text/plain");
//     httpClientAddHeaderField(&httpClientContext, "Transfer-Encoding", "chunked");
//
//     // Send HTTP request header
//     error = httpClientWriteHeader(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to write HTTP request header!\r\n");
//       break;
//     }
//
//     // Send HTTP request body
//     error = httpClientWriteBody(&httpClientContext, "Hello World!", 12,
//                                 NULL, 0);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to write HTTP request body!\r\n");
//       break;
//     }
//
//     // Receive HTTP response header
//     error = httpClientReadHeader(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to read HTTP response header!\r\n");
//       break;
//     }
//
//     // Retrieve HTTP status code
//     status = httpClientGetStatus(&httpClientContext);
//     // Debug message
//     TRACE_INFO("HTTP status code: %u\r\n", status);
//
//     // Retrieve the value of the Content-Type header field
//     value = httpClientGetHeaderField(&httpClientContext, "Content-Type");
//
//     // Header field found?
//     if (value != NULL)
//     {
//       // Debug message
//       TRACE_INFO("Content-Type header field value: %s\r\n", value);
//     }
//     else
//     {
//       // Debug message
//       TRACE_INFO("Content-Type header field not found!\r\n");
//     }
//
//     // Receive HTTP response body
//     while (!error)
//     {
//       // Read data
//       error = httpClientReadBody(&httpClientContext, buffer,
//                                  sizeof(buffer) - 1, &length, 0);
//
//       // Check status code
//       if (!error)
//       {
//         // Properly terminate the string with a NULL character
//         buffer[length] = '\0';
//         // Dump HTTP response body
//         TRACE_INFO("%s", buffer);
//       }
//     }
//
//     // Terminate the HTTP response body with a CRLF
//     TRACE_INFO("\r\n");
//
//     // Any error to report?
//     if (error != ERROR_END_OF_STREAM)
//       break;
//
//     // Close HTTP response body
//     error = httpClientCloseBody(&httpClientContext);
//     // Any error to report?
//     if (error)
//     {
//       // Debug message
//       TRACE_INFO("Failed to read HTTP response trailer!\r\n");
//       break;
//     }
//
//     // Gracefully disconnect from the HTTP server
//     httpClientDisconnect(&httpClientContext);
//
//     // Debug message
//     TRACE_INFO("Connection closed\r\n");
//
//     // End of exception handling block
//   } while (0);
//
//   // Release HTTP client context
//   httpClientDeinit(&httpClientContext);
//
//   // Return status code
//   return error;
// }
