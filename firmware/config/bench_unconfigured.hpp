#pragma once
#include "robot/safety.hpp"
namespace robot::config {
// NOT a sample calibration to copy to real hardware. All IDs and limits are unset.
inline SafetyConfig bench_unconfigured() noexcept { return {}; }
}
