// STEP 6 : The recommended way to write a constructor

#include <cstdint>    // for uint8_t
#include <iostream>

// 1. The state type MUST be declared BEFORE the class that uses it
enum class LedState { Off, On };

class Led {
public:
    //                    parameter 1     parameter 2
    //                    vvvvvvvvvvv     vvvvvvvvvvvvvvvv
    explicit Led(uint8_t pin, LedState initial)
        : pin_{pin}, isOn_{initial == LedState::On}
    //               ^^^^^^^ "initial" is the parameter declared just above.
    //                       It only exists inside this constructor.
    {
    }

    ~Led() { std::cout << "Led " << +pin_ << " destroyed\n"; }

    void toggle()     { isOn_ = !isOn_; }
    bool isOn() const { return isOn_; }

private:
    const uint8_t pin_;   // const: a LED never changes pin
    bool isOn_;           // initialized once, in the initializer list
};

int main() {
    Led red{2, LedState::Off};
    Led green{7, LedState::On};

    red.toggle();
    std::cout << "red:   " << red.isOn()   << "\n";
    std::cout << "green: " << green.isOn() << "\n";
    return 0;
}
