
/*
 * This file is derived from the libopencm3 project examples 
 */

/*
 B12        MICRO USB         GND
 B13                          GND
 B14                          3V3
 B15   RESET    GND    GND  RESET
 A8    BUTTON  *BOOT1 *BOOT0  B11
 (TX0) A9             3V3    3V3    B10
 (RX0) A10                           B1
 A11                           B0
 A12                           A7
 A15                           A6
 B3       STM32F103C8T6        A5
 B4                            A4
 B5                            A3 RX2
 B6                            A2 TX2
 B7              8M            A1 RTS2
 CANRX B8           32768            A0 CTS2
 CANTX B9                           C15
 5V       PC13     POWER      C14
 GND      LED      LED        C13
 3V3           SWD           VBAT
 3V3 DIO  DCLK GND
 PA13 PA14

 *100K IN SERIES
 */

#include "main.h"

extern struct ring output_ring;
extern struct ring input_ring;
volatile unsigned int counter;
volatile uint8_t status;
volatile uint8_t commands_pending;
uint8_t d_data[8];

static void gpio_setup(void) {
	/* Enable GPIOA & GPIOB & GPIOC clock */
	/* A2 & A3 USART */
	rcc_periph_clock_enable(RCC_GPIOA);
	/* B8 & B9 CAN */
	rcc_periph_clock_enable(RCC_GPIOB);
	/* C12 LED */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Preconfigure LED */
	gpio_set(GPIOC, GPIO13); /* LED green off */

	/* Preconfigure Osci pin CAN -> ASCII*/
	gpio_clear(GPIOC, GPIO14);

	/* Preconfigure Osci pin ASCII Buffer Send */
	gpio_clear(GPIOC, GPIO15);

	/* Configure LED&Osci GPIO */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO13);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO14);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
			GPIO15);

	/* Enable clocks for GPIO port A (for GPIO_USART2_TX) and USART2. */
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_DMA1);
}


static void systick_setup(void) {
	/* 72MHz / 8 => 9000000 counts per second */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

	/* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(8999);

	systick_interrupt_enable();

	/* Start counting */
	systick_counter_enable();
}

void sys_tick_handler(void) {

	/* We call this handler every 1ms so every 1ms = 0.001s
	 * resulting in 1Hz message rate.
	 */

	/* Transmit CAN frame. */
	counter++;
	if (counter == 500) {
		counter = 0;
		gpio_toggle(GPIOC, GPIO13); /* toggle green LED */
	}
}


int main(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	gpio_setup();
	usart_setup();
	systick_setup();

	/* endless loop */
	while (1) {
	  ring_write_ch(&output_ring, 'c');	  
	  ring_write_ch(&output_ring, 'i');	  
	  ring_write_ch(&output_ring, 'a');	  
	  ring_write_ch(&output_ring, 'o');	  
	  ring_write_ch(&output_ring, '\n');
	  /* enable the transmitter now */
	  USART_CR1(USART2) |= USART_CR1_TXEIE;
	}

}
