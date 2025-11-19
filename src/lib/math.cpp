#include "lib/math.hpp"

float floor(const float num) {
    const int i = static_cast<int>(num);
    return (num < static_cast<float>(i)) ? static_cast<float>(i - 1) : static_cast<float>(i);
}