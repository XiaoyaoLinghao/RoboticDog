#pragma once
#include "robot/bus.hpp"

namespace robot::stm32 {
// Intentionally inert. No USART instance, pin, clock, DMA, direction polarity,
// level shifter, power pin or board revision is fabricated.
class UnboundTransport final : public Transport {
 public:
  bool verified_ready() const noexcept override { return false; }
  bool begin_tx(const std::uint8_t*, std::size_t) noexcept override { return false; }
  TxState poll_tx() noexcept override { return TxState::Error; }
  bool enter_rx() noexcept override { return false; }
  std::size_t receive(std::uint8_t*, std::size_t) noexcept override { return 0; }
  bool clear_rx() noexcept override { return false; }
  void abort() noexcept override {}
};
} // namespace robot::stm32
