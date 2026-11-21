// STEP 7 : Padding - a concrete embedded example
//
// Scenario: a weather station stores 1000 sensor samples in RAM.
// Each sample has:
//   - a timestamp      (uint32_t, 4 bytes)
//   - a temperature    (int16_t,  2 bytes)  in 0.01 °C
//   - a humidity       (uint8_t,  1 byte)   in %
//   - a sensor id      (uint8_t,  1 byte)
//   - a "valid" flag   (bool,     1 byte)
//   - a pressure       (uint32_t, 4 bytes)  in Pa
// Useful data = 4 + 2 + 1 + 1 + 1 + 4 = 13 bytes

#include <cstddef>
#include <cstdint>
#include <iostream>

// ❌ BAD ORDER: written "in the order I thought of the fields"
struct SampleBad {
    bool     valid;        // 1
    uint32_t timestamp;    // 4  -> needs offset multiple of 4
    uint8_t  humidity;     // 1
    int16_t  temperature;  // 2  -> needs offset multiple of 2
    uint8_t  sensorId;     // 1
    uint32_t pressure;     // 4  -> needs offset multiple of 4
};

// ✅ GOOD ORDER: largest -> smallest
struct SampleGood {
    uint32_t timestamp;    // 4
    uint32_t pressure;     // 4
    int16_t  temperature;  // 2
    uint8_t  humidity;     // 1
    uint8_t  sensorId;     // 1
    bool     valid;        // 1
};

#define SHOW(T, m) std::cout << "  " #m " @ offset " << offsetof(T, m) << "\n"

int main() {
    const int N = 1000;

    std::cout << "SampleBad  : " << sizeof(SampleBad) << " bytes\n";
    SHOW(SampleBad, valid);
    SHOW(SampleBad, timestamp);
    SHOW(SampleBad, humidity);
    SHOW(SampleBad, temperature);
    SHOW(SampleBad, sensorId);
    SHOW(SampleBad, pressure);

    std::cout << "\nSampleGood : " << sizeof(SampleGood) << " bytes\n";
    SHOW(SampleGood, timestamp);
    SHOW(SampleGood, pressure);
    SHOW(SampleGood, temperature);
    SHOW(SampleGood, humidity);
    SHOW(SampleGood, sensorId);
    SHOW(SampleGood, valid);

    std::cout << "\nFor " << N << " samples:\n";
    std::cout << "  bad  order : " << sizeof(SampleBad)  * N << " bytes of RAM\n";
    std::cout << "  good order : " << sizeof(SampleGood) * N << " bytes of RAM\n";
    std::cout << "  saved      : " << (sizeof(SampleBad) - sizeof(SampleGood)) * N << " bytes\n";
    return 0;
}
