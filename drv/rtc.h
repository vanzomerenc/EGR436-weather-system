#ifndef RTC_H_
#define RTC_H_

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

enum IntervalSetting { POLL_SECOND, POLL_MINUTE, POLL_HOUR };

extern char const * const rtc_day_name[8];

int rtc_timechanged(struct rtc_time *current);
void rtc_init();
void rtc_gettime(struct rtc_time *result);
void rtc_settime(struct rtc_time *time);
void rtc_gettemp(float *result);
void rtc_format(struct rtc_time *time, char *result, size_t length);
int rtc_getinterval();
void rtc_setinterval(int setting);

//int rtc_decToHex(int dec);
#endif // RTC_H_