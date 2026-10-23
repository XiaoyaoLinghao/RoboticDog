#include "robot/lx16a.hpp"
#include <algorithm>

namespace robot::lx16a {
namespace {
bool unicast(std::uint8_t id) noexcept { return id < kBroadcastId; }
std::uint8_t checksum(const std::uint8_t* data, std::size_t count) noexcept {
  std::uint8_t sum = 0;
  for (std::size_t i = 0; i < count; ++i) sum = static_cast<std::uint8_t>(sum + data[i]);
  return static_cast<std::uint8_t>(~sum);
}
std::uint16_t u16(const Frame& f) noexcept {
  return static_cast<std::uint16_t>(f.params[0] | (static_cast<std::uint16_t>(f.params[1]) << 8U));
}
void put16(Frame& f, std::size_t i, std::uint16_t x) noexcept {
  f.params[i] = static_cast<std::uint8_t>(x & 0xffU);
  f.params[i + 1] = static_cast<std::uint8_t>(x >> 8U);
}
}
bool encode(const Frame& frame, Packet& out) noexcept {
  out = {};
  if (frame.id == kUnsetId || frame.param_count > kMaxParams) return false;
  out.bytes[0] = 0x55; out.bytes[1] = 0x55; out.bytes[2] = frame.id;
  out.bytes[3] = static_cast<std::uint8_t>(frame.param_count + 3);
  out.bytes[4] = static_cast<std::uint8_t>(frame.command);
  std::copy_n(frame.params.begin(), frame.param_count, out.bytes.begin() + 5);
  out.size = frame.param_count + 6;
  out.bytes[out.size - 1] = checksum(out.bytes.data() + 2, out.size - 3);
  return true;
}
std::size_t read_response_size(Command c) noexcept {
  switch (c) {
    case Command::PositionRead: case Command::VoltageRead: return 2;
    case Command::TemperatureRead: case Command::LoadRead: return 1;
    default: return 0;
  }
}
bool make_read(std::uint8_t id, Command c, Frame& out) noexcept {
  out = {};
  if (!unicast(id) || read_response_size(c) == 0) return false;
  out.id = id; out.command = c; return true;
}
bool make_move(std::uint8_t id, std::uint16_t ticks, std::uint16_t ms, Frame& out) noexcept {
  out = {};
  if (!unicast(id) || ticks > 1000 || ms > 30000) return false;
  out.id = id; out.command = Command::MoveTimeWrite; out.param_count = 4;
  put16(out, 0, ticks); put16(out, 2, ms); return true;
}
bool make_stop(std::uint8_t id, Frame& out) noexcept {
  out = {}; if (!unicast(id)) return false;
  out.id = id; out.command = Command::MoveStop; return true;
}
bool make_unload(std::uint8_t id, Frame& out) noexcept {
  out = {}; if (!unicast(id)) return false;
  out.id = id; out.command = Command::LoadWrite; out.param_count = 1; out.params[0] = 0; return true;
}
bool decode_position(const Frame& f, std::int16_t& out) noexcept {
  if (f.command != Command::PositionRead || f.param_count != 2 || !unicast(f.id)) return false;
  const auto raw = u16(f);
  const std::int32_t signed_value = raw >= 0x8000U ? static_cast<std::int32_t>(raw) - 65536 : raw;
  out = static_cast<std::int16_t>(signed_value); return true;
}
bool decode_voltage_mv(const Frame& f, std::uint16_t& out) noexcept {
  if (f.command != Command::VoltageRead || f.param_count != 2 || !unicast(f.id)) return false;
  out = u16(f); return true;
}
bool decode_temperature_c(const Frame& f, std::uint8_t& out) noexcept {
  if (f.command != Command::TemperatureRead || f.param_count != 1 || !unicast(f.id)) return false;
  out = f.params[0]; return true;
}
bool decode_loaded(const Frame& f, bool& out) noexcept {
  if (f.command != Command::LoadRead || f.param_count != 1 || !unicast(f.id) || f.params[0] > 1) return false;
  out = f.params[0] == 1; return true;
}
Parser::Parser(std::uint32_t timeout) noexcept : timeout_ms_(timeout > 0 && timeout < 0x80000000U ? timeout : 10) {}
void Parser::reset() noexcept { used_ = 0; }
void Parser::drop_front(std::size_t n) noexcept {
  n = std::min(n, used_);
  for (std::size_t i = n; i < used_; ++i) buffer_[i - n] = buffer_[i];
  used_ -= n;
}
void Parser::expire(std::uint32_t now) noexcept {
  if (used_ && static_cast<std::uint32_t>(now - last_byte_ms_) >= timeout_ms_) { reset(); ++rejected_; }
}
bool Parser::feed(std::uint8_t byte, std::uint32_t now, Frame& out) noexcept {
  expire(now); last_byte_ms_ = now;
  if (used_ == buffer_.size()) { drop_front(1); ++rejected_; }
  buffer_[used_++] = byte;
  while (used_ >= 2) {
    if (buffer_[0] != 0x55 || buffer_[1] != 0x55) { drop_front(1); continue; }
    if (used_ < 4) return false;
    const std::size_t length = buffer_[3];
    if (buffer_[2] == kUnsetId || length < 3 || length > kMaxParams + 3) {
      drop_front(1); ++rejected_; continue;
    }
    const std::size_t total = length + 3;
    if (used_ < total) return false;
    if (buffer_[total - 1] != checksum(buffer_.data() + 2, total - 3)) {
      drop_front(1); ++rejected_; continue;
    }
    out = {}; out.id = buffer_[2]; out.command = static_cast<Command>(buffer_[4]);
    out.param_count = length - 3;
    std::copy_n(buffer_.begin() + 5, out.param_count, out.params.begin());
    drop_front(total); return true;
  }
  return false;
}
} // namespace robot::lx16a
