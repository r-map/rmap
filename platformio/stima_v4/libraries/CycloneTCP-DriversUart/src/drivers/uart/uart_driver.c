/**@file uart_driver.cpp */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>
Moreno Gasperini <m.gasperini@digiteco.it>

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
UART_HandleTypeDef huart2;

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

   // Configure USART2
   huart2.Instance = USART2;
   huart2.Init.BaudRate = baud;
   huart2.Init.WordLength = UART_WORDLENGTH_8B;
   huart2.Init.StopBits = UART_STOPBITS_1;
   huart2.Init.Parity = UART_PARITY_NONE;
   huart2.Init.Mode = UART_MODE_TX_RX;
   huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   // uart2.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
   huart2.Init.OverSampling = UART_OVERSAMPLING_16;
   huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
   huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
   if (HAL_UART_Init(&huart2) != HAL_OK)
   {
      Error_Handler();
   }

   // Enable USART2
   __HAL_UART_ENABLE(&huart2);

   // Enable USART interrupts
   __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
   __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    // Setup Priority e CB CAN_IRQ_RX Enable
    HAL_NVIC_SetPriority(USART2_IRQn, UART2_NVIC_INT_PREMPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

   // Successful processing
   return NO_ERROR;
}

error_t uartDeInit(void)
{

    // Setup Priority e CB CAN_IRQ_RX Enable
    HAL_NVIC_DisableIRQ(USART2_IRQn);

   // Disable USART interrupts
   __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
   __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);

   // Disable USART2
   __HAL_UART_DISABLE(&huart2);

   if (HAL_UART_DeInit(&huart2) != HAL_OK)
   {
      Error_Handler();
   }

   // /* Peripheral clock disable */
   __HAL_RCC_USART2_FORCE_RESET();
   __HAL_RCC_USART2_RELEASE_RESET();
   __HAL_RCC_USART2_CLK_DISABLE();

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
   __HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
}

/**
 * @brief UART interrupt handler
 **/
void USART2_IRQHandler(void)
{
   // Point to the PPP netInterface[INTERFACE_0_INDEX];
   int_t c;

   // TXE interrupt?
   if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TXE) != RESET)
   {
      // Get next character
      pppHdlcDriverReadTxQueue(&netInterface[INTERFACE_0_INDEX], &c);

      // Valid character read?
      if (c != EOF)
      {
         // Send data byte
         huart2.Instance->TDR = c;
      }
      else
      {
         // Disable TXE interrupt
         __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);
      }
   }

   // RXNE interrupt?
   if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE) != RESET)
   {
      // Read data byte
      c = huart2.Instance->RDR;
      // Process incoming character
      pppHdlcDriverWriteRxQueue(&netInterface[INTERFACE_0_INDEX], c);
   }

   // ORE interrupt?
   if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE) != RESET &&
       __HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE) != RESET)
   {
      // Clear ORE interrupt flag
      __HAL_UART_CLEAR_OREFLAG(&huart2);
   }
}
