#include "adapters/ranging/stateless/dw3000_impl.h"

#include <cstddef>
#include <cstdint>

#include "domain/models/ranging.h"
#include "dw3000.h"

namespace adapters::ranging::stateless
{
    constexpr const char *initiateTag = "ranging::stateless::DW3000Impl::initiate";
    constexpr const char *listenTag = "ranging::stateless::DW3000Impl::listen";

    // Helpers

    constexpr uint8_t lowOrder(uint16_t value)
    {
        return static_cast<uint8_t>(value & 0xFF);
    }

    constexpr uint8_t highOrder(uint16_t value)
    {
        return static_cast<uint8_t>((value >> 8) & 0xFF);
    }

    inline uint16_t getUint16(const uint8_t *buffer, std::size_t low_index)
    {
        return static_cast<uint16_t>(buffer[low_index] | (buffer[low_index + 1] << 8));
    }

    void fillFrameHeader(uint8_t *frame, uint8_t sequence_number, uint16_t pan_id, uint16_t destination_address, uint16_t source_address, uint8_t function_code)
    {
        frame[models::RangingFrameIndex::FrameControlLow] = lowOrder(models::RangingFrameControl::ShortAddress);
        frame[models::RangingFrameIndex::FrameControlHigh] = highOrder(models::RangingFrameControl::ShortAddress);
        frame[models::RangingFrameIndex::SequenceNumber] = sequence_number;
        frame[models::RangingFrameIndex::PanIdLow] = lowOrder(pan_id);
        frame[models::RangingFrameIndex::PanIdHigh] = highOrder(pan_id);
        frame[models::RangingFrameIndex::DestinationAddressLow] = lowOrder(destination_address);
        frame[models::RangingFrameIndex::DestinationAddressHigh] = highOrder(destination_address);
        frame[models::RangingFrameIndex::SourceAddressLow] = lowOrder(source_address);
        frame[models::RangingFrameIndex::SourceAddressHigh] = highOrder(source_address);
        frame[models::RangingFrameIndex::FunctionCode] = function_code;
    }

    bool hasExpectedHeader(const uint8_t *frame, uint16_t pan_id, uint16_t destination_address, uint16_t source_address, uint8_t function_code)
    {
        return frame[models::RangingFrameIndex::FrameControlLow] == lowOrder(models::RangingFrameControl::ShortAddress) &&
               frame[models::RangingFrameIndex::FrameControlHigh] == highOrder(models::RangingFrameControl::ShortAddress) &&
               getUint16(frame, models::RangingFrameIndex::PanIdLow) == pan_id &&
               getUint16(frame, models::RangingFrameIndex::DestinationAddressLow) == destination_address &&
               getUint16(frame, models::RangingFrameIndex::SourceAddressLow) == source_address &&
               frame[models::RangingFrameIndex::FunctionCode] == function_code;
    }

    models::Error receiveFailureFromStatus(uint32_t status_reg, models::Error timeout_error, models::Error failed_error)
    {
        if (status_reg & SYS_STATUS_ALL_RX_TO)
            return timeout_error;

        return failed_error;
    }

    models::Error receiveFrame(
        ports::driven::logger::Leveled *logger,
        const char *tag,
        uint8_t *buffer,
        std::size_t buffer_len,
        uint32_t *frame_len,
        uint32_t timeout_uus,
        models::Error timeout_error,
        models::Error failed_error,
        models::Error too_long_error)
    {
        dwt_setrxtimeout(timeout_uus);

        if (dwt_rxenable(DWT_START_RX_IMMEDIATE) != DWT_SUCCESS)
        {
            logger->error(tag, "Failed to enable RX (timeout_uus=%lu)", static_cast<unsigned long>(timeout_uus));
            return failed_error;
        }

        uint32_t status_reg = 0;
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
                 (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        {
        }

        if (!(status_reg & SYS_STATUS_RXFCG_BIT_MASK))
        {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            logger->error(
                tag,
                "RX failed (status=0x%08lX timeout_uus=%lu)",
                static_cast<unsigned long>(status_reg),
                static_cast<unsigned long>(timeout_uus));
            return receiveFailureFromStatus(status_reg, timeout_error, failed_error);
        }

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        *frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (*frame_len > buffer_len)
        {
            logger->error(
                tag,
                "RX frame too long (frame_len=%lu buffer_len=%u)",
                static_cast<unsigned long>(*frame_len),
                static_cast<unsigned int>(buffer_len));
            return too_long_error;
        }

        dwt_readrxdata(buffer, *frame_len, 0);
        return models::Error::Ok;
    }

    // Adapter implementations

    DW3000Impl::DW3000Impl(
        ports::driven::logger::Leveled *logger,
        uint16_t tx_antenna_delay,
        uint32_t poll_tx_to_resp_rx_delay_uus,
        uint32_t poll_rx_to_resp_tx_delay_uus)
        : sequence_number(0),
          tx_antenna_delay(tx_antenna_delay),
          poll_tx_to_resp_rx_delay_uus(poll_tx_to_resp_rx_delay_uus),
          poll_rx_to_resp_tx_delay_uus(poll_rx_to_resp_tx_delay_uus),
          logger(logger)
    {
    }

    models::Error DW3000Impl::initiate(
        uint16_t pan_id,
        uint16_t source_address,
        uint16_t destination_address,
        uint32_t timeout_uus,
        float *distance)
    {
        if (distance == nullptr)
        {
            logger->error(initiateTag, "Invalid argument: distance is null");
            return models::Error::InvalidArgument;
        }

        if (timeout_uus == 0)
        {
            logger->error(initiateTag, "Invalid argument: timeout is 0");
            return models::Error::InvalidArgument;
        }

        uint8_t tx_poll[models::RangingFrameLength::Poll] = {};
        fillFrameHeader(tx_poll, sequence_number, pan_id, destination_address, source_address, models::RangingFunctionCode::Poll);

        dwt_setrxaftertxdelay(poll_tx_to_resp_rx_delay_uus);
        dwt_setrxtimeout(timeout_uus);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX | SYS_STATUS_ALL_RX_GOOD | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_writetxdata(sizeof(tx_poll), tx_poll, 0);
        dwt_writetxfctrl(sizeof(tx_poll), 0, 1);

        if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS)
        {
            logger->error(
                initiateTag,
                "Failed to start poll TX (pan_id=0x%04X source=0x%04X destination=0x%04X)",
                static_cast<unsigned int>(pan_id),
                static_cast<unsigned int>(source_address),
                static_cast<unsigned int>(destination_address));
            return models::Error::UwbPollTxFailed;
        }

        uint32_t status_reg = 0;
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
                 (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        {
        }
        sequence_number++;

        if (!(status_reg & SYS_STATUS_RXFCG_BIT_MASK))
        {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
            logger->error(
                initiateTag,
                "Response RX failed (status=0x%08lX timeout_uus=%lu)",
                static_cast<unsigned long>(status_reg),
                static_cast<unsigned long>(timeout_uus));
            return receiveFailureFromStatus(status_reg, models::Error::UwbResponseRxTimeout, models::Error::UwbResponseRxFailed);
        }

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        uint8_t rx_resp[models::RangingFrameLength::Resp] = {};
        uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
        if (frame_len > sizeof(rx_resp))
        {
            logger->error(
                initiateTag,
                "Response frame too long (frame_len=%lu buffer_len=%u)",
                static_cast<unsigned long>(frame_len),
                static_cast<unsigned int>(sizeof(rx_resp)));
            return models::Error::UwbResponseFrameTooLong;
        }

        dwt_readrxdata(rx_resp, frame_len, 0);
        if (frame_len < models::RangingFrameLength::Resp ||
            !hasExpectedHeader(rx_resp, pan_id, source_address, destination_address, models::RangingFunctionCode::Resp))
        {
            logger->error(
                initiateTag,
                "Unexpected response frame (frame_len=%lu pan_id=0x%04X source=0x%04X destination=0x%04X function=0x%02X)",
                static_cast<unsigned long>(frame_len),
                static_cast<unsigned int>(getUint16(rx_resp, models::RangingFrameIndex::PanIdLow)),
                static_cast<unsigned int>(getUint16(rx_resp, models::RangingFrameIndex::SourceAddressLow)),
                static_cast<unsigned int>(getUint16(rx_resp, models::RangingFrameIndex::DestinationAddressLow)),
                static_cast<unsigned int>(rx_resp[models::RangingFrameIndex::FunctionCode]));
            return models::Error::UwbUnexpectedResponseFrame;
        }

        const uint32_t poll_tx_ts = dwt_readtxtimestamplo32();
        const uint32_t resp_rx_ts = dwt_readrxtimestamplo32();
        const float clock_offset_ratio = static_cast<float>(dwt_readclockoffset()) / static_cast<uint32_t>(1 << 26);

        uint32_t poll_rx_ts = 0;
        uint32_t resp_tx_ts = 0;
        resp_msg_get_ts(&rx_resp[models::RangingFrameIndex::RespPollRxTime], &poll_rx_ts);
        resp_msg_get_ts(&rx_resp[models::RangingFrameIndex::RespRespTxTime], &resp_tx_ts);

        const int32_t rtd_init = static_cast<int32_t>(resp_rx_ts - poll_tx_ts);
        const int32_t rtd_resp = static_cast<int32_t>(resp_tx_ts - poll_rx_ts);
        const double tof = ((rtd_init - rtd_resp * (1.0 - clock_offset_ratio)) / 2.0) * DWT_TIME_UNITS;
        *distance = static_cast<float>(tof * SPEED_OF_LIGHT);

        return models::Error::Ok;
    }

    models::Error DW3000Impl::listen(
        uint16_t pan_id,
        uint16_t destination_address,
        uint16_t source_address,
        uint32_t timeout_uus)
    {
        if (timeout_uus == 0)
        {
            logger->error(listenTag, "Invalid argument: timeout is 0");
            return models::Error::InvalidArgument;
        }

        uint8_t rx_poll[models::RangingFrameLength::Poll] = {};
        uint32_t frame_len = 0;

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        const models::Error receive_error = receiveFrame(
            logger,
            listenTag,
            rx_poll,
            sizeof(rx_poll),
            &frame_len,
            timeout_uus,
            models::Error::UwbPollRxTimeout,
            models::Error::UwbPollRxFailed,
            models::Error::UwbPollFrameTooLong);
        if (receive_error != models::Error::Ok)
            return receive_error;

        if (frame_len < models::RangingFrameLength::Poll ||
            !hasExpectedHeader(rx_poll, pan_id, destination_address, source_address, models::RangingFunctionCode::Poll))
        {
            logger->error(
                listenTag,
                "Unexpected poll frame (frame_len=%lu pan_id=0x%04X source=0x%04X destination=0x%04X function=0x%02X)",
                static_cast<unsigned long>(frame_len),
                static_cast<unsigned int>(getUint16(rx_poll, models::RangingFrameIndex::PanIdLow)),
                static_cast<unsigned int>(getUint16(rx_poll, models::RangingFrameIndex::SourceAddressLow)),
                static_cast<unsigned int>(getUint16(rx_poll, models::RangingFrameIndex::DestinationAddressLow)),
                static_cast<unsigned int>(rx_poll[models::RangingFrameIndex::FunctionCode]));
            return models::Error::UwbUnexpectedPollFrame;
        }

        uint8_t tx_resp[models::RangingFrameLength::Resp] = {};
        fillFrameHeader(tx_resp, sequence_number, pan_id, source_address, destination_address, models::RangingFunctionCode::Resp);

        const uint64_t poll_rx_ts = get_rx_timestamp_u64();
        const uint32_t resp_tx_time = static_cast<uint32_t>((poll_rx_ts + (poll_rx_to_resp_tx_delay_uus * UUS_TO_DWT_TIME)) >> 8);
        dwt_setdelayedtrxtime(resp_tx_time);

        const uint64_t resp_tx_ts = (static_cast<uint64_t>(resp_tx_time & 0xFFFFFFFEUL) << 8) + tx_antenna_delay;
        resp_msg_set_ts(&tx_resp[models::RangingFrameIndex::RespPollRxTime], poll_rx_ts);
        resp_msg_set_ts(&tx_resp[models::RangingFrameIndex::RespRespTxTime], resp_tx_ts);

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX);
        dwt_writetxdata(sizeof(tx_resp), tx_resp, 0);
        dwt_writetxfctrl(sizeof(tx_resp), 0, 1);

        if (dwt_starttx(DWT_START_TX_DELAYED) != DWT_SUCCESS)
        {
            logger->error(
                listenTag,
                "Failed to start delayed response TX (pan_id=0x%04X source=0x%04X destination=0x%04X)",
                static_cast<unsigned int>(pan_id),
                static_cast<unsigned int>(destination_address),
                static_cast<unsigned int>(source_address));
            return models::Error::UwbResponseTxFailed;
        }

        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
        {
        }

        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        sequence_number++;
        return models::Error::Ok;
    }
}
