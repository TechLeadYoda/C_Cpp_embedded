// runner.cpp — LED class reviewed with the "what does it assume?" method
//
// Format:
//   // OLD     : old line
//   // assumed : what the old line assumed (and was false)
//   new line   // assumes: what the corrected line assumes
//
// Compile and run:
//   g++ -std=c++17 -Wall -Wextra runner.cpp -o runner && ./runner

#include <cassert>
#include <cstdint>
// OLD     : #include <iostream>
// assumed : its Flash size does not matter on an MCU
#include <cstdio>   // assumes: printf is light and has an output (UART retarget on MCU)


// OLD     : enum State{ isOn, isOff };
// assumed : names never clash, never used as an int, size does not matter
enum class State : uint8_t { isOff, isOn };   // assumes: names scoped, no int conversion, 1 byte


enum class Port : uint8_t { A, B, C, D, E };   // assumes: ports A..E exist on this chip


struct Pin {
    Port    port;
    // OLD     : uint8_t const pin_n;
    // assumed : const here protects the pin (it only blocks assigning a Pin)
    uint8_t pin_n;   // assumes: the user of Pin (LED) checks 0..15
};


// Fake hardware (PC). On the STM32 these write MODER / BSRR.
void gpioInitOutput(Pin p) {
    std::printf("  [gpio] P%c%u -> output\n", 'A' + static_cast<int>(p.port), p.pin_n);
}
void gpioWrite(Pin p, bool high) {
    std::printf("  [gpio] P%c%u = %s\n", 'A' + static_cast<int>(p.port), p.pin_n,
                high ? "HIGH (on)" : "LOW  (off)");
}


class LED {
public:
    // OLD     : explicit LED(Pin pin, State state = State::isOff) ...
    // assumed : the caller knows the hidden default (Off)
    explicit LED(Pin pin, State state) : pin_{pin}, pin_state{state} {   // assumes: the caller always writes the initial state
        assert(pin.pin_n < 16);   // assumes: the pin number is valid (checked in debug builds)

        // OLD     : //configure the pins
        // assumed : the hardware is configured (nothing was done)
        gpioInitOutput(pin_);     // assumes: the pin becomes an output
        apply();                  // assumes: the real pin shows the initial state
    }

    // OLD     : ~LED(){ std::cout<< " LED has been destroyed\n";}
    // assumed : printing is enough (the pin stayed in its last state)
    ~LED() {                      // assumes: the pin is left off when the LED disappears
        pin_state = State::isOff;
        apply();
        std::printf("  LED destroyed (pin left off)\n");
    }

    // OLD     : (nothing)
    // assumed : copying an LED makes sense (2 objects, 1 pin)
    LED(const LED&)            = delete;   // assumes: one LED object owns one pin
    LED& operator=(const LED&) = delete;

    void toggle_led() {
        if (pin_state == State::isOn) {
            pin_state = State::isOff;
        } else {
            pin_state = State::isOn;
        }
        // OLD     : (nothing)
        // assumed : changing pin_state changes the LED (only the variable changed)
        apply();                  // assumes: the real pin follows the variable
    }

    [[nodiscard]] bool is_on() const { return pin_state == State::isOn; }   // assumes: the variable matches the real pin

private:
    void apply() { gpioWrite(pin_, pin_state == State::isOn); }   // assumes: the only place that writes the pin

    // OLD     : Pin pin_;
    // assumed : nobody will change the pin later
    const Pin pin_;      // assumes: the pin never changes (compiler enforces it)
    State pin_state;     // assumes: always set by the constructor
};


int main() {
    // OLD     : LED led(3);
    // assumed : "3" is enough (PA3? PB3?), initial state is obvious, (3.7) is refused
    LED led{Pin{Port::A, 3}, State::isOff};   // assumes: port + number + initial state are explicit

    led.toggle_led();
    std::printf("  led is %s\n", led.is_on() ? "ON" : "OFF");
    led.toggle_led();
    std::printf("  led is %s\n", led.is_on() ? "ON" : "OFF");

    return 0;   // assumes: running on the PC (on an MCU, main never returns)
}
