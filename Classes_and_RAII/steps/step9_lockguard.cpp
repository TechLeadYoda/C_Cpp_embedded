// STEP 9 : LockGuard - disable / re-enable interrupts with RAII
//
// Compile (PC):  g++ -std=c++17 -Wall -Wextra step9_lockguard.cpp -o step9 && ./step9
//
// THE PROBLEM
// -----------
// A variable shared between main() and an interrupt (ISR):
//
//     volatile uint32_t ticks;        // incremented by a timer ISR
//
// If main() reads/modifies it while the ISR fires in the middle,
// the value can be corrupted. The classic fix: disable interrupts
// during the access (a "critical section"), then re-enable them.
//
// THE C WAY (and its bug)
// -----------------------
//     __disable_irq();
//     if (error) return;     // ❌ BUG: interrupts are NEVER re-enabled!
//     ticks = 0;
//     __enable_irq();
//
// THE RAII WAY
// ------------
//     {
//         LockGuard lock;    // constructor -> disables interrupts
//         if (error) return; // ✅ destructor runs -> re-enabled
//         ticks = 0;
//     }                      // ✅ destructor runs -> re-enabled

#include <cstdint>
#include <cstdio>

// ---------------------------------------------------------------------------
// 1. Low-level CPU access
//    On a real Cortex-M we use CMSIS. On the PC we simulate it with prints,
//    so the example can run here.
//
//    PRIMASK is a Cortex-M register:
//       0 = interrupts ENABLED
//       1 = interrupts DISABLED
// ---------------------------------------------------------------------------
namespace cpu {

#if defined(__ARM_ARCH)          // ---- real microcontroller ----
// #include "stm32f4xx.h"        // your chip's CMSIS header
inline uint32_t getPrimask()           { return __get_PRIMASK(); }
inline void     setPrimask(uint32_t v) { __set_PRIMASK(v); }
inline void     disableIrq()           { __disable_irq(); }

#else                            // ---- PC simulation ----
inline uint32_t primask = 0;     // fake register: starts with IRQs enabled

inline uint32_t getPrimask()           { return primask; }
inline void     setPrimask(uint32_t v) {
    primask = v;
    std::printf("    [cpu] PRIMASK = %u -> IRQs %s\n", v, v ? "OFF" : "ON");
}
inline void     disableIrq() {
    primask = 1;
    std::printf("    [cpu] disable_irq  -> IRQs OFF\n");
}
#endif

} // namespace cpu

// ---------------------------------------------------------------------------
// 2. The LockGuard class
// ---------------------------------------------------------------------------
//
// ABOUT noexcept
// --------------
// noexcept = a promise that the function NEVER throws an exception.
// If one escapes anyway -> std::terminate() (program stops at once).
//
// WHEN TO USE IT
//   - destructors (already noexcept by default; never throw from them)
//   - move constructor / move assignment (std::vector only moves
//     your objects when growing if the move is noexcept)
//   - swap functions
//   - simple functions that cannot fail: getters, register read/write,
//     small calculations (like this LockGuard)
//   - code called from an ISR or a destructor
//   Do NOT use it on a function that can fail, or if you are not sure.
//
// WHICH FUNCTIONS CAN THROW
//   1. Memory allocation: new, vector::push_back/resize/reserve,
//      std::string (+, append...), map::insert, make_unique/make_shared
//      -> std::bad_alloc when the heap is full
//   2. Checked access / parsing: vec.at(i) (std::out_of_range),
//      std::stoi("abc") (std::invalid_argument), optional::value()
//   3. An explicit throw:  if (pin > 15) throw std::invalid_argument("bad pin");
//   4. Unknown code: callbacks, virtual functions, template parameters
//
// WHICH CANNOT THROW
//   - reading/writing variables or hardware registers
//   - arithmetic (divide by 0 is undefined behavior, NOT an exception)
//   - pointer access, vec[i] (no bounds check)
//   - C functions (printf, memcpy...): C has no exceptions
//
// HERE: getPrimask / setPrimask / disableIrq only touch a variable and
// call printf -> they cannot throw -> noexcept on LockGuard is correct.
//
// EMBEDDED NOTE: firmware often avoids the heap (point 1) and builds with
// -fno-exceptions -> nothing can throw; errors are return values, like in C.
// noexcept is then mostly documentation, but still a good habit.
//
class LockGuard {
public:
    // Constructor: SAVE the current state, THEN disable interrupts
    LockGuard() noexcept
        : saved_{cpu::getPrimask()}
    {
        cpu::disableIrq();
    }

    // Destructor: RESTORE the saved state
    // (not "always enable": see the nested example below)
    ~LockGuard() noexcept
    {
        cpu::setPrimask(saved_);
    }

    // A LockGuard must never be copied: two destructors
    // would restore the state twice.
    LockGuard(const LockGuard&)            = delete;
    LockGuard& operator=(const LockGuard&) = delete;

private:
    const uint32_t saved_;   // PRIMASK value before we disabled interrupts
};

// ---------------------------------------------------------------------------
// 3. Examples
// ---------------------------------------------------------------------------
volatile uint32_t ticks = 1234;   // shared with a (pretend) timer ISR

// Normal use
uint32_t readAndResetTicks() {
    LockGuard lock;               // IRQs OFF from here...
    uint32_t value = ticks;
    ticks = 0;
    return value;
}                                 // ...to here: IRQs restored automatically

// Early return: the destructor still runs
bool resetTicksIf(bool allowed) {
    LockGuard lock;
    if (!allowed) {
        std::printf("    not allowed -> early return\n");
        return false;             // ✅ ~LockGuard() runs here too
    }
    ticks = 0;
    return true;
}

// Nested: a function that locks, called from code that ALREADY locked
void helper() {
    LockGuard inner;              // saves PRIMASK = 1 (already OFF)
    std::printf("    helper works with IRQs OFF\n");
}                                 // restores 1 -> IRQs stay OFF ✅

void outer() {
    LockGuard outerLock;          // saves PRIMASK = 0 (ON)
    helper();
    std::printf("    back in outer: IRQs must still be OFF -> primask=%u\n",
                cpu::getPrimask());
}                                 // restores 0 -> IRQs ON

int main() {
    std::printf("== 1. normal use\n");
    uint32_t t = readAndResetTicks();
    std::printf("    read ticks = %u\n", t);

    std::printf("\n== 2. early return\n");
    resetTicksIf(false);

    std::printf("\n== 3. nested locks\n");
    outer();

    std::printf("\n== end: IRQs are %s\n", cpu::getPrimask() ? "OFF" : "ON");
    return 0;
}
