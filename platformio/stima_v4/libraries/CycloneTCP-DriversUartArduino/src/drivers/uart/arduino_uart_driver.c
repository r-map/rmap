/**
 * @file uart_driver.c
 * @brief UART driver
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
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
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.1.8
 **/

//Dependencies
#include "arduino_uart_driver.h"

//Enable hardware flow control
// #define APP_UART_HW_FLOW_CTRL ENABLED

//Variable declaration
// static UART_HandleTypeDef UART_Handle;


/**
 * @brief UART driver
 **/

const UartDriver uartDriver =
{
   uartInit,
   uartEnableIrq,
   uartDisableIrq,
   uartStartTx
};


/**
 * @brief UART configuration
 * @return Error code
 **/

error_t uartInit(void)
{
   // Serial2.begin(115200);
   //Successful processing
   return NO_ERROR;
}


/**
 * @brief Enable UART interrupts
 **/

void uartEnableIrq(void)
{
   //Enable USART1 interrupts
   // NVIC_EnableIRQ(USART1_IRQn);
}


/**
 * @brief Disable UART interrupts
 **/

void uartDisableIrq(void)
{
   //Disable USART1 interrupt
   // NVIC_DisableIRQ(USART1_IRQn);
}


/**
 * @brief Start transmission
 **/

void uartStartTx(void)
{
   //Enable TXE interrupt
   // __HAL_UART_ENABLE_IT(&UART_Handle, UART_IT_TXE);
}


/**
 * @brief UART interrupt handler
 **/

// void USART1_IRQHandler(void)
// {
//    int_t c;
//    bool_t flag;
//    NetInterface *interface;

//    //Enter interrupt service routine
//    osEnterIsr();

//    //This flag will be set if a higher priority task must be woken
//    flag = FALSE;

//    //Point to the PPP network interface
//    interface = &netInterface[0];

//    //TXE interrupt?
//    // if(__HAL_UART_GET_FLAG(&UART_Handle, UART_FLAG_TXE) != RESET &&
//    //    __HAL_UART_GET_IT_SOURCE(&UART_Handle, UART_IT_TXE) != RESET)
//    // {
//    //    //Get next character
//    //    flag |= pppHdlcDriverReadTxQueue(interface, &c);

//    //    //Valid character read?
//    //    if(c != EOF)
//    //    {
//    //       //Send data byte
//    //       UART_Handle.Instance->TDR = c;
//    //    }
//    //    else
//    //    {
//    //       //Disable TXE interrupt
//    //       __HAL_UART_DISABLE_IT(&UART_Handle, UART_IT_TXE);
//    //    }
//    // }

//    // //RXNE interrupt?
//    // if(__HAL_UART_GET_FLAG(&UART_Handle, UART_FLAG_RXNE) != RESET &&
//    //    __HAL_UART_GET_IT_SOURCE(&UART_Handle, UART_IT_RXNE) != RESET)
//    // {
//    //    //Read data byte
//    //    c = UART_Handle.Instance->RDR;
//    //    //Process incoming character
//    //    flag |= pppHdlcDriverWriteRxQueue(interface, c);
//    // }

//    // //ORE interrupt?
//    // if(__HAL_UART_GET_FLAG(&UART_Handle, UART_FLAG_ORE) != RESET &&
//    //    __HAL_UART_GET_IT_SOURCE(&UART_Handle, UART_IT_RXNE) != RESET)
//    // {
//    //    //Clear ORE interrupt flag
//    //    __HAL_UART_CLEAR_OREFLAG(&UART_Handle);
//    // }

//    //Leave interrupt service routine
//    osExitIsr(flag);
// }
