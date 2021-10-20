#include "fsl_usart.h"
#include "fsl_mcan.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_usart.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
#include "usbtin.h"


void app_usbd_cdc_init(void);

void usbd_cdc_send(uint8_t *buf, uint32_t len);

static void app_can_init(void);
uint32_t app_can_send(uint32_t id, uint8_t *buf, uint8_t len);



void cdc_rx_cb(uint8_t *buf, uint32_t len)
{
    int i;
    
    for(i=0; i<len; i++)
    {
        usbtin_input(buf[i]);
    }
}



void can_rx_cb(uint32_t id, uint8_t *buf, uint32_t len)
{
    usbtin_can_input(id, buf, len);
}


const usbtin_ops_t usbtin_ops = 
{
    usbd_cdc_send,
    app_can_send,
};

void main(void)
{
        /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    
    BOARD_InitDebugConsole();
    USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    EnableIRQ(FLEXCOMM0_IRQn);
    
    app_usbd_cdc_init();
    app_can_init();
    
    usbtin_init(&usbtin_ops);
    while(1)
    {
        
    }
}




void FLEXCOMM0_IRQHandler(void)
{
    uint8_t data;

    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(USART0))
    {
        data = USART_ReadByte(USART0);
        



    }
}
