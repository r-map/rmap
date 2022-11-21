/**@file uart_driver.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

// Dependencies
#include "drivers/uart/uart_driver.h"

// Variable declaration
static UART_HandleTypeDef uart2;

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
   uartInitConfig(UART_DRIVER_BAUD_RATE_DEFAULT);
}

error_t uartInitConfig(uint32_t baud)
{
   // GPIO_InitTypeDef GPIO_InitStruct = {0};
   RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
   PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler();
   }

   /* Peripheral clock enable */
   __HAL_RCC_USART2_FORCE_RESET();
   __HAL_RCC_USART2_RELEASE_RESET();
   __HAL_RCC_USART2_CLK_ENABLE();

   // /** USART2 GPIO Configuration
   // PD5     ------> USART2_TX
   // PD6     ------> USART2_RX
   // */
   // __HAL_RCC_GPIOD_FORCE_RESET();
   // __HAL_RCC_GPIOD_RELEASE_RESET();
   // __HAL_RCC_GPIOD_CLK_ENABLE();

   // GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
   // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   // GPIO_InitStruct.Pull = GPIO_NOPULL;
   // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   // GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
   // HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

   // Configure USART2
   uart2.Instance = USART2;
   uart2.Init.BaudRate = baud;
   uart2.Init.WordLength = UART_WORDLENGTH_8B;
   uart2.Init.StopBits = UART_STOPBITS_1;
   uart2.Init.Parity = UART_PARITY_NONE;
   uart2.Init.Mode = UART_MODE_TX_RX;
   uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   // uart2.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
   uart2.Init.OverSampling = UART_OVERSAMPLING_16;
   uart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
   uart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
   if (HAL_UART_Init(&uart2) != HAL_OK)
   {
      Error_Handler();
   }

   // Enable USART interrupts
   __HAL_UART_ENABLE_IT(&uart2, UART_IT_TXE);
   __HAL_UART_ENABLE_IT(&uart2, UART_IT_RXNE);

   // Set priority grouping (4 bits for pre-emption priority, no bits
   // for subpriority)
   NVIC_SetPriorityGrouping(3);

   // Configure Usart interrupt priority
   NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(3, 12, 0));

   // Enable USART2
   __HAL_UART_ENABLE(&uart2);

   // Successful processing
   return NO_ERROR;
}

error_t uartDeInit(void)
{
   // Disable USART2
   __HAL_UART_DISABLE(&uart2);

   // Disable USART interrupts
   __HAL_UART_DISABLE_IT(&uart2, UART_IT_TXE);
   __HAL_UART_DISABLE_IT(&uart2, UART_IT_RXNE);

   if (HAL_UART_DeInit(&uart2) != HAL_OK)
   {
      Error_Handler();
   }

   // HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);

   // /* Peripheral clock disable */
   // __HAL_RCC_USART2_FORCE_RESET();
   // __HAL_RCC_USART2_RELEASE_RESET();
   // __HAL_RCC_USART2_CLK_DISABLE();

   // __HAL_RCC_GPIOD_FORCE_RESET();
   // __HAL_RCC_GPIOD_RELEASE_RESET();
   // __HAL_RCC_GPIOD_CLK_DISABLE();

   // Successful processing
   return NO_ERROR;
}

/**
 * @brief Enable UART interrupts
 **/

void uartEnableIrq(void)
{
   // Enable USART2 interrupts
   NVIC_EnableIRQ(USART2_IRQn);
}

/**
 * @brief Disable UART interrupts
 **/

void uartDisableIrq(void)
{
   // Disable USART2 interrupt
   NVIC_DisableIRQ(USART2_IRQn);
}

/**
 * @brief Start transmission
 **/

void uartStartTx(void)
{
   // Enable TXE interrupt
   __HAL_UART_ENABLE_IT(&uart2, UART_IT_TXE);
}

/**
 * @brief UART interrupt handler
 **/

void USART2_IRQHandler(void)
{
   int_t c;
   bool_t flag;
   NetInterface *interface;

   // Enter interrupt service routine
   osEnterIsr();

   // This flag will be set if a higher priority task must be woken
   flag = FALSE;

   // Point to the PPP network interface
   interface = &netInterface[INTERFACE_0_INDEX];

   // TXE interrupt?
   if (__HAL_UART_GET_FLAG(&uart2, UART_FLAG_TXE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&uart2, UART_IT_TXE) != RESET)
   {
      // Get next character
      flag |= pppHdlcDriverReadTxQueue(interface, &c);

      // Valid character read?
      if (c != EOF)
      {
         // Send data byte
         uart2.Instance->TDR = c;
      }
      else
      {
         // Disable TXE interrupt
         __HAL_UART_DISABLE_IT(&uart2, UART_IT_TXE);
      }
   }

   // RXNE interrupt?
   if (__HAL_UART_GET_FLAG(&uart2, UART_FLAG_RXNE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&uart2, UART_IT_RXNE) != RESET)
   {
      // Read data byte
      c = uart2.Instance->RDR;
      // Process incoming character
      flag |= pppHdlcDriverWriteRxQueue(interface, c);
   }

   // ORE interrupt?
   if (__HAL_UART_GET_FLAG(&uart2, UART_FLAG_ORE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&uart2, UART_IT_RXNE) != RESET)
   {
      // Clear ORE interrupt flag
      __HAL_UART_CLEAR_OREFLAG(&uart2);
   }

   // Leave interrupt service routine
   osExitIsr(flag);
}
