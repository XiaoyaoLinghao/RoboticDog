#include "robot/lx16a.hpp"
#include "robot/safety.hpp"
#include "platform/stm32/unbound_transport.hpp"
#include "config/bench_unconfigured.hpp"
#include <cstdio>
int main() {
  robot::stm32::UnboundTransport port;
  robot::ServoBus bus(port);
  robot::SafetyGate gate;
  const bool configured = gate.configure(robot::config::bench_unconfigured());
  std::printf("Host dry-run only. Transport ready: %s. Configuration accepted: %s.\n",
              port.verified_ready() ? "yes" : "no", configured ? "yes" : "no");
  std::puts("No servo IDs, pin assignments, or mechanical limits are configured. No hardware I/O.");
  return (configured || port.verified_ready() || bus.busy()) ? 1 : 0;
}
