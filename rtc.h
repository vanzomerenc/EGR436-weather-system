#pragma once

#include <stddef.h>
#include <driverlib.h>
struct rtc_time
{
	int sec;
	int min;
	int hour;
	int day_of_week;
	int date;
	int month;
	int year;
};

extern char const * const rtc_day_name[8];

void rtc_init();
void rtc_gettime(struct rtc_time *result);
void rtc_settime(struct rtc_time *time);

void rtc_gettemp(float *result);

void rtc_format(struct rtc_time *time, char *result, size_t length);

int rtc_decToHex(int dec);
