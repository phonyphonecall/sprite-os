#include "general.h"

#define PIC_BASE_ADDR (0x81800000)
#define PIC_MASTER_ENABLE_ADDR (PIC_BASE_ADDR + 0x1C)
#define PIC_INTERRUPT_ENABLE_ADDR (PIC_BASE_ADDR + 0x08)
#define PIC_MASTER_ENABLE_MASK (0x03)
#define PIC_VSYNC_ENABLE_MASK (0x04)

extern void microblaze_enable_interrupts(void);

inline void sos_enable_mb_interrupts(void) {
    microblaze_enable_interrupts();
}

void sos_enable_vsync_interrupt(char* str) {
    sos_enable_mb_interrupts();

    SET_ADDR(PIC_MASTER_ENABLE_ADDR, PIC_MASTER_ENABLE_MASK);
    SET_ADDR(PIC_INTERRUPT_ENABLE_ADDR, PIC_VSYNC_ENABLE_MASK);
}



