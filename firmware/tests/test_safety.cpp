#include "robot/safety.hpp"
#include "config/bench_unconfigured.hpp"
#include "test_util.hpp"
#include <cmath>
using namespace robot;
// Synthetic numbers for host testing only. Not an LX-16A calibration.
SafetyConfig fixture() {
  SafetyConfig c; c.feedback_timeout_ms=100; c.target_timeout_ms=100; c.watchdog_ms=200; c.min_move_ms=10; c.max_move_ms=2000;
  c.geometry_and_transmission_verified=c.electrical_interface_verified=c.bench_and_estop_verified=true;
  for(std::size_t i=0;i<kJointCount;++i) { auto& j=c.joints[i]; j.servo_id=static_cast<std::uint8_t>(10+i); j.zero_tick=500; j.direction=i==1?-1:1; j.ticks_per_rad=100; j.min_rad=-1; j.max_rad=1; j.min_tick=400; j.max_tick=600; j.max_speed_rad_s=1; j.max_step_rad=0.5; j.verified=true; }
  return c;
}
int main() {
  SafetyGate gate; JointFeedback feedback{{0,0,0},0,true}; JointTarget target{{0.1,0.2,0.3},0,1000}; CommandPlan out;
  CHECK(gate.state()==SafetyState::Disarmed);
  CHECK(!gate.configure(config::bench_unconfigured())); CHECK(gate.arm(feedback,0)==SafetyError::BadConfig);
  CHECK(gate.plan(target,feedback,0,out)==SafetyError::Disarmed && !out.valid);
  auto c=fixture(); auto invalid=c; invalid.joints[1].servo_id=invalid.joints[0].servo_id; CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].direction=0; CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].zero_tick=std::nan(""); CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].ticks_per_rad=std::numeric_limits<double>::infinity(); CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].min_rad=-2; CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].max_tick=1001; CHECK(!gate.configure(invalid));
  invalid=c; invalid.joints[0].servo_id=254; CHECK(!gate.configure(invalid));
  invalid=c; invalid.feedback_timeout_ms=0x80000000U; CHECK(!gate.configure(invalid));
  invalid=c; invalid.min_move_ms=0; CHECK(!gate.configure(invalid));
  invalid=c; invalid.geometry_and_transmission_verified=false; CHECK(gate.configure(invalid)); CHECK(gate.arm(feedback,0)==SafetyError::NotVerified);
  CHECK(gate.configure(c)); feedback.measured_and_valid=false; CHECK(gate.arm(feedback,0)==SafetyError::BadFeedback); feedback.measured_and_valid=true;
  CHECK(gate.arm(feedback,100)==SafetyError::StaleFeedback); CHECK(gate.arm(feedback,0)==SafetyError::None); CHECK(!gate.configure(c));
  CHECK(gate.plan(target,feedback,0,out)==SafetyError::None && out.valid);
  CHECK(out.frames[0].params[0]==(510 & 255)); CHECK(out.frames[1].params[0]==(480 & 255)); CHECK(out.frames[2].params[0]==(530 & 255));
  target.radians[2]=2; CHECK(gate.plan(target,feedback,1,out)==SafetyError::JointLimit && !out.valid); CHECK(gate.state()==SafetyState::Fault);
  gate.disarm(); CHECK(gate.state()==SafetyState::Fault); CHECK(gate.arm(feedback,1)==SafetyError::LatchedFault); CHECK(gate.clear_fault()); CHECK(!gate.clear_fault());
  const auto reset = [&] { gate.disarm(); if(gate.state()==SafetyState::Fault) CHECK(gate.clear_fault()); CHECK(gate.configure(c)); feedback={{0,0,0},10,true}; target={{0.1,0.2,0.3},10,1000}; CHECK(gate.arm(feedback,10)==SafetyError::None); };
  reset(); target.radians[0]=std::nan(""); CHECK(gate.plan(target,feedback,10,out)==SafetyError::JointLimit);
  reset(); target.radians[0]=0.6; CHECK(gate.plan(target,feedback,10,out)==SafetyError::StepLimit);
  reset(); target.duration_ms=100; CHECK(gate.plan(target,feedback,10,out)==SafetyError::SpeedLimit);
  reset(); target.duration_ms=0; CHECK(gate.plan(target,feedback,10,out)==SafetyError::BadDuration);
  reset(); feedback.captured_ms=11; CHECK(gate.plan(target,feedback,10,out)==SafetyError::StaleFeedback);
  reset(); target.created_ms=11; CHECK(gate.plan(target,feedback,10,out)==SafetyError::StaleTarget);
  reset(); feedback.captured_ms=110; CHECK(gate.plan(target,feedback,110,out)==SafetyError::StaleTarget);
  reset(); target.created_ms=110; CHECK(gate.plan(target,feedback,110,out)==SafetyError::StaleFeedback);
  reset(); CHECK(gate.arm(feedback,209)==SafetyError::None); CHECK(gate.poll(210)==SafetyError::Watchdog); // Re-arm does not renew timer.
  reset(); gate.disarm(); feedback.captured_ms=0xfffffff0U; target.created_ms=0xfffffff0U;
  CHECK(gate.arm(feedback,0xfffffff0U)==SafetyError::None); CHECK(gate.plan(target,feedback,4,out)==SafetyError::None);
  CHECK(gate.poll(204)==SafetyError::Watchdog);
  // Quantization must also respect limits and speed, not just requested radians.
  reset(); target.radians={0.0051,0,0}; target.duration_ms=10; CHECK(gate.plan(target,feedback,10,out)==SafetyError::None);
  // Extreme finite calibration: avoid infinity > infinity bypass of speed limit.
  reset(); gate.disarm(); auto extreme=c; auto& j=extreme.joints[0];
  j.ticks_per_rad=1e-306; j.min_rad=-2e307; j.max_rad=2e307; j.min_tick=480; j.max_tick=520; j.max_step_rad=1e308; j.max_speed_rad_s=1e306;
  CHECK(gate.configure(extreme)); CHECK(gate.arm(feedback,10)==SafetyError::None);
  target.radians={1e307,0,0}; target.duration_ms=2000; CHECK(gate.plan(target,feedback,10,out)==SafetyError::SpeedLimit && !out.valid);
  // A mathematically in-range target rounding past a calibrated radian limit is rejected.
  reset(); gate.disarm(); auto quant=c; quant.joints[0].max_rad=0.006;
  CHECK(gate.configure(quant)); CHECK(gate.arm(feedback,10)==SafetyError::None);
  target.radians={0.006,0,0}; CHECK(gate.plan(target,feedback,10,out)==SafetyError::JointLimit && !out.valid);
  std::cout << "PASS safety: fail-closed defaults, unique IDs, calibration/geometry/electrical gates, atomic plan, limits/NaN/speed/step, stale data, latched faults, watchdog/wrap\n";
}
