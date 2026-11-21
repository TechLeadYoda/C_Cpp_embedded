// STEP 1 : The constructor
// A constructor is a special function that runs AUTOMATICALLY
// when an object is created.
//   - same name as the class
//   - no return type (not even void)

#include <iostream>

class Led {
public:
    Led() {                       // <-- the constructor
        std::cout << "Led created!\n";
    }
};

int main() {
    std::cout << "Before\n";
    Led myLed;                    // creating the object calls Led()
    std::cout << "After\n";
    return 0;
}

