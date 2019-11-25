#include "cc2500_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include "cc2500_rx_spi_common.h"

//#include "platform.h"

#ifdef USE_RX_SPI

//#include "drivers/io.h"
//#include "drivers/time.h"
//#include "rx/rx_spi_common.h"
//#include "rx/rx_spi.h"

static IO_t ledPin;
static bool ledInversion = false;

static IO_t bindPin;
static bool bindRequested;
static bool lastBindPinStatus;

void rxSpiCommonIOInit(const rxSpiConfig_t *rxSpiConfig)
{
    if (rxSpiConfig->ledIoTag) {
        ledPin = IOGetByTag(rxSpiConfig->ledIoTag);
        IOInit(ledPin, OWNER_LED, 0);
        IOConfigGPIO(ledPin, IOCFG_OUT_PP);
        ledInversion = rxSpiConfig->ledInversion;
        rxSpiLedOff();
    } else {
        ledPin = IO_NONE;
    }

    if (rxSpiConfig->bindIoTag) {
        bindPin = IOGetByTag(rxSpiConfig->bindIoTag);
        IOInit(bindPin, OWNER_RX_SPI_BIND, 0);
        IOConfigGPIO(bindPin, IOCFG_IPU);
        lastBindPinStatus = IORead(bindPin);
    } else {
        bindPin = IO_NONE;
    }
}

void rxSpiLedOn(void)
{
    if (ledPin) {
        ledInversion ? IOLo(ledPin) : IOHi(ledPin);
    }
}

void rxSpiLedOff(void)
{
    if (ledPin) {
        ledInversion ? IOHi(ledPin) : IOLo(ledPin);
    }
}

void rxSpiLedToggle(void)
{
    if (ledPin) {
        IOToggle(ledPin);
    }
}

void rxSpiLedBlink(timeMs_t blinkMs)
{
    static timeMs_t ledBlinkMs = 0;

    if ((ledBlinkMs + blinkMs) > millis()) {
        return;
    }
    ledBlinkMs = millis();

    rxSpiLedToggle();
}

void rxSpiLedBlinkRxLoss(rx_spi_received_e result)
{
    static uint16_t rxLossCount = 0;

    if (ledPin) {
        if (result == RX_SPI_RECEIVED_DATA) {
            rxLossCount = 0;
            rxSpiLedOn();
        } else {
            if (rxLossCount  < RX_LOSS_COUNT) {
                rxLossCount++;
            } else {
                rxSpiLedBlink(INTERVAL_RX_LOSS_MS);
            }
        }
    }
}

void rxSpiLedBlinkBind(void)
{
    rxSpiLedBlink(INTERVAL_RX_BIND_MS);
}

void rxSpiBind(void)
{
    bindRequested = true;
}

bool rxSpiCheckBindRequested(bool reset)
{
    if (bindPin) {
        bool bindPinStatus = IORead(bindPin);
        if (lastBindPinStatus && !bindPinStatus) {
            bindRequested = true;
        }
        lastBindPinStatus = bindPinStatus;
    }

    if (!bindRequested) {
        return false;
    } else {
        if (reset) {
            bindRequested = false;
        }

        return true;
    }
}
#endif