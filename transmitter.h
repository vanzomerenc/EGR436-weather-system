#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

// AUTHOR: Chris van Zomeren

/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include "drv/nrf24/msprf24.h"
#include "comm.h"

struct comm_message transmitted_message;
uint8_t num_retransmits = 0;

void transmitterInit()
{
    spi_init();

    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    MAP_Interrupt_enableInterrupt(INT_PORT2);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1);

    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width      = 5;
    rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
    rf_channel         = 42;

    msprf24_init();

    while(!msprf24_is_alive()) {}

    msprf24_set_pipe_packetsize(0, 32);
    msprf24_open_pipe(0, 1);
    msprf24_standby();

    uint8_t rx_addr[5] = {'B', 'E', 'A', 'N', 'S'};

    w_tx_addr(rx_addr);
    w_rx_addr(0, rx_addr);

}

void transmitterRoutine(struct sensor_atmospheric_result *atmospheric, float light_level)
{
    comm_encode_sensor_readings(atmospheric, light_level, &transmitted_message);
    w_tx_payload(32, transmitted_message.data);
    msprf24_activate_tx();

    while(!(rf_irq & RF24_IRQ_FLAGGED))
        ;

    if (rf_irq & RF24_IRQ_FLAGGED) {
        msprf24_get_irq_reason();
        if (rf_irq & RF24_IRQ_TXFAILED){
            P2OUT &= ~BIT1; // Green LED off
            P2OUT |= BIT0; // Red LED on
        }
        else
        {
            P2OUT &= ~BIT0;
            P2OUT |= BIT1; // Green LED on
        }

        msprf24_irq_clear(rf_irq);
        num_retransmits = msprf24_get_last_retransmits();
    }
}
#endif // TRANSMITTER_H_
