#ifndef _I2C_COMMON_H_
#define _I2C_COMMON_H_
//#include "main.h"
#include "stm32f0xx_ll_i2c.h"
#include "stm32f0xx_ll_cortex.h"

#define I2C_ERR 0

#define I2C_REQUEST_WRITE                       0x00
#define I2C_REQUEST_READ                        0x01

void I2C1_WriteBytesToAddr (uint8_t iic_addr, uint8_t addr,uint8_t *buf, uint16_t len);
void I2C1_ReadBytesFromAddr(uint8_t iic_addr, uint16_t addr, uint8_t *buf, uint8_t len);
void TimeOutChecker(uint8_t*, uint8_t);
void I2C_Error_Handler(uint8_t);
#endif
