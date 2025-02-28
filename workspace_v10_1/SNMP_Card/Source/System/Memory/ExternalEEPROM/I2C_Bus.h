/*
 * I2C_Bus.h
 *
 *  Created on: 23-Aug-2021
 *      Author: rushikesh.sonone
 */

#ifndef SYSTEM_I2C_BUS_H_
#define SYSTEM_I2C_BUS_H_

#include <Common.h>
void Init_External_EEPROM(void);

void I2C_Receive_Burst (uint32_t I2CBase, uint32_t slave_addr, uint8_t reg, uint16_t numElem, uint8_t *buff);
uint16_t I2C_Receive(uint32_t I2CBase, uint32_t slave_addr, uint8_t Register);

uint32_t I2C_Transmit_Burst(uint32_t I2CBase, const uint8_t SlaveAddr, uint8_t *DataPtr, const uint32_t NumBytes);



#endif /* SYSTEM_I2C_BUS_H_ */
