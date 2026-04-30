#include "adapters/outbound/ranging/peer/dw3000_ss_twr.h"
#include "domain/models/ranging.h"
#include "dw3000.h"

#define TAG_PATH "ranging/peer/DW3000SSTWR"

namespace ao::ranging
{
    DW3000SSTWR::DW3000SSTWR(uint16_t pan_id, uint16_t address, po::logging::Leveled *logger)
        : pan_id(pan_id), address(address), logger(logger) {}

    dom::models::Error DW3000SSTWR::getDistance(uint16_t target, float *distance)
    {
        const char *tag = TAG_PATH "::getDistance";

        if (distance == nullptr)
            return dom::models::Error::InvalidArgument;

        static uint8_t seq_nb = 0;
        uint8_t tx_poll[dom::models::RangingFrameLength::Poll] = {
            0x41,
            0x88,
            seq_nb++,
            (uint8_t)(pan_id & 0xFF),
            (uint8_t)(pan_id >> 8),
            (uint8_t)(target & 0xFF),
            (uint8_t)(target >> 8),
            (uint8_t)(address & 0xFF),
            (uint8_t)(address >> 8),
            dom::models::RangingFunctionCode::Poll,
            0,
            0,
        };

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        dwt_writetxdata(dom::models::RangingFrameLength::Poll, tx_poll, 0);
        dwt_writetxfctrl(dom::models::RangingFrameLength::Poll, 0, 1);
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

        uint32_t status_reg;
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
                 (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        {
        }

        if (!(status_reg & SYS_STATUS_RXFCG_BIT_MASK))
        {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            if (logger)
                logger->error(tag, "No response from target 0x%04X", target);
            return dom::models::Error::SystemFail;
        }

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        uint8_t rx_buffer[dom::models::RangingFrameLength::Resp];
        uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (frame_len > dom::models::RangingFrameLength::Resp)
        {
            if (logger)
                logger->error(tag, "Unexpected frame length: %lu", frame_len);
            return dom::models::Error::SystemFail;
        }

        dwt_readrxdata(rx_buffer, frame_len, 0);

        bool valid = rx_buffer[9] == dom::models::RangingFunctionCode::Resp &&
                     (uint16_t)(rx_buffer[dom::models::RangingFrameIndex::SourceAddressLow] |
                                (rx_buffer[dom::models::RangingFrameIndex::SourceAddressHigh] << 8)) == target &&
                     (uint16_t)(rx_buffer[dom::models::RangingFrameIndex::DestinationAddressLow] |
                                (rx_buffer[dom::models::RangingFrameIndex::DestinationAddressHigh] << 8)) == address;
        if (!valid)
        {
            if (logger)
                logger->error(tag, "Frame validation failed (target=0x%04X)", target);
            return dom::models::Error::SystemFail;
        }

        uint32_t poll_tx_ts = dwt_readtxtimestamplo32();
        uint32_t resp_rx_ts = dwt_readrxtimestamplo32();
        float clock_offset = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

        uint32_t poll_rx_ts, resp_tx_ts;
        resp_msg_get_ts(&rx_buffer[dom::models::RangingFrameIndex::RespPollRxTime], &poll_rx_ts);
        resp_msg_get_ts(&rx_buffer[dom::models::RangingFrameIndex::RespRespTxTime], &resp_tx_ts);

        int32_t rtd_init = (int32_t)(resp_rx_ts - poll_tx_ts);
        int32_t rtd_resp = (int32_t)(resp_tx_ts - poll_rx_ts);
        double tof = ((rtd_init - rtd_resp * (1.0 - clock_offset)) / 2.0) * DWT_TIME_UNITS;
        *distance = (float)(tof * SPEED_OF_LIGHT);

        if (logger)
            logger->info(tag, "Distance retrieved from 0x%04X %.3f m", target, *distance);

        return dom::models::Error::Ok;
    }
}
