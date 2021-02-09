#pragma once

#include <stdint.h>
#include <system.h>
#include <altera_msgdma.h>
#include <stream_data.hpp>

/**
 * UARTでJetsonへ定期的にデータを送信する
 */
class StreamTransmitter {
public:
    /**
     * 初期化する
     */
    static void Initialize(void) {
        alt_msgdma_dev *dev = alt_msgdma_open(MSGDMA_0_CSR_NAME);
        _Device = dev;
    }

    /**
     * ステータスフラグを送信する
     */
    static void TransmitStatus(void);

    /**
     * ADC2の測定値を送信する
     */
    static void TransmitAdc2(void);

    /**
     * モーションデータを送信する
     */
    static void TransmitMotion(int performance_counter);

private:
    /// mSGDMAのハンドル
    static alt_msgdma_dev *_Device;
};
