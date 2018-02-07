/*
 * adc.h
 *
 *  Created on: Feb 6, 2018
 *      Author: chris
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdbool.h>
#include <stdint.h>

#define ADC_NUM_CHANNELS 32

struct ADC
{
    struct
    {
        int input_id
        bool is_high_range;
    } config[ADC_NUM_CHANNELS];
    int16_t result[ADC_NUM_CHANNELS];
};

extern volatile struct ADC ADC;

void init_ADC();

#endif /* ADC_H_ */
