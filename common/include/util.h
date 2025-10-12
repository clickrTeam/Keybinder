
// helper type for the std::visit
// https://en.cppreference.com/w/cpp/utility/variant/visit2.html
#include <cstdint>
template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

uint64_t current_time_ms();
