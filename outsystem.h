#ifndef OUTSYSTEM_H_
#define OUTSYSTEM_H_

#include "sdcard.h"
#include "driverlib.h"
#include "drv/ST7735/ST7735.h"
#include "drv/adc/adc.h"
#include "drv/i2c/i2c.h"
#include "sensors/atmospheric.h"
#include "drv/rtc.h"
#include "weather_station_status.h"
#include "gui/weather_station_ui.h"
#include "gui/gui_layout.h"
#include "gui/embedded_gui.h"
#include "transmitter.h"

#define IS_OUTSIDE // ST 3-6-2018 Comment this out for the indoor module if naming conflicts occur
#ifdef IS_OUTSIDE
    struct bme280_dev sensor_atmospheric = {0};
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
    struct rtc_time timeToEnter;
#endif // IS_OUTSIDE

// Call first in main
void initOutSystem()
{
    struct adc_channel_config adc_channels[1];
    adc_channels[0] = (struct adc_channel_config){.input_id = 0, .is_high_range = true};
    adc_init(1, adc_channels);
    initUART(); // prelab 7

    sensor_atmospheric_init(&sensor_atmospheric);

    //ST7735_InitR(INITR_REDTAB); // initialize LCD controller IC
    initUART();
    initSD();
    transmitterInit();
    printf("Press enter to set the time.\n");
}

// Call inside while-loop in main
void runOutSystem()
{
    MAP_ADC14_toggleConversionTrigger();
    int pollData = 0;
    float light_reading = 0;
    sensor_light_level_read(&light_reading);
    adc_get_single_raw(0, &light_reading);

    struct sensor_atmospheric_result sensor_atmospheric_result = {0};
    sensor_atmospheric_read(&sensor_atmospheric, &sensor_atmospheric_result);
    status.outdoor_humidity = sensor_atmospheric_result.humidity;
    status.outdoor_temperature = sensor_atmospheric_result.temperature;
    status.pressure = sensor_atmospheric_result.pressure;

    // RTC Functions (Prelab 7)
    if(rtc_timechanged(&status.time))
    {
        rtc_gettime(&status.time);
        pollData = 1;
    }
    processUART(&timeToEnter);

    if(timeSetState == DONE_SETTING) // note: this means user has already set the rate i.e. already finished configuring everything since it's the last step
    {
        if(pollData)
        {
            writeSD(status.time.min, status.time.hour, status.time.date, status.time.month, status.time.year
                    , light_reading, status.outdoor_temperature, status.outdoor_humidity, status.pressure);
            pollData = 0;
        }
    }
    transmitterRoutine(&sensor_atmospheric_result, light_reading);
}


#endif // OUTSYSTEM_H_
