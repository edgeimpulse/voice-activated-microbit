// WS2812B timings, +-0.15uS
// 0 - 0.40uS hi 0.85uS low
// 1 - 0.80uS hi 0.45uS low

#include "neopixel.h"

#if CONFIG_ENABLED(HARDWARE_NEOPIXEL)

void neopixel_send_buffer(Pin &pin, const uint8_t *ptr, int numBytes)
{
    static NRF52PWM *pwm = NULL;
    static WS2812B *ws = NULL;

    if (pwm == NULL)
    {
        ws = new WS2812B();
        pwm = new NRF52PWM(NRF_PWM2, *ws, WS2812B_PWM_FREQ);
        pwm->setStreamingMode(true, false);
        pwm->setDecoderMode(PWM_DECODER_LOAD_Common);
        pwm->setSampleRate(WS2812B_PWM_FREQ);
    }

    pwm->connectPin(pin, 0);

    ws->play(ptr, numBytes);
}

#else

__attribute__((noinline)) void neopixel_send_buffer(Pin &pin, const uint8_t *ptr, int numBytes)
{
    pin.setDigitalValue(0);

#ifdef NRF_P1
    auto port = pin.name < 32 ? NRF_P0 : NRF_P1;
#else
    auto port = NRF_P0;
#endif
    uint32_t PIN = 1 << (pin.name & 31);

    if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    uint32_t startTime = DWT->CYCCNT;
    // min. 50uS reset time; give it 100uS
    while ((DWT->CYCCNT - startTime) < 64 * 100)
        ;

    uint32_t mask = 0x80;
    int i = 0;

    target_disable_irq();
    uint32_t phase = DWT->CYCCNT;
    for (;;)
    {
        port->OUTSET = PIN;

        // phase = CPU_MHZ / 0.8
        // 52/25 are relative to phase
        uint32_t change = ptr[i] & mask ? phase + 52 : phase + 25;
        phase += 80;

        mask = mask >> 1;
        if (mask == 0)
        {
            mask = 0x80;
            i++;
        }

        while (DWT->CYCCNT < change)
            ;

        port->OUTCLR = PIN;

        if (i >= numBytes)
            break;

        while (DWT->CYCCNT < phase)
            ;
    }
    target_enable_irq();
}
#endif

void neopixel_send_buffer(Pin &pin, ManagedBuffer buffer)
{
    neopixel_send_buffer(pin, &buffer[0], buffer.length());
}
