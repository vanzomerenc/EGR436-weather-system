/* DriverLib Includes */
#include "driverlib.h"

#include "drv/timing.h"
#include "outsystem.h"
#include "insystem.h"

#define INSIDE_MODULE // comment this out for outside module

int main(void)
{
    init_clocks();
    expect_frequency(CS_MCLK, 48000000);
    expect_frequency(CS_SMCLK, 24000000);
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
        delay_ms(1000);
    }
}

