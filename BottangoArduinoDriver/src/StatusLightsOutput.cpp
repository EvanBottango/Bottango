#include "../BottangoArduinoModules.h"
#ifdef ENABLE_STATUS_LIGHTS

#include "StatusLightsOutput.h"
#include "../BottangoArduinoConfig.h"

// Status lights are a fixed, four-pixel WS2812B chain.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <esp32-hal-rmt.h>
#else
#include <FastLED.h>
#endif

namespace StatusLightsOutput
{
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    static const uint32_t RMT_FREQUENCY_HZ = 40000000UL;
    static const uint16_t WS2812_T0H_TICKS = 10;
    static const uint16_t WS2812_T0L_TICKS = 40;
    static const uint16_t WS2812_T1H_TICKS = 35;
    static const uint16_t WS2812_T1L_TICKS = 15;

    static const int BITS_PER_COLOR = 8;
    static const int COLORS_PER_LED = 3;
    static const int OUTPUT_SYMBOL_COUNT = NUM_STATUS_LED * COLORS_PER_LED * BITS_PER_COLOR;
    static rmt_data_t outputSymbols[OUTPUT_SYMBOL_COUNT];

    static const unsigned long WS2812_RESET_TIME_US = 50UL;
    static const unsigned long WS2812_FRAME_TIME_US = (unsigned long)(((uint64_t)OUTPUT_SYMBOL_COUNT * (WS2812_T0H_TICKS + WS2812_T0L_TICKS) * 1000000ULL + RMT_FREQUENCY_HZ - 1) /
                                                                      RMT_FREQUENCY_HZ);
    static bool outputBusy = false;
    static unsigned long nextOutputTimeUs = 0;

    static rmt_data_t getWs2812Symbol(bool isOne)
    {
        rmt_data_t symbol;
        symbol.duration0 = isOne ? WS2812_T1H_TICKS : WS2812_T0H_TICKS;
        symbol.level0 = HIGH;
        symbol.duration1 = isOne ? WS2812_T1L_TICKS : WS2812_T0L_TICKS;
        symbol.level1 = LOW;
        return symbol;
    }

    static void appendColorSymbols(byte color, int &symbolIndex)
    {
        for (int bit = BITS_PER_COLOR - 1; bit >= 0; --bit)
        {
            outputSymbols[symbolIndex++] = getWs2812Symbol((color & (1 << bit)) != 0);
        }
    }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    static bool outputInitialized = false;

    static bool initRmtOutput()
    {
        outputInitialized = rmtInit(STATUS_LED_PIN, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_2, RMT_FREQUENCY_HZ);
        if (outputInitialized)
        {
            rmtSetEOT(STATUS_LED_PIN, LOW);
        }
        return outputInitialized;
    }

    static bool writeRmtSymbols()
    {
        return outputInitialized && rmtWriteAsync(STATUS_LED_PIN, outputSymbols, OUTPUT_SYMBOL_COUNT);
    }
#else
    static rmt_obj_t *output = nullptr;

    static bool initRmtOutput()
    {
        output = rmtInit(STATUS_LED_PIN, RMT_TX_MODE, RMT_MEM_128);
        if (output != nullptr)
        {
            rmtSetTick(output, 25.0f);
        }
        return output != nullptr;
    }

    static bool writeRmtSymbols()
    {
        return output != nullptr && rmtWrite(output, outputSymbols, OUTPUT_SYMBOL_COUNT);
    }
#endif

    static bool outputReadyForNextFrame()
    {
        if (!outputBusy)
        {
            return true;
        }

        if ((long)(micros() - nextOutputTimeUs) < 0)
        {
            return false;
        }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        if (!rmtTransmitCompleted(STATUS_LED_PIN))
        {
            return false;
        }
#endif

        outputBusy = false;
        return true;
    }

    void init()
    {
        outputBusy = false;
        initRmtOutput();

        Color offColors[NUM_STATUS_LED] = {};
        write(offColors, NUM_STATUS_LED);
    }

    bool write(const Color *colors, int colorCount)
    {
        if (colors == nullptr || colorCount != NUM_STATUS_LED)
        {
            return false;
        }

        if (!outputReadyForNextFrame())
        {
            return false;
        }

        int symbolIndex = 0;
        for (int ledIndex = 0; ledIndex < colorCount; ++ledIndex)
        {
            appendColorSymbols(colors[ledIndex].g, symbolIndex);
            appendColorSymbols(colors[ledIndex].r, symbolIndex);
            appendColorSymbols(colors[ledIndex].b, symbolIndex);
        }

        if (!writeRmtSymbols())
        {
            return false;
        }

        outputBusy = true;
        nextOutputTimeUs = micros() + WS2812_FRAME_TIME_US + WS2812_RESET_TIME_US;
        return true;
    }

#else

    static CRGB outputColors[NUM_STATUS_LED];

    void init()
    {
        FastLED.addLeds<WS2812B, STATUS_LED_PIN, GRB>(outputColors, NUM_STATUS_LED);
        FastLED.clear();
        FastLED.show();
    }

    bool write(const Color *colors, int colorCount)
    {
        if (colors == nullptr || colorCount != NUM_STATUS_LED)
        {
            return false;
        }

        for (int ledIndex = 0; ledIndex < colorCount; ++ledIndex)
        {
            outputColors[ledIndex] = CRGB(colors[ledIndex].r, colors[ledIndex].g, colors[ledIndex].b);
        }
        FastLED.show();
        return true;
    }

#endif // ESP32 output
} // namespace StatusLightsOutput

#endif // ENABLE_STATUS_LIGHTS
