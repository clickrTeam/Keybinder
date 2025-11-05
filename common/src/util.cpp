#include "util.h"
#include <chrono>
#include <cstdint>

uint64_t current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
        .count();
}
