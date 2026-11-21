// STEP 2 : The destructor
// A destructor runs AUTOMATICALLY when the object is destroyed,
// that is, when it reaches the closing brace } of its block.
//   - name = ~ + class name
//   - no parameters, no return type

#include <iostream>

class Led {
public:
    Led()  { std::cout << "Led created\n"; }
    Led() { std::cout << "Led destroyed\n"; }   // <-- the destructor
};

int main() {
    std::cout << "Start of main\n";
    {                               // a new block opens
        Led myLed;                  // constructor runs here
        std::cout << "Inside the block\n";
    }                               // destructor runs HERE, automatically
    std::cout << "End of main\n";
    return 0;
}
