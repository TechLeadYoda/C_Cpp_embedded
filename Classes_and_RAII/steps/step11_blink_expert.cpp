// STEP 11 : Blink PA5 - from pseudo-code to expert code
//
// Need: "I want to blink a LED connected to pin PA5 every 500 ms,
//        without blocking the rest of the program."
//
// FINAL PSEUDO-CODE (after applying the lenses)
// ---------------------------------------------
//   START
//       start a 1 ms clock
//       prepare the LED on PA5              (output, OFF)
//       prepare a blinker: LED, 500 ms
//       REPEAT FOREVER
//           give the blinker the current time
//               IF 500 ms passed since the last toggle
//                   toggle the LED
//                   last toggle += 500 ms
//           do other work (buttons, UART...)
//   END
//
// PC   :  g++ -std=c++17 -Wall -Wextra -Werror step11_blink_expert.cpp -o step11 && ./step11
// STM32:  build in your IDE (STM32F4 + CMSIS). PA5 = user LED on Nucleo boards.

#include <cstdint>

// ===========================================================================
// 1. HARDWARE LAYER  (lens 4: what changes is kept apart)
// ===========================================================================
namespace hw {

enum class Port : uint8_t { A, B, C, D, E };   // lens 2: no 'Z' port possible

struct Pin {
    Port    port;
    uint8_t number;   // 0..15
};

} // namespace hw

#if defined(__ARM_ARCH)   // ========================= real STM32F4 ==========
#include "stm32f4xx.h"

namespace hw {

volatile uint32_t g_ms = 0;   // written ONLY by SysTick_Handler.
                              // lens 5: a 32-bit read is atomic on Cortex-M,
                              // so main() can read it without a LockGuard.

struct Stm32Gpio {
    static GPIO_TypeDef* base(Port p) noexcept {
        return reinterpret_cast<GPIO_TypeDef*>(
            GPIOA_BASE + static_cast<uint32_t>(p) * 0x400u);
    }
    static void initOutput(Pin p) noexcept {
        RCC->AHB1ENR |= (1u << static_cast<uint32_t>(p.port));  // port clock ON
        GPIO_TypeDef* g = base(p.port);
        g->MODER &= ~(3u << (p.number * 2));
        g->MODER |=  (1u << (p.number * 2));                     // 01 = output
    }
    static void write(Pin p, bool high) noexcept {
        base(p.port)->BSRR = high ? (1u << p.number)             // atomic set
                                  : (1u << (p.number + 16));     // atomic reset
    }
};

struct Stm32Clock {
    static void init() noexcept { SysTick_Config(SystemCoreClock / 1000u); } // 1 ms
    static uint32_t now() noexcept { return g_ms; }
};

using Gpio  = Stm32Gpio;
using Clock = Stm32Clock;

} // namespace hw

extern "C" void SysTick_Handler() { hw::g_ms = hw::g_ms + 1u; }

#else                     // ========================= PC simulation =========
#include <cstdio>

namespace hw {

struct FakeClock {
    static inline uint32_t ms = 0;                 // simulated time
    static void     init() noexcept {}
    static uint32_t now()  noexcept { return ms; }
    static void     advance(uint32_t d) noexcept { ms += d; }
};

struct FakeGpio {
    static char name(Port p) noexcept { return static_cast<char>('A' + static_cast<int>(p)); }
    static void initOutput(Pin p) noexcept {
        std::printf("t=%10u  [gpio] P%c%u -> output\n",
                    static_cast<unsigned>(FakeClock::ms), name(p.port), p.number);
    }
    static void write(Pin p, bool high) noexcept {
        std::printf("t=%10u  [gpio] P%c%u = %s\n",
                    static_cast<unsigned>(FakeClock::ms), name(p.port), p.number,
                    high ? "HIGH (on)" : "LOW  (off)");
    }
};

using Gpio  = FakeGpio;
using Clock = FakeClock;

} // namespace hw
#endif

// ===========================================================================
// 2. Led  (lenses 1, 2, 5)
// ===========================================================================
template <typename Gpio>
class Led {
public:
    explicit Led(hw::Pin pin) noexcept
        : pin_{pin}, isOn_{false}
    {
        Gpio::initOutput(pin_);          // lens 1: configured from birth
        Gpio::write(pin_, false);        // lens 1: known state
    }
    ~Led() noexcept { Gpio::write(pin_, false); }

    Led(const Led&)            = delete; // lens 2: one physical LED
    Led& operator=(const Led&) = delete;

    void on()     noexcept { isOn_ = true;  Gpio::write(pin_, true);  }
    void off()    noexcept { isOn_ = false; Gpio::write(pin_, false); }
    void toggle() noexcept { isOn_ ? off() : on(); }

    [[nodiscard]] bool isOn() const noexcept { return isOn_; }

private:
    const hw::Pin pin_;    // 2 bytes
    bool          isOn_;   // 1 byte
};

static_assert(sizeof(Led<hw::Gpio>) <= 4, "Led must stay tiny");  // lens 5

// ===========================================================================
// 3. Blinker  (lens 3: non-blocking timing)
//    It must REMEMBER the last toggle time -> it deserves its own class.
// ===========================================================================
template <typename Gpio>
class Blinker {
public:
    Blinker(Led<Gpio>& led, uint32_t periodMs, uint32_t nowMs) noexcept
        : led_{led}, periodMs_{periodMs}, lastMs_{nowMs}
    {
    }

    Blinker(const Blinker&)            = delete;  // two blinkers on one LED = chaos
    Blinker& operator=(const Blinker&) = delete;

    // Call as often as possible. Returns immediately: never blocks.
    void update(uint32_t nowMs) noexcept {
        // Unsigned subtraction stays correct when the ms counter wraps
        // around (after 49.7 days at 1 ms).
        if (nowMs - lastMs_ >= periodMs_) {
            led_.toggle();
            lastMs_ += periodMs_;   // += period (not = now): no slow drift
        }
    }

private:
    Led<Gpio>&     led_;
    const uint32_t periodMs_;
    uint32_t       lastMs_;
};

// ===========================================================================
// 4. APPLICATION
// ===========================================================================
int main() {
    using namespace hw;

#if !defined(__ARM_ARCH)
    // PC test: start just before the 32-bit counter wraps, to prove
    // the timing still works across the wrap-around.
    Clock::ms = 0xFFFFFFFFu - 1199u;
#endif

    Clock::init();
    Led<Gpio>     led{Pin{Port::A, 5}};
    Blinker<Gpio> blinker{led, 500, Clock::now()};

#if defined(__ARM_ARCH)
    while (true) {
        blinker.update(Clock::now());
        // ... other work: buttons, UART, sensors ... (never blocked)
    }
#else
    for (int i = 0; i < 25; ++i) {      // simulate 2.5 s of main loop
        blinker.update(Clock::now());
        Clock::advance(100);            // each loop turn "takes" 100 ms
    }
#endif
}
