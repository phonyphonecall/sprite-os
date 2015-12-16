#ifndef SOS_H
#define SOS_H

#include "stdbool.h"
#include "stdint.h"
#include "string.h"

#define SOS_ERR (-1)

#define GET_ADDR(addr) (*((uint32_t*) addr))
#define SET_ADDR(addr, val) (*((volatile uint32_t*) addr) = ((volatile uint32_t) val))

#endif // SOS_H
