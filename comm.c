#include "comm.h"

void comm_encode_rtc_time(struct rtc_time *time, struct comm_message *out_message)
{
    out_message->data[0] = COMM_MESSAGE_RTC_TIME;
    out_message->data[1] = time->year & 0XFF;
    out_message->data[2] = (time->year >> 8) & 0XFF;
    out_message->data[3] = time->month & 0XFF;
    out_message->data[4] = time->day_of_week & 0XFF;
    out_message->data[5] = time->date & 0xFF;
    out_message->data[6] = time->hour & 0xFF;
    out_message->data[7] = time->min & 0xFF;
    out_message->data[8] = time->sec & 0xFF;
}

int comm_decode_rtc_time(struct comm_message *message, struct rtc_time *out_time)
{
    if(message->data[0] != COMM_MESSAGE_RTC_TIME) return -1;

    out_time->year = message->data[1] + (message->data[2] << 8);
    out_time->month = message->data[3];
    out_time->day_of_week = message->data[4];
    out_time->date = message->data[5];
    out_time->hour = message->data[6];
    out_time->min = message->data[7];
    out_time->sec = message->data[8];
    return 0;
}

void comm_encode_sensor_readings(struct sensor_atmospheric_result *atmospheric, float light_level, struct comm_message *out_message)
{
    out_message->data[0] = COMM_MESSAGE_SENSORS;
    memcpy(&out_message->data[1], atmospheric, sizeof(*atmospheric));
    memcpy(&out_message->data[1+sizeof(*atmospheric)], &light_level, sizeof(light_level));
}

int comm_decode_sensor_readings(struct comm_message *message, struct sensor_atmospheric_result *out_atmospheric, float *out_light_level)
{
    if(message->data[0] != COMM_MESSAGE_SENSORS) return -1;

    memcpy(out_atmospheric, &message->data[1], sizeof(*out_atmospheric));
    memcpy(out_light_level, &message->data[1+sizeof(*out_atmospheric)], sizeof(*out_light_level));
    return 0;
}
