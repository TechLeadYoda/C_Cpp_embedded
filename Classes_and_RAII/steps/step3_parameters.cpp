// STEP 3 : Constructor with parameters + member variables
// The constructor can take values used to set up the object.
// This version sets the members INSIDE the body { }.
// It works, but it is not the best way (see step 4).

#include <iostream>
/*
class Led {
public:
    Led(int pin) {
        pin_ = pin;               // assignment inside the body
        std::cout << "Led on pin " << pin_ << " created\n";
    }
    ~Led() { std::cout << "Led on pin " << pin_ << " destroyed\n"; }

private:
    int pin_;                     // member variable (each Led has its own)
};*/

class Led{
    public:
        Led(int pin):pin_{pin}{
            std::cout << "Led on pin " << pin_ << " created\n";
        }
    private:
        int pin_;

};

int main() {
    Led red(2);                   // pin_ = 2
    Led green(7);                 // pin_ = 7
    return 0;
}
