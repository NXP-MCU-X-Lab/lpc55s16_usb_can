#ifndef _USBTIN_H_
#define _USBTIN_H_

#include <stdint.h>


#define VERSION_HARDWARE_MAJOR '1'
#define VERSION_HARDWARE_MINOR '0'
#define VERSION_FIRMWARE_MAJOR '1'
#define VERSION_FIRMWARE_MINOR '8'

#define MAX_BUF_SIZE    (64)
#define USBTIN_IDLE     (0)
#define USBTIN_CMD      (1)
#define USBTIN_DATA     (2)

#define LINE_MAXLEN 100
#define BELL 7
#define CR 13
#define LR 10

#define RX_STEP_TYPE 0
#define RX_STEP_ID_EXT 1
#define RX_STEP_ID_STD 6
#define RX_STEP_DLC 9
#define RX_STEP_DATA 10
#define RX_STEP_TIMESTAMP 26
#define RX_STEP_CR 30
#define RX_STEP_FINISHED 0xff

typedef struct
{
    uint32_t    (*uart_send)(uint8_t *buf, uint32_t len);
    uint32_t    (*can_send)(uint32_t id, uint8_t *buf, uint32_t len);
}usbtin_ops_t;


// can message data structure
typedef struct
{
    unsigned long id; 			// identifier (11 or 29 bit)
    struct {
       unsigned char rtr : 1;		// remote transmit request
       unsigned char extended : 1;	// extended identifier
    } flags;

    unsigned char dlc;                  // data length code
    unsigned char data[8];		// payload data
    unsigned short timestamp;           // timestamp
} canmsg_t;


void usbtin_input(uint8_t ch);
void usbtin_init(const usbtin_ops_t *ops);
void usbtin_can_input(uint32_t id, uint8_t *buf, uint32_t len);

#endif