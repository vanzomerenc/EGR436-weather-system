/* DriverLib Includes */
#include "driverlib.h"

#include "drv/timing.h"
#include "drv/ST7735/ST7735.h"
#include "drv/ST7735/ST7735.h"
#include "drv/adc/adc.h"
#include "drv/BME280/i2c.h"
#include "drv/BME280/bme280_support.h"
#include "rtc.h"
#include "weather_station_status.h"
#include "gui/weather_station_ui.h"
#include "gui/gui_layout.h"
#include "gui/embedded_gui.h"



int main(void)
{
	init_clocks();
	expect_frequency(CS_MCLK, 48*MHz);
	expect_frequency(CS_SMCLK, 24*MHz);

    init_ADC();
    I2C_Init();

    int32_t err = bme280_data_readout_template();

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

        uint32_t pressure = 0;
        int32_t temperature = 0;
        uint32_t humidity = 0;
        bme280_read_pressure_temperature_humidity(&pressure, &temperature, &humidity);
        status.pressure = pressure * 0.0002953;
        status.indoor_temperature = (temperature * 0.01) * (9.0 / 5.0) + 32.0;
        status.indoor_humidity = humidity * (1.0 / 1024.0);

        delay_ms(1000);
    }
}

