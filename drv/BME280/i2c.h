/*
 * i2c.h
 *
 *  Created on: Feb 13, 2018
 *      Author: chris
 */

#ifndef DRV_BME280_I2C_H_
#define DRV_BME280_I2C_H_

void I2C_Init(void);

int I2C_WRITE_STRING(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t byteCount);

int I2C_WRITE_READ_STRING(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t byteCount);

#endif /* DRV_BME280_I2C_H_ */
