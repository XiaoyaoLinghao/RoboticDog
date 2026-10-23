#include "robot/bus.hpp"
namespace robot {
bool ServoBus::start(const lx16a::Frame& frame, std::uint32_t now, std::uint32_t timeout) noexcept {
  if (busy() || recovery_required_ || !transport_.verified_ready() || timeout == 0 || timeout >= 0x80000000U || frame.id >= lx16a::kBroadcastId) return false;
  const auto expected = lx16a::read_response_size(frame.command);
  if (expected && frame.param_count != 0) return false;
  if (!expected) {
    // Deliberately limited driver subset; no EEPROM, ID writes, broadcast, or torque-on.
    if (frame.command == lx16a::Command::MoveTimeWrite) {
      if (frame.param_count != 4) return false;
      const auto pos = frame.params[0] | (static_cast<unsigned>(frame.params[1]) << 8U);
      const auto time = frame.params[2] | (static_cast<unsigned>(frame.params[3]) << 8U);
      if (pos > 1000 || time > 30000) return false;
    } else if (frame.command == lx16a::Command::MoveStop) {
      if (frame.param_count != 0) return false;
    } else if (frame.command == lx16a::Command::LoadWrite) {
      if (frame.param_count != 1 || frame.params[0] != 0) return false;
    } else return false;
  }
  if (!lx16a::encode(frame, packet_)) return false;
  parser_.reset(); response_ = {}; request_ = frame; expected_params_ = expected;
  if (!transport_.clear_rx() || !transport_.begin_tx(packet_.bytes.data(), packet_.size)) {
    transport_.abort(); recovery_required_ = true; result_ = BusResult::TransportError; return false;
  }
  started_ms_ = now; timeout_ms_ = timeout; phase_ = Phase::Tx; result_ = BusResult::Busy; return true;
}
void ServoBus::finish(BusResult r) noexcept {
  transport_.abort(); phase_ = Phase::Idle; result_ = r; parser_.reset();
  if (r == BusResult::Timeout || r == BusResult::TransportError || r == BusResult::Cancelled) recovery_required_ = true;
}
bool ServoBus::recover_after_quiet(bool verified_quiet) noexcept {
  if (busy() || !verified_quiet || !transport_.verified_ready() || !transport_.clear_rx()) return false;
  parser_.reset(); recovery_required_ = false; result_ = BusResult::Idle; return true;
}
void ServoBus::cancel() noexcept { if (busy()) finish(BusResult::Cancelled); }
BusResult ServoBus::poll(std::uint32_t now) noexcept {
  if (!busy()) return result_;
  if (!transport_.verified_ready()) { finish(BusResult::TransportError); return result_; }
  if (static_cast<std::uint32_t>(now - started_ms_) >= timeout_ms_) { finish(BusResult::Timeout); return result_; }
  if (phase_ == Phase::Tx) {
    const auto state = transport_.poll_tx();
    if (state == TxState::Error) { finish(BusResult::TransportError); return result_; }
    if (state == TxState::Busy) return result_;
    if (!transport_.enter_rx()) { finish(BusResult::TransportError); return result_; }
    if (expected_params_ == 0) { finish(BusResult::WriteSent); return result_; }
    phase_ = Phase::Rx;
  }
  parser_.expire(now);
  std::uint8_t bytes[32]{}; // Per-poll work bounded regardless of incoming noise rate.
  const auto count = transport_.receive(bytes, sizeof bytes);
  if (count > sizeof bytes) { finish(BusResult::TransportError); return result_; }
  for (std::size_t i = 0; i < count; ++i) {
    lx16a::Frame frame;
    if (parser_.feed(bytes[i], now, frame) && frame.id == request_.id && frame.command == request_.command && frame.param_count == expected_params_) {
      response_ = frame; finish(BusResult::Reply); return result_;
    }
  }
  return result_;
}
} // namespace robot
