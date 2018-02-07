/*
 * adc.c
 *
 *  Created on: Feb 6, 2018
 *      Author: chris
 */

#include <stdint.h>

#include "driverlib.h"

#include "../gpio/gpio.h"
#include "adc.h"


#define BIT32(N) ((uint32_t)1 << (N))

#define ADC_MEM(N) BIT32(N)
#define ADC_INPUT_A(N) (N)
#define ADC_INT(N) BIT32(N)

volatile struct ADC ADC;


static struct gpio_pin pins[24] =
{
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN5}, //  0
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN4}, //  1
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN3}, //  2
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN2}, //  3
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN1}, //  4
 {.port = GPIO_PORT_P5, .pin = GPIO_PIN0}, //  5
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN7}, //  6
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN6}, //  7
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN5}, //  8
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN4}, //  9
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN3}, // 10
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN2}, // 11
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN1}, // 12
 {.port = GPIO_PORT_P4, .pin = GPIO_PIN0}, // 13
 {.port = GPIO_PORT_P6, .pin = GPIO_PIN1}, // 14
 {.port = GPIO_PORT_P6, .pin = GPIO_PIN0}, // 15
 {.port = GPIO_PORT_P9, .pin = GPIO_PIN1}, // 16
 {.port = GPIO_PORT_P9, .pin = GPIO_PIN0}, // 17
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN7}, // 18
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN6}, // 19
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN5}, // 20
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN4}, // 21
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN3}, // 22
 {.port = GPIO_PORT_P8, .pin = GPIO_PIN2}, // 23
 /* ... */
};



void init_ADC()
{
    /* Initializing reference voltage */
    MAP_REF_A_setReferenceVoltage(REF_A_VREF1_2V);
    MAP_REF_A_enableReferenceVoltage();

    /* Initializing ADC (MCLK/1/4) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4, 0);

    /* GPIO pins */
    for(int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
        int input_id = ADC.config[i].input_id;

        reserve_pin(pins[input_id]);
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
                pins[input_id].port,
                pins[input_id].pin,
                GPIO_TERTIARY_MODULE_FUNCTION);
    }

    /* Configuring sample mode */
    MAP_ADC14_configureMultiSequenceMode(ADC_MEM(0), ADC_MEM(ADC_NUM_CHANNELS - 1), true);

    /* Configuring conversion modes */
    for(int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
        MAP_ADC14_configureConversionMemory(
                ADC_MEM(i),
                ADC.config[i].is_high_range?
                        ADC_VERFPOS_AVCC_VREFNEG_VSS
                        : ADC_VREFPOS_INTBUF_VREFNEG_VSS,
                ADC_INPUT_A(i), false);
    }

    /* Configuring Sample Timer */
    MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    /* Enabling/Toggling Conversion */
    MAP_ADC14_enableConversion();

    /* Enabling interrupts */
    for(int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
        MAP_ADC14_enableInterrupt(ADC_INT(i));
    }
    MAP_Interrupt_enableInterrupt(INT_ADC14);
}

void handle_ADC_interrupt()
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    for(int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
        uint32_t flag = BIT32(i);
        if(flag & status)
        {
            ADC.result[i] = MAP_ADC14_getResult(flag);
        }
    }
}
