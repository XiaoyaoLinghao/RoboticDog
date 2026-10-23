#include "robot/safety.hpp"
#include <cmath>

namespace robot {
namespace {
bool interval(std::uint32_t value) noexcept { return value > 0 && value < 0x80000000U; }
bool calibration_valid(const JointCalibration& j) noexcept {
  if (!j.verified || j.servo_id >= lx16a::kBroadcastId || !std::isfinite(j.zero_tick) || j.zero_tick < 0 || j.zero_tick > 1000 ||
      (j.direction != 1 && j.direction != -1) || !std::isfinite(j.ticks_per_rad) || j.ticks_per_rad <= 0 ||
      !std::isfinite(j.min_rad) || !std::isfinite(j.max_rad) || j.min_rad >= j.max_rad ||
      j.min_tick >= j.max_tick || j.max_tick > 1000 || !std::isfinite(j.max_speed_rad_s) || j.max_speed_rad_s <= 0 ||
      !std::isfinite(j.max_step_rad) || j.max_step_rad <= 0) return false;
  for (const auto q : {j.min_rad, j.max_rad}) {
    const double tick = j.zero_tick + j.direction * j.ticks_per_rad * q;
    if (!std::isfinite(tick) || tick < j.min_tick || tick > j.max_tick) return false;
  }
  return true;
}
}
bool SafetyGate::configure(const SafetyConfig& c) noexcept {
  if (state_ != SafetyState::Disarmed) return false;
  configured_ = false;
  if (!interval(c.feedback_timeout_ms) || !interval(c.target_timeout_ms) || !interval(c.watchdog_ms) ||
      c.min_move_ms == 0 || c.min_move_ms > c.max_move_ms || c.max_move_ms > 30000) return false;
  for (std::size_t i = 0; i < kJointCount; ++i) {
    if (!calibration_valid(c.joints[i])) return false;
    for (std::size_t j = 0; j < i; ++j) if (c.joints[i].servo_id == c.joints[j].servo_id) return false;
  }
  config_ = c; configured_ = true; return true;
}
SafetyError SafetyGate::check_feedback(const JointFeedback& f, std::uint32_t now) const noexcept {
  if (!f.measured_and_valid) return SafetyError::BadFeedback;
  if (static_cast<std::uint32_t>(now - f.captured_ms) >= config_.feedback_timeout_ms) return SafetyError::StaleFeedback;
  for (std::size_t i = 0; i < kJointCount; ++i) {
    if (!std::isfinite(f.radians[i]) || f.radians[i] < config_.joints[i].min_rad || f.radians[i] > config_.joints[i].max_rad) return SafetyError::BadFeedback;
  }
  return SafetyError::None;
}
SafetyError SafetyGate::arm(const JointFeedback& f, std::uint32_t now) noexcept {
  if (state_ == SafetyState::Fault) return SafetyError::LatchedFault;
  if (state_ == SafetyState::Armed) return SafetyError::None; // Re-arming cannot refresh watchdog.
  if (!configured_) return SafetyError::BadConfig;
  if (!config_.geometry_and_transmission_verified || !config_.electrical_interface_verified || !config_.bench_and_estop_verified) return SafetyError::NotVerified;
  const auto error = check_feedback(f, now);
  if (error != SafetyError::None) return error;
  last_valid_target_ms_ = now; state_ = SafetyState::Armed; return SafetyError::None;
}
void SafetyGate::disarm() noexcept { if (state_ != SafetyState::Fault) state_ = SafetyState::Disarmed; }
void SafetyGate::latch_fault() noexcept { state_ = SafetyState::Fault; }
bool SafetyGate::clear_fault() noexcept {
  if (state_ != SafetyState::Fault) return false;
  state_ = SafetyState::Disarmed; return true;
}
SafetyError SafetyGate::fail(SafetyError e) noexcept { state_ = SafetyState::Fault; return e; }
SafetyError SafetyGate::poll(std::uint32_t now) noexcept {
  if (state_ == SafetyState::Fault) return SafetyError::LatchedFault;
  if (state_ != SafetyState::Armed) return SafetyError::Disarmed;
  if (static_cast<std::uint32_t>(now - last_valid_target_ms_) >= config_.watchdog_ms) return fail(SafetyError::Watchdog);
  return SafetyError::None;
}
SafetyError SafetyGate::plan(const JointTarget& t, const JointFeedback& f, std::uint32_t now, CommandPlan& out) noexcept {
  out = {}; // A failed request never leaves a reusable valid plan.
  auto error = poll(now); if (error != SafetyError::None) return error;
  error = check_feedback(f, now); if (error != SafetyError::None) return fail(error);
  if (static_cast<std::uint32_t>(now - t.created_ms) >= config_.target_timeout_ms) return fail(SafetyError::StaleTarget);
  if (t.duration_ms < config_.min_move_ms || t.duration_ms > config_.max_move_ms) return fail(SafetyError::BadDuration);
  CommandPlan candidate;
  for (std::size_t i = 0; i < kJointCount; ++i) {
    const auto& j = config_.joints[i]; const double q = t.radians[i];
    if (!std::isfinite(q) || q < j.min_rad || q > j.max_rad) return fail(SafetyError::JointLimit);
    const double tick = j.zero_tick + j.direction * j.ticks_per_rad * q;
    if (!std::isfinite(tick) || tick < j.min_tick || tick > j.max_tick) return fail(SafetyError::JointLimit);
    const auto rounded = static_cast<std::uint16_t>(std::lround(tick)); // Safe 0..1000 from calibration.
    const double realized_q = (rounded - j.zero_tick) / (j.direction * j.ticks_per_rad);
    if (realized_q < j.min_rad || realized_q > j.max_rad) return fail(SafetyError::JointLimit);
    const double delta = std::abs(realized_q - f.radians[i]);
    if (delta > j.max_step_rad) return fail(SafetyError::StepLimit);
    if (delta / (static_cast<double>(t.duration_ms) / 1000.0) > j.max_speed_rad_s) return fail(SafetyError::SpeedLimit);
    if (!lx16a::make_move(j.servo_id, rounded, t.duration_ms, candidate.frames[i])) return fail(SafetyError::JointLimit);
  }
  candidate.valid = true; out = candidate; last_valid_target_ms_ = now; return SafetyError::None;
}
} // namespace robot
