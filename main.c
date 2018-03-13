/* DriverLib Includes */
#include "driverlib.h"

#include "drv/timing.h"
#include "outsystem.h"
#include "insystem.h"

//#define INSIDE_MODULE // comment this out for outside module

int main(void)
{
    for(int i = 0; i <= 10; i++)
    {
        MAP_GPIO_setAsOutputPin(i, PIN_ALL8);
        MAP_GPIO_setOutputLowOnPin(i, PIN_ALL8);
    }

    init_clocks();
    expect_frequency(CS_MCLK, 48000000);
    expect_frequency(CS_SMCLK, 24000000);
    rtc_init();
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    Interrupt_enableMaster();

#ifdef INSIDE_MODULE
    initInSystem();
#else
    initOutSystem();
#endif
    while(1) {
#ifdef INSIDE_MODULE
        runInSystem();
#else
        runOutSystem();
#endif
        while(!rtc_second_passed)
        {
            MAP_PCM_gotoLPM0();
        }
        rtc_second_passed = false;
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

