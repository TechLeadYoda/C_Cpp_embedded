// STEP 4 : The initializer list
// Syntax:   Led(int pin) : pin_(pin) { ... }
//                       ^^^^^^^^^^^
// The part after ':' sets each member BEFORE the body { } runs.
//
// Why use it?
//   - The member is initialized directly (no "empty value first, then assign").
//   - It is REQUIRED for const members: they cannot be assigned later.

#include <iostream>

class Led {
public:
    Led(int pin) : pin_(pin), isOn_(false)    // <-- initializer list
    {
        std::cout << "Led on pin " << pin_ << " created\n";
    }
    ~Led() { std::cout << "Led on pin " << pin_ << " destroyed\n"; }

    void toggle() {
        isOn_ = !isOn_;
        std::cout << "Pin " << pin_ << (isOn_ ? " ON\n" : " OFF\n");
    }

private:
    const int pin_;   // const: a LED never changes pin.
                      // Try writing "pin_ = pin;" in the body: it will NOT compile!
    bool isOn_;
};

int main() {
    Led red(2);
    red.toggle();
    red.toggle();
    return 0;
}
