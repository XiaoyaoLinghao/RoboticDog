#include "robot/bus.hpp"
#include "platform/stm32/unbound_transport.hpp"
#include "test_util.hpp"
#include <algorithm>
#include <vector>
#include <type_traits>
using namespace robot;
struct Fake : Transport {
  bool ready=true, tx_ok=true, rx_ok=true, clear_ok=true, oversized_return=false;
  TxState tx=TxState::Busy;
  std::vector<std::uint8_t> sent, incoming;
  unsigned aborts=0, rx_calls=0; std::size_t chunk=32;
  bool verified_ready() const noexcept override { return ready; }
  bool begin_tx(const std::uint8_t* p,std::size_t n) noexcept override { sent.assign(p,p+n); return tx_ok; }
  TxState poll_tx() noexcept override { return tx; }
  bool enter_rx() noexcept override { ++rx_calls; return rx_ok; }
  std::size_t receive(std::uint8_t* out,std::size_t cap) noexcept override {
    if(oversized_return) return cap+1;
    auto n=std::min({cap,chunk,incoming.size()}); std::copy_n(incoming.begin(),n,out); incoming.erase(incoming.begin(),incoming.begin()+static_cast<std::ptrdiff_t>(n)); return n;
  }
  bool clear_rx() noexcept override { incoming.clear(); return clear_ok; }
  void abort() noexcept override { ++aborts; }
  void append(const lx16a::Frame& f) { lx16a::Packet p; CHECK(lx16a::encode(f,p)); incoming.insert(incoming.end(),p.bytes.begin(),p.bytes.begin()+static_cast<std::ptrdiff_t>(p.size)); }
};
int main() {
  static_assert(!std::is_copy_constructible_v<ServoBus> && !std::is_move_constructible_v<ServoBus>);
  lx16a::Frame request; CHECK(lx16a::make_read(1,lx16a::Command::PositionRead,request));
  stm32::UnboundTransport unbound; ServoBus disabled(unbound); CHECK(!disabled.start(request,0,50));
  Fake t; ServoBus bus(t);
  CHECK(!bus.start(request,0,0)); CHECK(!bus.start(request,0,0x80000000U));
  CHECK(bus.start(request,0,50)); CHECK(!bus.start(request,0,50)); CHECK(bus.poll(1)==BusResult::Busy && t.rx_calls==0);
  t.tx=TxState::Complete; CHECK(bus.poll(2)==BusResult::Busy && t.rx_calls==1);
  lx16a::Frame response=request; response.param_count=2; response.params[0]=0xf4; response.params[1]=1;
  auto unrelated=response; unrelated.id=2; t.append(unrelated);
  auto echo=request; t.append(echo); // A request echo must not count as a reply.
  t.append(response); t.chunk=1;
  for (unsigned i=3;i<30 && bus.busy();++i) bus.poll(i);
  CHECK(bus.result()==BusResult::Reply && !bus.busy()); std::int16_t pos; CHECK(lx16a::decode_position(bus.response(),pos) && pos==500);
  CHECK(bus.poll(500)==BusResult::Reply);
  t.chunk=32; CHECK(bus.start(request,0xfffffffaU,10)); CHECK(bus.poll(3)==BusResult::Busy); CHECK(bus.poll(4)==BusResult::Timeout);
  CHECK(t.aborts>=2);
  CHECK(bus.recovery_required() && !bus.start(request,10,10));
  t.append(response); CHECK(!bus.recover_after_quiet(false)); CHECK(bus.recover_after_quiet(true)); CHECK(t.incoming.empty());
  CHECK(bus.start(request,10,10)); bus.cancel(); CHECK(bus.result()==BusResult::Cancelled); CHECK(bus.recover_after_quiet(true));
  CHECK(bus.start(request,20,10)); t.ready=false; CHECK(bus.poll(21)==BusResult::TransportError); t.ready=true; CHECK(bus.recover_after_quiet(true));
  CHECK(bus.start(request,30,10)); t.tx=TxState::Error; CHECK(bus.poll(31)==BusResult::TransportError); CHECK(bus.recover_after_quiet(true));
  t.tx=TxState::Complete; t.rx_ok=false; CHECK(bus.start(request,40,10)); CHECK(bus.poll(41)==BusResult::TransportError); t.rx_ok=true; CHECK(bus.recover_after_quiet(true));
  t.tx_ok=false; CHECK(!bus.start(request,50,10)); CHECK(bus.result()==BusResult::TransportError); t.tx_ok=true; CHECK(bus.recover_after_quiet(true));
  CHECK(lx16a::make_move(3,500,1000,request)); CHECK(bus.start(request,60,10)); CHECK(bus.poll(61)==BusResult::WriteSent); // NOT motor ACK.
  request.command=lx16a::Command::MoveStart; request.param_count=0; CHECK(!bus.start(request,70,10));
  request.command=lx16a::Command::LoadWrite; request.param_count=1; request.params[0]=1; CHECK(!bus.start(request,70,10));
  CHECK(lx16a::make_unload(3,request)); CHECK(bus.start(request,70,10)); CHECK(bus.poll(71)==BusResult::WriteSent);
  CHECK(lx16a::make_stop(3,request)); request.id=254; CHECK(!bus.start(request,80,10));
  CHECK(lx16a::make_read(1,lx16a::Command::PositionRead,request));
  t.clear_ok=false; CHECK(!bus.start(request,90,20)); CHECK(bus.recovery_required()); CHECK(!bus.recover_after_quiet(true)); t.clear_ok=true; CHECK(bus.recover_after_quiet(true));
  t.oversized_return=true; CHECK(bus.start(request,90,20)); CHECK(bus.poll(91)==BusResult::TransportError); t.oversized_return=false; CHECK(bus.recover_after_quiet(true));
  request.command=lx16a::Command::IdRead; CHECK(!bus.start(request,100,20)); request.command=lx16a::Command::MoveTimeWaitRead; CHECK(!bus.start(request,100,20));
  const auto before=t.aborts; { ServoBus active(t); CHECK(lx16a::make_read(1,lx16a::Command::PositionRead,request)); CHECK(active.start(request,100,20)); } CHECK(t.aborts==before+1);
  std::cout << "PASS bus: unbound disabled, async TX/RX, echo/unrelated rejection, partial RX, deadline wrap, cancellation, faults, no ACK assumption\n";
}
