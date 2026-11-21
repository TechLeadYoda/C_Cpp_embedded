// STEP 10 : Blink a LED on PA5 - the whole design, start to finish
//
// Need:  "I want to blink a LED connected to pin PA5 every 500 ms."
//
// PC   :  g++ -std=c++17 -Wall -Wextra step10_blink_pa5.cpp -o step10 && ./step10
// STM32:  build in your IDE (STM32F4 + CMSIS). PA5 = user LED on Nucleo boards.

#include <cstdint>

// ===========================================================================
// 1. HARDWARE LAYER: the only part that knows about registers
// ===========================================================================
struct Pin {
    char    port;     // 'A', 'B', 'C'...
    uint8_t number;   // 0..15
};

#if defined(__ARM_ARCH)   // ------------------------- real STM32F4 -----------
#include "stm32f4xx.h"

static GPIO_TypeDef* portBase(char port) {
    return reinterpret_cast<GPIO_TypeDef*>(GPIOA_BASE + (port - 'A') * 0x400u);
}

void gpioInitOutput(Pin p) {
    RCC->AHB1ENR |= (1u << (p.port - 'A'));           // turn on the port's clock
    GPIO_TypeDef* g = portBase(p.port);
    g->MODER &= ~(3u << (p.number * 2));              // clear the 2 mode bits
    g->MODER |=  (1u << (p.number * 2));              // 01 = output
}

void gpioWrite(Pin p, bool high) {
    // BSRR: low 16 bits SET a pin, high 16 bits RESET it (atomic write)
    portBase(p.port)->BSRR = high ? (1u << p.number) : (1u << (p.number + 16));
}

void delayMs(uint32_t ms) {                           // rough busy-wait (16 MHz)
    for (volatile uint32_t i = 0; i < ms * 4000u; ++i) {}
}

#else                     // ------------------------- PC simulation ----------
#include <cstdio>

void gpioInitOutput(Pin p)      { std::printf("[gpio] P%c%u -> output\n", p.port, p.number); }
void gpioWrite(Pin p, bool high){ std::printf("[gpio] P%c%u = %s\n", p.port, p.number, high ? "HIGH (LED on)" : "LOW  (LED off)"); }
void delayMs(uint32_t ms)       { std::printf("       ... wait %u ms\n", static_cast<unsigned>(ms)); }
#endif

// ===========================================================================
// 2. THE Led CLASS: comes straight from the word analysis
//      noun  "LED"   -> class Led
//      noun  "pin"   -> member pin_
//      state on/off  -> member isOn_
//      verbs         -> on(), off(), toggle()
// ===========================================================================
class Led {
public:
    explicit Led(Pin pin)            // the pin is the only thing a Led needs
        : pin_{pin}, isOn_{false}
    {
        gpioInitOutput(pin_);        // problem found: forgot to configure the pin
        gpioWrite(pin_, false);      // problem found: start in a KNOWN state (off)
    }

    ~Led() { gpioWrite(pin_, false); }   // leave the pin safe (off)

    Led(const Led&)            = delete; // one physical LED = no copies
    Led& operator=(const Led&) = delete;

    void on()     { isOn_ = true;   gpioWrite(pin_, true);  }
    void off()    { isOn_ = false;  gpioWrite(pin_, false); }
    void toggle() { isOn_ ? off() : on(); }

private:
    const Pin pin_;    // never changes
    bool      isOn_;   // only changed by on()/off()
};

// ===========================================================================
// 3. THE APPLICATION: "blink every 500 ms"
//    blink = toggle + wait, repeated.
//    Nothing to remember between turns -> a simple loop is enough.
// ===========================================================================
int main() {
    Led led{Pin{'A', 5}};            // the LED on PA5

#if defined(__ARM_ARCH)
    while (true) {                   // on the board: forever
#else
    for (int i = 0; i < 4; ++i) {    // on the PC: only 4 turns, to see the output
#endif
        led.toggle();
        delayMs(500);
    }
}
