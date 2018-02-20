#include "rtc.h"

/* Statics */
// @NOTE ST 2-19-2018 This is what gets updated by the MSP432's RTC
 static volatile RTC_C_Calendar newTime;

 /* Time is November 12th 1955 10:03:00 PM */
 const RTC_C_Calendar currentTime =
 {
         .seconds = 0,
         .minutes = 3,
         .hours = 22,
         .dayOfmonth = 12,
         .month = 11,
         .year = 1955
 };

void rtc_init()
{
    /* Configuring pins for peripheral/crystal usage and LED for output */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Initializing RTC with current time as described in time in
         * definitions section */
    MAP_RTC_C_initCalendar(&currentTime, RTC_C_FORMAT_BINARY);

    /* Specify an interrupt to assert every minute */
    MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    /* Enable interrupt for RTC Ready Status, which asserts when the RTC
     * Calendar registers are ready to read.
     * Also, enable interrupts for the Calendar alarm and Calendar event. */
    MAP_RTC_C_clearInterruptFlag(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                   );
    MAP_RTC_C_enableInterrupt(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                   );

    /* Start RTC Clock */
    MAP_RTC_C_startClock();

    /* Enable interrupts  */
    MAP_Interrupt_enableInterrupt(INT_RTC_C);

}

void rtc_gettime(struct rtc_time *result)
{
    result->hour = newTime.hours;
    result->sec = newTime.seconds;
    result->min = newTime.minutes;
    result->date = newTime.dayOfmonth;
    result->month = newTime.month;
    result->year = newTime.year;
}

// Make sure to set values as hex
void rtc_settime(struct rtc_time *time)
{
    RTC_C_Calendar toBeSetTime;
    toBeSetTime.seconds = time->sec;
    toBeSetTime.minutes = time->min;
    toBeSetTime.hours = time->hour;
    toBeSetTime.dayOfmonth = time->date;
    toBeSetTime.month = time->month;
    toBeSetTime.year = time->year;

    MAP_RTC_C_holdClock(); // Unlock the RTC so we can write to the registers
    MAP_RTC_C_initCalendar(&toBeSetTime, RTC_C_FORMAT_BINARY ); // Init with new date/time
    // Clean up interrupts
    MAP_RTC_C_clearInterruptFlag(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                    | RTC_C_CLOCK_ALARM_INTERRUPT);
    MAP_RTC_C_enableInterrupt(
            RTC_C_CLOCK_READ_READY_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT
                    | RTC_C_CLOCK_ALARM_INTERRUPT);
    // Start clock again
    MAP_RTC_C_startClock();
}

void rtc_gettemp(float *result)
{

}

void rtc_format(struct rtc_time *time, char *result, size_t length)
{

}

/* RTC ISR */
void RTC_C_IRQHandler(void)
{
    uint32_t status;

    status = MAP_RTC_C_getEnabledInterruptStatus();
    MAP_RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_CLOCK_READ_READY_INTERRUPT)
    {
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        // For testing...
        newTime = MAP_RTC_C_getCalendarTime();
    }

    if (status & RTC_C_TIME_EVENT_INTERRUPT)
    {
        /* Interrupts every minute - Set breakpoint here */
        __no_operation();
        newTime = MAP_RTC_C_getCalendarTime();

    }

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT)
    {
        __no_operation();
    }

}

// TODO Interrupt for user with terminal?
