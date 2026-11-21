// STEP 8 : How an experienced embedded C++ developer would write Led
//
// Compile:  g++ -std=c++17 -Wall -Wextra step8_expert_led.cpp -o step8 && ./step8

#include <cstdint>
#include <cstdio>

// ---------------------------------------------------------------------------
// (1) Small, strong types instead of raw int / bool
// ---------------------------------------------------------------------------
enum class LedState : uint8_t { Off, On };              // : uint8_t -> 1 byte
enum class Polarity : uint8_t { ActiveHigh, ActiveLow }; // how the LED is wired

struct Pin {            // a pin = a port (A, B, C...) + a number (0..15)
    uint8_t port;
    uint8_t number;
};

// ---------------------------------------------------------------------------
// (2) The hardware access is SEPARATED from the Led logic.
//     Here: a fake driver that prints, so the code runs on a PC.
//     On a real STM32 you would write a struct with the SAME functions
//     that writes to the GPIO registers (or calls the HAL).
// ---------------------------------------------------------------------------
struct FakeGpio {
    static void configureOutput(Pin p) {
        std::printf("  [gpio] P%c%u -> output\n", 'A' + p.port, p.number);
    }
    static void write(Pin p, bool high) {
        std::printf("  [gpio] P%c%u = %s\n", 'A' + p.port, p.number, high ? "HIGH" : "LOW");
    }
};

// ---------------------------------------------------------------------------
// (3) The Led class
// ---------------------------------------------------------------------------
template <typename Gpio>            // the driver is chosen at compile time
class Led {
public:
    // RAII: the constructor ACQUIRES the pin and puts it in a known state
    Led(Pin pin, LedState initial, Polarity polarity) noexcept
        : pin_{pin}, polarity_{polarity}, state_{initial}
    {
        Gpio::configureOutput(pin_);
        apply();
    }

    // RAII: the destructor RELEASES the pin in a SAFE state (off)
    ~Led() noexcept { off(); }

    // (4) A Led owns a physical pin: it must NOT be copied
    Led(const Led&)            = delete;
    Led& operator=(const Led&) = delete;

    // (5) Small, clear public interface
    void on()     noexcept { set(LedState::On); }
    void off()    noexcept { set(LedState::Off); }
    void toggle() noexcept { set(isOn() ? LedState::Off : LedState::On); }

    void set(LedState s) noexcept {
        state_ = s;
        apply();
    }

    [[nodiscard]] bool     isOn()  const noexcept { return state_ == LedState::On; }
    [[nodiscard]] LedState state() const noexcept { return state_; }

private:
    // Translate the LOGICAL state (On/Off) into the ELECTRICAL level (HIGH/LOW)
    void apply() noexcept {
        const bool logicalOn = (state_ == LedState::On);
        const bool high = (polarity_ == Polarity::ActiveHigh) ? logicalOn : !logicalOn;
        Gpio::write(pin_, high);
    }

    // (6) Members ordered and typed so the object is tiny: 2 + 1 + 1 = 4 bytes
    const Pin      pin_;
    const Polarity polarity_;
    LedState       state_;
};

// The class is checked at compile time: no hidden padding, fits in 4 bytes
static_assert(sizeof(Led<FakeGpio>) == 4, "Led should stay 4 bytes");

// A short name for the application code
using BoardLed = Led<FakeGpio>;

// ---------------------------------------------------------------------------
int main() {
    std::printf("== create\n");
    {
        BoardLed status{Pin{0, 5}, LedState::Off, Polarity::ActiveHigh};  // PA5
        BoardLed error {Pin{2, 13}, LedState::Off, Polarity::ActiveLow};  // PC13 (wired to 3.3V)

        std::printf("== use\n");
        status.toggle();
        error.on();
        std::printf("  status is %s\n", status.isOn() ? "ON" : "OFF");

        // BoardLed copy = status;   // ❌ does not compile: copy is deleted

        std::printf("== end of block\n");
    }   // destructors: both LEDs are switched OFF automatically
    std::printf("== done\n");
    return 0;
}
