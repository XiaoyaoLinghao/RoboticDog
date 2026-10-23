#pragma once
#include "robot/lx16a.hpp"

namespace robot {
enum class TxState { Busy, Complete, Error };
// All callbacks must be bounded and nonblocking. Never call this API concurrently.
class Transport {
 public:
  virtual ~Transport() = default;
  virtual bool verified_ready() const noexcept = 0;
  // Driver must copy or retain data until Complete/abort. Bus owns packet lifetime.
  virtual bool begin_tx(const std::uint8_t* data, std::size_t size) noexcept = 0;
  // Complete means last STOP bit left the UART, not just DMA completion.
  virtual TxState poll_tx() noexcept = 0;
  // Switch driver to high impedance before allowing servo responses; discard TX echo.
  virtual bool enter_rx() noexcept = 0;
  virtual std::size_t receive(std::uint8_t* dst, std::size_t capacity) noexcept = 0;
  // Discard pending RX before starting a request. Must not block.
  virtual bool clear_rx() noexcept = 0;
  // Abort pending DMA/UART and release bus. Failure must be handled by hardware layer.
  virtual void abort() noexcept = 0;
};
enum class BusResult { Idle, Busy, WriteSent, Reply, Timeout, TransportError, Cancelled };
class ServoBus {
 public:
  // Transport must outlive this bus. abort() must cancel DMA before returning.
  explicit ServoBus(Transport& transport) noexcept : transport_(transport) {}
  ~ServoBus() { if (busy()) transport_.abort(); }
  ServoBus(const ServoBus&) = delete;
  ServoBus& operator=(const ServoBus&) = delete;
  ServoBus(ServoBus&&) = delete;
  ServoBus& operator=(ServoBus&&) = delete;
  bool start(const lx16a::Frame&, std::uint32_t now_ms, std::uint32_t timeout_ms) noexcept;
  BusResult poll(std::uint32_t now_ms) noexcept;
  void cancel() noexcept;
  // After timeout/cancel/I/O fault: caller must establish a quiet bus, then explicitly recover.
  // The LX-16A protocol has no sequence number; a late same-command reply is ambiguous.
  bool recover_after_quiet(bool verified_quiet) noexcept;
  bool recovery_required() const noexcept { return recovery_required_; }
  bool busy() const noexcept { return phase_ != Phase::Idle; }
  BusResult result() const noexcept { return result_; }
  const lx16a::Frame& response() const noexcept { return response_; }
 private:
  enum class Phase { Idle, Tx, Rx };
  void finish(BusResult) noexcept;
  Transport& transport_;
  lx16a::Parser parser_{};
  lx16a::Packet packet_{};
  lx16a::Frame request_{}, response_{};
  Phase phase_{Phase::Idle};
  BusResult result_{BusResult::Idle};
  std::uint32_t started_ms_{0}, timeout_ms_{0};
  std::size_t expected_params_{0};
  bool recovery_required_{false};
};
} // namespace robot
