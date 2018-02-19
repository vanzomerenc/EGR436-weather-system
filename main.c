/* DriverLib Includes */
#include "driverlib.h"

#include "drv/ST7735/ST7735.h"
#include "drv/ST7735/ST7735.h"
#include "drv/adc/adc.h"
#include "drv/BME280/bme280.h"
#include "drv/i2c/i2c.h"
#include "rtc.h"
#include "weather_station_status.h"
#include "gui/weather_station_ui.h"
#include "gui/gui_layout.h"
#include "gui/embedded_gui.h"
//hello
volatile int MCLKfreq, SMCLKfreq;

void clockInit48MHzXTL(void) {  // sets the clock module to use the external 48 MHz crystal

    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    CS_setExternalClockSourceFrequency(32000,48000000); // enables getMCLK, getSMCLK to know externally set frequencies

    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);  // false means that there are no timeouts set, will return when stable

    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

// Subroutine to wait 1 msec (assumes 48 MHz clock)
// Inputs: n  number of 1 msec to wait
// Outputs: None
// Notes: implemented in ST7735.c as count of assembly instructions executed
void Delay1ms(uint32_t n);
// Subroutine to wait 10 msec
// Inputs: n  number of 10 msec to wait
// Outputs: None
// Notes: calls Delay1ms repeatedly
void DelayWait10ms(uint32_t n){
  Delay1ms(n*10);
}



int main(void)
{
    clockInit48MHzXTL();  // set up the clock to use the crystal oscillator on the Launchpad
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_2);
//    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);  // use this if the crystal oscillator does not respond
//    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
//    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);  // set SMCLK to 12 MHz
    SMCLKfreq=MAP_CS_getSMCLK();  // get SMCLK value to verify it was set correctly
    MCLKfreq=MAP_CS_getMCLK();  // get MCLK value

    init_ADC();

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
    DelayWait10ms(100);

    I2C_Init();

    struct bme280_dev bme280_dev =
    {
        .dev_id = BME280_I2C_ADDR_PRIM,
        .intf = BME280_I2C_INTF,
        .read = &I2C_WRITE_READ_STRING,
        .write = &I2C_WRITE_STRING,
        .delay_ms = Delay1ms,
        .settings.osr_h = BME280_OVERSAMPLING_1X,
        .settings.osr_p = BME280_OVERSAMPLING_1X,
        .settings.osr_t = BME280_OVERSAMPLING_1X,
        .settings.filter = BME280_FILTER_COEFF_OFF
    };
    bme280_init(&bme280_dev);
    bme280_set_sensor_settings(BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL, &bme280_dev);

    ST7735_InitR(INITR_REDTAB); // initialize LCD controller IC

    // initialize button

    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);


    ST7735_FillScreen(0);

    struct weather_station_status status = (struct weather_station_status) {
        .lighting = lighting_dark,
        .outdoor_temperature = 0.0f,
        .indoor_temperature = 0.0f,
        .outdoor_humidity = 0.0f,
        .indoor_humidity = 0.0f,
        .pressure = 0.0f,
        .time = (struct rtc_time) {
                .sec = 0,
                .min = 0,
                .hour = 12,
                .date = 15,
                .month = 1,
                .year = 2018
            }
    };

    int lighting_index = 0;
    while(1) {  // loop through the test functions to demonstrate the LCD capabilities

        MAP_ADC14_toggleConversionTrigger();

        int light_reading = ADC.result[1];

        if(light_reading < 100) {lighting_index = 0;}
        else if(light_reading < 3000) {lighting_index = 1;}
        else if(light_reading < 7000) {lighting_index = 2;}
        else if(light_reading < 10000) {lighting_index = 3;}
        else {lighting_index = 4;}

        //if(lighting_index != status.lighting)
        //{
            status.lighting = lighting_index;
            draw_weather_station_ui(status);
        //}

        bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280_dev);
        struct bme280_data sensor_data = {0};
        int8_t err = bme280_get_sensor_data(BME280_ALL, &sensor_data, &bme280_dev);

        status.pressure = sensor_data.pressure * 0.000002953;
        status.indoor_temperature = (sensor_data.temperature * 0.01) * (9.0 / 5.0) + 32.0;
        status.indoor_humidity = sensor_data.humidity * (1.0 / 1024.0);

        DelayWait10ms(100);
    }
}

