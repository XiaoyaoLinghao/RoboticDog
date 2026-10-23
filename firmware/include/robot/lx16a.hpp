#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace robot::lx16a {
constexpr std::uint32_t kBaudRate = 115200;
constexpr std::uint8_t kBroadcastId = 254;
constexpr std::uint8_t kUnsetId = 255;
constexpr std::size_t kMaxParams = 16; // Deliberately bounded supported subset.
constexpr std::size_t kMaxFrame = kMaxParams + 6;
enum class Command : std::uint8_t {
  MoveTimeWrite = 1, MoveTimeRead = 2, MoveTimeWaitWrite = 7,
  MoveTimeWaitRead = 8, MoveStart = 11, MoveStop = 12,
  IdRead = 14, TemperatureRead = 26, VoltageRead = 27,
  PositionRead = 28, LoadWrite = 31, LoadRead = 32
};
struct Frame {
  std::uint8_t id{kUnsetId};
  Command command{Command::PositionRead};
  std::array<std::uint8_t, kMaxParams> params{};
  std::size_t param_count{0};
};
struct Packet { std::array<std::uint8_t, kMaxFrame> bytes{}; std::size_t size{0}; };
bool encode(const Frame&, Packet&) noexcept;
bool make_read(std::uint8_t id, Command command, Frame&) noexcept;
bool make_move(std::uint8_t id, std::uint16_t ticks, std::uint16_t duration_ms, Frame&) noexcept;
bool make_stop(std::uint8_t id, Frame&) noexcept;
bool make_unload(std::uint8_t id, Frame&) noexcept;
bool decode_position(const Frame&, std::int16_t&) noexcept;
bool decode_voltage_mv(const Frame&, std::uint16_t&) noexcept;
bool decode_temperature_c(const Frame&, std::uint8_t&) noexcept;
bool decode_loaded(const Frame&, bool&) noexcept;
std::size_t read_response_size(Command) noexcept; // 0 means unsupported read.

// Single-threaded feed. ISR must enqueue bytes; main loop calls feed().
// At most kMaxFrame bytes retained; caller receives at most one frame per byte.
class Parser {
 public:
  explicit Parser(std::uint32_t interbyte_timeout_ms = 10) noexcept;
  bool feed(std::uint8_t byte, std::uint32_t now_ms, Frame& out) noexcept;
  void expire(std::uint32_t now_ms) noexcept;
  void reset() noexcept;
  std::uint32_t rejected() const noexcept { return rejected_; }
 private:
  void drop_front(std::size_t n) noexcept;
  std::array<std::uint8_t, kMaxFrame> buffer_{};
  std::size_t used_{0};
  std::uint32_t last_byte_ms_{0}, timeout_ms_{10}, rejected_{0};
};
} // namespace robot::lx16a
