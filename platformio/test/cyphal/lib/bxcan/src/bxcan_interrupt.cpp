#include <bxcan_interrupt.h>
//#include <bxcan_registers.h>
//#include <assert.h>


static inline volatile uint32_t &periphBit(uint32_t addr, int bitNum) // peripheral bit tool
{
  return MMIO32(0x42000000 + ((addr & 0xFFFFF) << 5) + (bitNum << 2)); // uses bit band memory
}

void bxCANenableInterrupt()
{

  //assert(BXCAN1->IER == 1U);
  periphBit(ier, fmpie0) = 1U; // set fifo RX int enable request
  MMIO32(iser) = 1UL << 20;
}

void bxCANdisableInterrupt()
{
  //assert(BXCAN1->IER == 0U);
  periphBit(ier, fmpie0) = 0U;
  MMIO32(iser) = 1UL << 20;
}

void bxCANattachInterrupt(void func()) // copy IRQ table to SRAM, point VTOR reg to it, set IRQ addr to user ISR
{
    static uint8_t newTbl[0xF0] __attribute__((aligned(0x100)));
    uint8_t *pNewTbl = newTbl;
    int origTbl = MMIO32(vtor);
    for (int j = 0; j < 0x3c; j++) // table length = 60 integers
        MMIO32((pNewTbl + (j << 2))) = MMIO32((origTbl + (j << 2)));

    uint32_t canVectTblAdr = reinterpret_cast<uint32_t>(pNewTbl) + (36 << 2); // calc new ISR addr in new vector tbl
    MMIO32(canVectTblAdr) = reinterpret_cast<uint32_t>(func);                 // set new CAN/USB ISR jump addr into new table
    MMIO32(vtor) = reinterpret_cast<uint32_t>(pNewTbl);                       // load vtor register with new tbl location
    bxCANenableInterrupt();
}

