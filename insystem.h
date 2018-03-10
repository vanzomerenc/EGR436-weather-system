#ifndef INSYSTEM_H_
#define INSYSTEM_H_

#include "sdcard.h"
#include "driverlib.h"

#include "drv/timing.h"
#include "drv/ST7735/ST7735.h"
#include "drv/ST7735/ST7735.h"
#include "drv/adc/adc.h"
#include "drv/i2c/i2c.h"
#include "sensors/atmospheric.h"
#include "drv/rtc.h"
#include "weather_station_status.h"
#include "gui/weather_station_ui.h"
#include "gui/gui_layout.h"
#include "gui/embedded_gui.h"
#include "sensors/light_level.h"
#include "outsystem.h"
#include "receiver.h"


struct weather_station_status status;


void initInSystem()
{
    struct adc_channel_config adc_channels[1];
    adc_channels[0] = (struct adc_channel_config){.input_id = 0, .is_high_range = true};
    adc_init(1, adc_channels);
    sensor_light_level_init();

    rtc_init(); // prelab 7
    initUART(); // prelab 7
    receiverInit();

    struct bme280_dev sensor_atmospheric = {0};
    sensor_atmospheric_init(&sensor_atmospheric);

    ST7735_InitR(INITR_REDTAB); // initialize LCD controller IC

    // initialize button
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);

    ST7735_FillScreen(0);


    status = (struct weather_station_status) {
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

    printf("Press enter to set the time.\n");
    struct rtc_time timeToEnter;
    // TODO testing that we can set the RTC registers
    //rtc_settime(&status.time);
}


void runInSystem()
{
    receiverRoutine();
    MAP_ADC14_toggleConversionTrigger();

    float light_reading = 0;
    sensor_light_level_read(&light_reading);

    if(light_reading < 100) {status.lighting = 0;}
    else if(light_reading < 300) {status.lighting = 1;}
    else if(light_reading < 1000) {status.lighting = 2;}
    else if(light_reading < 3000) {status.lighting = 3;}
    else {status.lighting = 4;}

    draw_weather_station_ui(status);

    struct sensor_atmospheric_result sensor_atmospheric_result = {0};
    sensor_atmospheric_read(&sensor_atmospheric, &sensor_atmospheric_result);
    status.indoor_humidity = sensor_atmospheric_result.humidity;
    status.indoor_temperature = sensor_atmospheric_result.temperature;
    status.pressure = sensor_atmospheric_result.pressure;

    // RTC Functions (Prelab 7)
    rtc_gettime(&status.time);
    processUART(&timeToEnter);
}


#endif
