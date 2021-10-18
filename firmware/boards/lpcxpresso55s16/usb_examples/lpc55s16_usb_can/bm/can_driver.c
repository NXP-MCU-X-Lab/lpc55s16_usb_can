#include "fsl_mcan.h"


#define CAN_DATASIZE (8)
/* If user need to auto execute the improved timming configuration. */


#define EXAMPLE_MCAN               CAN0
#define MCAN_CLK_FREQ              CLOCK_GetMCanClkFreq()
#define STDID_OFFSET               (18U)
#define MSG_RAM_BASE               0x04000000U
#define STD_FILTER_OFS 0x0
#define RX_FIFO0_OFS   0x10U
#define TX_BUFFER_OFS  0x20U
#define MSG_RAM_SIZE   (TX_BUFFER_OFS + 8 + CAN_DATASIZE)


void can_rx_cb(uint32_t id, uint8_t *buf, uint32_t len);

mcan_tx_buffer_frame_t txFrame;
mcan_rx_buffer_frame_t rxFrame;
uint8_t rx_data[CAN_DATASIZE];
mcan_handle_t mcanHandle;
mcan_buffer_transfer_t txXfer;
mcan_fifo_transfer_t rxXfer;
uint32_t txIdentifier;
uint32_t rxIdentifier;
#define msgRam MSG_RAM_BASE



static void mcan_callback(CAN_Type *base, mcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
    switch (status)
    {
        case kStatus_MCAN_RxFifo0Idle:
        {

            memcpy(rx_data, rxFrame.data, rxFrame.size);
            MCAN_TransferReceiveFifoNonBlocking(EXAMPLE_MCAN, 0, &mcanHandle, &rxXfer);
            can_rx_cb(rxFrame.id >> STDID_OFFSET, rx_data, rxFrame.size);
        }
        break;

        case kStatus_MCAN_TxIdle:
        {

        }
        break;

        default:
            break;
    }
}



void app_can_init(void)
{
    mcan_config_t mcanConfig;
    mcan_frame_filter_config_t rxFilter;
    mcan_std_filter_element_config_t stdFilter;
    mcan_rx_fifo_config_t rxFifo0;
    mcan_tx_buffer_config_t txBuffer;
    uint8_t node_type;
    uint8_t numMessage = 0;
    uint8_t cnt        = 0;
    
    /* Set MCAN clock 100Mhz/5=20MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivCanClk, 5U, true);
    CLOCK_AttachClk(kMCAN_DIV_to_MCAN);
    
    
    txIdentifier = 0x321U;
    rxIdentifier = 0x123U;

    printf("TXID:0x%X RXID:0x%X\r\n", txIdentifier, rxIdentifier);
    MCAN_GetDefaultConfig(&mcanConfig);


    mcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(timing_config));

    if (MCAN_CalculateImprovedTimingValues(mcanConfig.baudRateA, MCAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(mcanConfig.timingConfig), &timing_config, sizeof(mcan_timing_config_t));
    }
    else
    {
        printf("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
    }


    MCAN_Init(EXAMPLE_MCAN, &mcanConfig, MCAN_CLK_FREQ);

    /* Create MCAN handle structure and set call back function. */
    MCAN_TransferCreateHandle(EXAMPLE_MCAN, &mcanHandle, mcan_callback, NULL);

    /* Set Message RAM base address and clear to avoid BEU/BEC error. */
    MCAN_SetMsgRAMBase(EXAMPLE_MCAN, (uint32_t)msgRam);
    memset((void *)msgRam, 0, MSG_RAM_SIZE * sizeof(uint8_t));

    /* STD filter config. */
    rxFilter.address  = STD_FILTER_OFS;
    rxFilter.idFormat = kMCAN_FrameIDStandard;
    rxFilter.listSize = 1U;
    rxFilter.nmFrame  = kMCAN_reject0;
    rxFilter.remFrame = kMCAN_rejectFrame;
    MCAN_SetFilterConfig(EXAMPLE_MCAN, &rxFilter);

    stdFilter.sfec = kMCAN_storeinFifo0;
    /* Classic filter mode, only filter matching ID. */
    stdFilter.sft   = kMCAN_classic;
    stdFilter.sfid1 = rxIdentifier;
    stdFilter.sfid2 = 0x7FFU;
    MCAN_SetSTDFilterElement(EXAMPLE_MCAN, &rxFilter, &stdFilter, 0);

    /* RX fifo0 config. */
    rxFifo0.address       = RX_FIFO0_OFS;
    rxFifo0.elementSize   = 1U;
    rxFifo0.watermark     = 0;
    rxFifo0.opmode        = kMCAN_FifoBlocking;
    rxFifo0.datafieldSize = kMCAN_8ByteDatafield;
    MCAN_SetRxFifo0Config(EXAMPLE_MCAN, &rxFifo0);

    /* TX buffer config. */
    memset(&txBuffer, 0, sizeof(txBuffer));
    txBuffer.address       = TX_BUFFER_OFS;
    txBuffer.dedicatedSize = 1U;
    txBuffer.fqSize        = 0;
    txBuffer.datafieldSize = kMCAN_8ByteDatafield;

    MCAN_SetTxBufferConfig(EXAMPLE_MCAN, &txBuffer);

    /* Finish software initialization and enter normal mode, synchronizes to
       CAN bus, ready for communication */
    MCAN_EnterNormalMode(EXAMPLE_MCAN);


    /* Start receive data through Rx FIFO 0. */
    memset(rx_data, 0, sizeof(uint8_t) * CAN_DATASIZE);
    /* the MCAN engine can't auto to get rx payload size, we need set it. */
    rxFrame.size = CAN_DATASIZE;
    rxXfer.frame = &rxFrame;
    MCAN_TransferReceiveFifoNonBlocking(EXAMPLE_MCAN, 0, &mcanHandle, &rxXfer);

}


uint32_t app_can_send(uint32_t id, uint8_t *buf, uint8_t len)
{
    txFrame.xtd  = kMCAN_FrameIDStandard;
    txFrame.rtr  = kMCAN_FrameTypeData;
    txFrame.fdf  = 0;
    txFrame.brs  = 0;
    txFrame.dlc  = len;
    txFrame.id   = txIdentifier << STDID_OFFSET;
    txFrame.data = buf;
    txFrame.size = CAN_DATASIZE;

    txXfer.frame     = &txFrame;
    txXfer.bufferIdx = 0;
    MCAN_TransferSendNonBlocking(EXAMPLE_MCAN, &mcanHandle, &txXfer);

}




