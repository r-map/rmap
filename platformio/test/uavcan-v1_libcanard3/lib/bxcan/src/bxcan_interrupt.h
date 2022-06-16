#pragma once

#include <Arduino.h>

#define MMIO32(x) (*(volatile uint32_t *)(x))
#define fmpie0 1 // rx interrupt enable on rx msg pending bit


constexpr static uint32_t scsBase = 0xE000E000UL;        // System Control Space Base Address
constexpr static uint32_t nvicBase = scsBase + 0x0100UL; // NVIC Base Address
constexpr static uint32_t CANBase = 0x40006400;
constexpr static uint32_t ier = CANBase + 0x014;         // interrupt enable
constexpr static uint32_t iser = nvicBase + 0x000;       //  NVIC interrupt set (enable)
constexpr static uint32_t scbBase = scsBase + 0x0D00UL;
constexpr static uint32_t vtor = scbBase + 0x008;

void bxCANenableInterrupt(void);
void bxCANdisableInterrupt(void);
void bxCANattachInterrupt(void func());

