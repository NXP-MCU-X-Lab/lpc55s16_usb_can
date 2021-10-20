#include "usbtin.h"
#include "stdio.h"



typedef struct
{
    const usbtin_ops_t      *ops;
    uint8_t                 state;
    uint8_t                 recv_buf[MAX_BUF_SIZE];
    uint8_t                 resp_buf[MAX_BUF_SIZE];
    uint8_t                 resp_buf_len;
    uint8_t                 recv_buf_cnt;
}usbtin_t;

static usbtin_t usbtin;
unsigned char timestamping = 0;


void usbtin_init(const usbtin_ops_t *ops)
{
    usbtin.state = USBTIN_IDLE;
    usbtin.ops = ops;
}

unsigned char parseHex(char * line, unsigned char len, unsigned long * value)
{
    *value = 0;
    while (len--) {
        if (*line == 0) return 0;
        *value <<= 4;
        if ((*line >= '0') && (*line <= '9')) {
           *value += *line - '0';
        } else if ((*line >= 'A') && (*line <= 'F')) {
           *value += *line - 'A' + 10;
        } else if ((*line >= 'a') && (*line <= 'f')) {
           *value += *line - 'a' + 10;
        } else return 0;
        line++;
    }
    return 1;
}


unsigned char parseCmd_transmit(uint8_t *line, uint8_t len)
{
    int i;
    
    canmsg_t canmsg;
    unsigned long temp;
    unsigned char idlen;

    canmsg.flags.rtr = ((line[0] == 'r') || (line[0] == 'R'));

    // upper case -> extended identifier
    if (line[0] < 'Z') {
        canmsg.flags.extended = 1;
        idlen = 8;
    } else {
        canmsg.flags.extended = 0;
        idlen = 3;
    }

    if (!parseHex(&line[1], idlen, &temp)) return 0;
    canmsg.id = temp;

    if (!parseHex(&line[1 + idlen], 1, &temp)) return 0;
    canmsg.dlc = temp;

    if (!canmsg.flags.rtr) {
        unsigned char i;
        unsigned char length = canmsg.dlc;
        if (length > 8) length = 8;
        for (i = 0; i < length; i++) {
            if (!parseHex(&line[idlen + 2 + i*2], 2, &temp)) return 0;
            canmsg.data[i] = temp;
        }
    }
    
    usbtin.ops->can_send(canmsg.id, canmsg.data, canmsg.dlc);
}


char canmsg2ascii_getNextChar(canmsg_t * canmsg, unsigned char * step) {
    
    char ch = BELL;
    char newstep = *step;       
    
    if (*step == RX_STEP_TYPE) {
        
            // type
             
            if (canmsg->flags.extended) {                 
                newstep = RX_STEP_ID_EXT;                
                if (canmsg->flags.rtr) ch = 'R';
                else ch = 'T';
            } else {
                newstep = RX_STEP_ID_STD;
                if (canmsg->flags.rtr) ch = 'r';
                else ch = 't';
            }        
             
    } else if (*step < RX_STEP_DLC) {

	// id        

        unsigned char i = *step - 1;
        unsigned char * id_bp = (unsigned char*) &canmsg->id;
        ch = id_bp[3 - (i / 2)];
        if ((i % 2) == 0) ch = ch >> 4;
        
        ch = ch & 0xF;
        if (ch > 9) ch = ch - 10 + 'A';
        else ch = ch + '0';
        
        newstep++;
        
    } else if (*step < RX_STEP_DATA) {

	// length        

        ch = canmsg->dlc;
        
        ch = ch & 0xF;
        if (ch > 9) ch = ch - 10 + 'A';
        else ch = ch + '0';

        if ((canmsg->dlc == 0) || canmsg->flags.rtr) newstep = RX_STEP_TIMESTAMP;
        else newstep++;
        
    } else if (*step < RX_STEP_TIMESTAMP) {
        
        // data        

        unsigned char i = *step - RX_STEP_DATA;
        ch = canmsg->data[i / 2];
        if ((i % 2) == 0) ch = ch >> 4;
        
        ch = ch & 0xF;
        if (ch > 9) ch = ch - 10 + 'A';
        else ch = ch + '0';
        
        newstep++;        
        if (newstep - RX_STEP_DATA == canmsg->dlc*2) newstep = RX_STEP_TIMESTAMP;
        
    } else if (timestamping && (*step < RX_STEP_CR)) {
        
        // timestamp
        
        unsigned char i = *step - RX_STEP_TIMESTAMP;
        if (i < 2) ch = (canmsg->timestamp >> 8) & 0xff;
        else ch = canmsg->timestamp & 0xff;
        if ((i % 2) == 0) ch = ch >> 4;
        
        ch = ch & 0xF;
        if (ch > 9) ch = ch - 10 + 'A';
        else ch = ch + '0';
        
        newstep++;        
        
    } else {
        
        // linebreak
        
        ch = CR;
        newstep = RX_STEP_FINISHED;
    }
    
    *step = newstep;
    return ch;
}



void usbtin_input(uint8_t ch)
{
    switch(usbtin.state)
    {
        case USBTIN_IDLE:
            usbtin.recv_buf_cnt = 0;
            switch(ch)
            {
                case 'C': /* connect */
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 1;
                    usbtin.resp_buf[0] = 0x0D;
                    break;
                case 'v': /* Get firmware version */
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 4;
                    usbtin.resp_buf[0] = ch;
                    usbtin.resp_buf[1] = VERSION_FIRMWARE_MAJOR;
                    usbtin.resp_buf[2] = VERSION_FIRMWARE_MINOR;
                    usbtin.resp_buf[3] = 0x0D;
                    break;
                case 'V': /*  Get hardware version */
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 4;
                    usbtin.resp_buf[0] = ch;
                    usbtin.resp_buf[1] = VERSION_HARDWARE_MAJOR;
                    usbtin.resp_buf[2] = VERSION_HARDWARE_MINOR;
                    usbtin.resp_buf[3] = 0x0D;
                    break;
                case 'N': /*  Get serial number */
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 5;
                    usbtin.resp_buf[0] = '1';
                    usbtin.resp_buf[1] = '2';
                    usbtin.resp_buf[2] = '3';
                    usbtin.resp_buf[3] = '4';
                    usbtin.resp_buf[4] = 0x0D;
                    break;
                case 'W': // Write given MCP2515 register
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 1;
                    usbtin.resp_buf[0] = 0x0D;
                    break;
                case 'S': // Setup with standard CAN bitrates
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 1;
                    usbtin.resp_buf[0] = 0x0D;
                    break;
                case 'O': // Open CAN channel
                    usbtin.state = USBTIN_CMD;
                    usbtin.resp_buf_len = 1;
                    usbtin.resp_buf[0] = 0x0D;
                    break;
                case 'r': // Transmit standard RTR (11 bit) frame
                case 'R': // Transmit extended RTR (29 bit) frame
                case 't': // Transmit standard (11 bit) frame
                case 'T': // Transmit extended (29 bit) frame
                    usbtin.state = USBTIN_DATA;
                    usbtin.recv_buf[usbtin.recv_buf_cnt++] = ch;
                    usbtin.resp_buf_len = 2;
                    usbtin.resp_buf[0] = 'z';
                    usbtin.resp_buf[1] = 0x0D;
                    break;
            }
            break;
        case USBTIN_DATA:
            usbtin.recv_buf[usbtin.recv_buf_cnt++] = ch;
            if(ch == 0x0D)
            {
                usbtin.state = USBTIN_IDLE;
                usbtin.ops->uart_send(usbtin.resp_buf, usbtin.resp_buf_len);
                parseCmd_transmit(usbtin.recv_buf, usbtin.recv_buf_cnt);
            }
            break;
        case USBTIN_CMD:
            if(ch == 0x0D)
            {
                usbtin.state = USBTIN_IDLE;
                usbtin.ops->uart_send(usbtin.resp_buf, usbtin.resp_buf_len);
            }
            break;
        default:
            printf("unknown");
            break;
    }
}


void usbtin_can_input(uint32_t id, uint8_t *buf, uint32_t len)
{
    int i, cdc_len;
    uint8_t rxstep = 0;
    uint8_t cdc_buf[64];
    canmsg_t msg;
        
    msg.dlc = len;
    msg.id = id;
    msg.flags.extended = 0;
    msg.flags.rtr = 0;
    msg.timestamp = 0;
    for(i=0; i<len; i++)
    {
        msg.data[i] = buf[i];
    }
          
    cdc_len = 0;
    // printf("usart rx:%c\r\n", data);
    while(rxstep != RX_STEP_FINISHED)
    {
        cdc_buf[cdc_len++] = canmsg2ascii_getNextChar(&msg, &rxstep);
    }
    rxstep = 0;

    usbtin.ops->uart_send(cdc_buf, cdc_len);
}






