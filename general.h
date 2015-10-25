#ifndef GENERAL_H
#define GENERAL_H

#include "stdint.h"

#define SET_ADDR(reg, val) (*((uint32_t*) reg) = ((uint32_t) val))

void sos_uart_printf(char* str);
void sos_enable_vsync_interrupt();

#endif // GENERAL_H
