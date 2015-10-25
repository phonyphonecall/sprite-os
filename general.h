#ifndef GENERAL_H
#define GENERAL_H

#include "stdbool.h"
#include "stdint.h"

#include "interrupt.h"

#define SOS_ERR (-1)

#define SET_ADDR(reg, val) (*((uint32_t*) reg) = ((uint32_t) val))

void sos_uart_printf(char* str);

#endif // GENERAL_H
