#pragma once

#include <drv/rtc.h>
#include <sensors/atmospheric.h>

enum comm_message_type
{
    COMM_MESSAGE_RTC_TIME,
    COMM_MESSAGE_SENSORS
};

struct comm_message
{
    uint8_t data[32];
};

void comm_encode_rtc_time(struct rtc_time *time, struct comm_message *out_message);
int comm_decode_rtc_time(struct comm_message *message, struct rtc_time *out_time);

void comm_encode_sensor_readings(struct sensor_atmospheric_result *atmospheric, float light_level, struct comm_message *out_message);
int comm_decode_sensor_readings(struct comm_message *message, struct sensor_atmospheric_result *out_atmospheric, float *out_light_level);
