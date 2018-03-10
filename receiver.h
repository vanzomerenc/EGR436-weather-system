/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include <drv/msprf24.h>
volatile unsigned int user;

char buf[32];

void receiverInit()
{
    char addr[5];
    msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst

    // Set our RX address
    //addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00;
    addr[0] = 'B'; addr[1] = 'E'; addr[2] = 'A'; addr[3] = 'N'; addr[4] = 'S';
    w_rx_addr(0, addr);

    // Receive mode
    if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
        flush_rx();
    }

    GPIO_interruptEdgeSelect (GPIO_PORT_P2, GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION );
    MAP_GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    MAP_Interrupt_enableInterrupt(INT_PORT2);
    Interrupt_enableMaster();
    msprf24_activate_rx();
    msprf24_irq_clear(RF24_IRQ_RX);

    int isAlive = msprf24_is_alive();
}

void receiverRoutine()
{
    if (rf_irq & RF24_IRQ_FLAGGED) {
        msprf24_get_irq_reason();
    }
    if (rf_irq & RF24_IRQ_RX) {
        r_rx_payload(32, buf);
        msprf24_irq_clear(RF24_IRQ_RX);
        user = buf[0];

        if (buf[0] == '0')
            P2OUT &= ~BIT0;
        if (buf[0] == '1')
            P2OUT |= BIT0;

    } else {
        user = 0xFF;
    }
}


