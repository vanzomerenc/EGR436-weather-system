/* DriverLib Includes */
#include "driverlib.h"

#include "wifi.h"
#include "insystem.h"
#include "outsystem.h"
#include "drv/timing.h"

// PRELAB 9 MAIN
int main(void)
{
    for(int i = 0; i <= 10; i++)
    {
        MAP_GPIO_setAsOutputPin(i, PIN_ALL8);
        MAP_GPIO_setOutputLowOnPin(i, PIN_ALL8);
    }

    init_clocks();
    expect_frequency(CS_MCLK, 48000000);
    expect_frequency(CS_SMCLK, 12000000);
    wifiInit();
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    Interrupt_enableMaster();

    while(1) {
    }
}

