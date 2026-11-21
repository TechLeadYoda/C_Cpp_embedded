// STEP 5 : Destruction order
// Objects are destroyed in the REVERSE order of their creation
// (like a stack of plates: the last one placed is the first one removed).

#include <iostream>

class Led {
public:
    Led(int pin) : pin_(pin) { std::cout << "create  " << pin_ << "\n"; }
    ~Led()                   { std::cout << "destroy " << pin_ << "\n"; }
private:
    const int pin_;
};

int main() {
    Led a(1);
    Led b(2);
    Led c(3);
    std::cout << "--- end of main ---\n";
    return 0;
}   // destroyed in this order: 3, then 2, then 1
