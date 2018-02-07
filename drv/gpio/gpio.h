/*
 * gpio.h
 *
 *  Created on: Feb 6, 2018
 *      Author: chris
 */

#ifndef DRV_GPIO_GPIO_H_
#define DRV_GPIO_GPIO_H_

#include <stdbool.h>

struct gpio_pin
{
    int port;
    int pin;
};

bool reserve_pin(struct gpio_pin pin);

#endif /* DRV_GPIO_GPIO_H_ */
