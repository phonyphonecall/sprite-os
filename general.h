#ifndef GENERAL_H
#define GENERAL_H

#include "stdbool.h"
#include "stdint.h"

#include "interrupt.h"
#include "input.h"

#define SOS_ERR (-1)

#define GET_ADDR(addr) (*((uint32_t*) addr))
#define SET_ADDR(addr, val) (*((uint32_t*) addr) = ((uint32_t) val))

void sos_uart_printf(char* str);

#endif // GENERAL_H
