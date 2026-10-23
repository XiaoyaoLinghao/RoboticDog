#pragma once
#include "robot/lx16a.hpp"
#include <array>
#include <limits>

namespace robot {
constexpr std::size_t kJointCount = 3;
struct JointCalibration {
  std::uint8_t servo_id{lx16a::kUnsetId};
  double zero_tick{std::numeric_limits<double>::quiet_NaN()};
  int direction{0}; // Must be +1 or -1, measured for this joint.
  double ticks_per_rad{0}; // Includes verified transmission ratio; no assumed gearing.
  double min_rad{std::numeric_limits<double>::quiet_NaN()};
  double max_rad{std::numeric_limits<double>::quiet_NaN()};
  std::uint16_t min_tick{0}, max_tick{0};
  double max_speed_rad_s{0}, max_step_rad{0};
  bool verified{false};
};
struct SafetyConfig {
  std::array<JointCalibration, kJointCount> joints{};
  std::uint32_t feedback_timeout_ms{0}, target_timeout_ms{0}, watchdog_ms{0};
  std::uint16_t min_move_ms{0}, max_move_ms{0};
  bool geometry_and_transmission_verified{false};
  bool electrical_interface_verified{false};
  bool bench_and_estop_verified{false};
};
struct JointFeedback {
  std::array<double, kJointCount> radians{};
  std::uint32_t captured_ms{0}; // OLDEST of the three included physical sample timestamps.
  bool measured_and_valid{false}; // Never mark commands/predictions as measured feedback.
};
// Replaceable producer boundary: manual UI, remote commands or future trained policy.
// All producers must pass through SafetyGate; neither FK nor IK drives a UART.
struct JointTarget {
  std::array<double, kJointCount> radians{};
  std::uint32_t created_ms{0};
  std::uint16_t duration_ms{0};
};
class JointTargetSource {
 public:
  virtual ~JointTargetSource() = default;
  virtual bool sample(std::uint32_t now_ms, JointTarget& out) noexcept = 0;
};
// Cached plans are not automatically revoked by disarm/fault; future dispatcher must revalidate.
struct CommandPlan { std::array<lx16a::Frame, kJointCount> frames{}; bool valid{false}; };
enum class SafetyState { Disarmed, Armed, Fault };
enum class SafetyError { None, Disarmed, LatchedFault, BadConfig, NotVerified, BadFeedback, StaleFeedback, StaleTarget, BadDuration, JointLimit, SpeedLimit, StepLimit, Watchdog };

// This is a validation/planning gate, NOT a certified emergency stop or a dispatcher.
// Disarm/fault invalidate future plans only; already-issued motor moves may continue.
class SafetyGate {
 public:
  bool configure(const SafetyConfig&) noexcept;
  SafetyError arm(const JointFeedback&, std::uint32_t now_ms) noexcept;
  void disarm() noexcept;
  void latch_fault() noexcept;
  bool clear_fault() noexcept; // Explicit operator action only; returns to DISARMED.
  SafetyError poll(std::uint32_t now_ms) noexcept;
  SafetyError plan(const JointTarget&, const JointFeedback&, std::uint32_t now_ms, CommandPlan&) noexcept;
  SafetyState state() const noexcept { return state_; }
 private:
  SafetyError check_feedback(const JointFeedback&, std::uint32_t now_ms) const noexcept;
  SafetyError fail(SafetyError) noexcept;
  SafetyConfig config_{};
  bool configured_{false};
  SafetyState state_{SafetyState::Disarmed};
  std::uint32_t last_valid_target_ms_{0};
};
} // namespace robot
