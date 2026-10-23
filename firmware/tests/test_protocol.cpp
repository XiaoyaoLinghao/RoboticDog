#include "robot/lx16a.hpp"
#include "test_util.hpp"
#include <vector>
using namespace robot::lx16a;
int main() {
  Frame f, decoded; Packet p;
  CHECK(make_move(1, 500, 1000, f)); CHECK(encode(f, p));
  const std::uint8_t expected[] = {0x55,0x55,1,7,1,0xf4,1,0xe8,3,0x16};
  CHECK(p.size == sizeof expected);
  for (std::size_t i = 0; i < p.size; ++i) CHECK(p.bytes[i] == expected[i]);
  Parser parser;
  std::size_t frames = 0;
  for (std::size_t i = 0; i < p.size; ++i) if (parser.feed(p.bytes[i], 0, decoded)) ++frames;
  CHECK(frames == 1 && decoded.param_count == 4 && decoded.params[0] == 0xf4);
  CHECK(!make_move(254, 0, 1, f)); CHECK(!make_move(255, 0, 1, f));
  CHECK(!make_move(0, 1001, 1, f)); CHECK(!make_move(0, 0, 30001, f));
  CHECK(make_move(253, 1000, 30000, f));
  CHECK(!make_read(254, Command::PositionRead, f));
  CHECK(!make_read(1, Command::MoveStart, f));
  CHECK(!make_read(1, Command::IdRead, f)); CHECK(!make_read(1, Command::MoveTimeRead, f)); CHECK(!make_read(1, Command::MoveTimeWaitRead, f));
  CHECK(make_read(1, Command::PositionRead, f)); CHECK(encode(f, p));
  CHECK(p.size == 6 && p.bytes[5] == 0xdf);
  f.param_count = kMaxParams + 1; CHECK(!encode(f, p) && p.size == 0);
  f = {}; f.id = 1; f.command = Command::PositionRead; f.param_count = 2;
  for (const int value : {-32768, -1, 0, 500, 1000, 32767}) {
    const auto raw = static_cast<std::uint16_t>(value);
    f.params[0] = static_cast<std::uint8_t>(raw); f.params[1] = static_cast<std::uint8_t>(raw >> 8U);
    std::int16_t result = 0; CHECK(decode_position(f, result)); CHECK(result == value);
  }
  f.param_count = 1; std::int16_t pos; CHECK(!decode_position(f, pos));
  f.command = Command::LoadRead; f.params[0] = 2; bool loaded; CHECK(!decode_loaded(f, loaded));
  f.params[0] = 1; CHECK(decode_loaded(f, loaded) && loaded);
  f.command = Command::VoltageRead; f.param_count = 2; f.params[0] = 0x10; f.params[1] = 0x1d;
  std::uint16_t mv; CHECK(decode_voltage_mv(f, mv) && mv == 7440);
  f.command = Command::TemperatureRead; f.param_count = 1; f.params[0] = 35;
  std::uint8_t temp; CHECK(decode_temperature_c(f, temp) && temp == 35);
  CHECK(make_read(85, Command::PositionRead, f)); CHECK(encode(f, p));
  parser.reset(); frames = 0;
  const std::uint8_t garbage[] = {0,0x55,0,0x55,0x55,0xff,0xff,0x55,0x55,1,0,0x55,0x55,1,255};
  for (auto b : garbage) if (parser.feed(b, 1, decoded)) ++frames;
  for (std::size_t i=0;i<p.size;++i) if (parser.feed(p.bytes[i], 1, decoded)) ++frames;
  CHECK(frames == 1 && decoded.id == 85);
  // Corrupt checksum, then recovery and back-to-back valid frames.
  parser.reset(); auto bad = p; bad.bytes[bad.size - 1] ^= 1;
  for (std::size_t i=0;i<bad.size;++i) CHECK(!parser.feed(bad.bytes[i], 2, decoded));
  frames=0; for (int k=0;k<2;++k) for (std::size_t i=0;i<p.size;++i) if (parser.feed(p.bytes[i], 2, decoded)) ++frames;
  CHECK(frames==2 && parser.rejected() > 0);
  // Partial packet expires across the uint32 tick wrap.
  parser.reset(); CHECK(!parser.feed(0x55, 0xfffffffcU, decoded));
  CHECK(!parser.feed(0x55, 0xfffffffdU, decoded)); parser.expire(8);
  frames=0; for (std::size_t i=0;i<p.size;++i) if (parser.feed(p.bytes[i], 8, decoded)) ++frames;
  CHECK(frames==1);
  // Deterministic noise, all supported payload sizes and every valid ID.
  std::uint32_t rng = 0x12345678;
  for (unsigned id=0;id<=254;++id) for (std::size_t count=0;count<=kMaxParams;++count) {
    parser.reset(); f={}; f.id=static_cast<std::uint8_t>(id); f.param_count=count;
    for (std::size_t i=0;i<count;++i) { rng = rng * 1664525U + 1013904223U; f.params[i]=static_cast<std::uint8_t>(rng >> 24U); }
    CHECK(encode(f,p)); frames=0;
    for (std::size_t i=0;i<p.size;++i) if (parser.feed(p.bytes[i], 100, decoded)) ++frames;
    CHECK(frames==1 && decoded.id==f.id && decoded.param_count==count);
    for (std::size_t i=0;i<count;++i) CHECK(decoded.params[i]==f.params[i]);
  }
  for (unsigned i=0;i<200000;++i) { rng=rng*1664525U+1013904223U; if (parser.feed(static_cast<std::uint8_t>(rng >> 24U),i/100,decoded)) CHECK(decoded.param_count<=kMaxParams); }
  parser.reset(); CHECK(make_stop(3,f)); CHECK(encode(f,p)); frames=0;
  for (std::size_t i=0;i<p.size;++i) if (parser.feed(p.bytes[i],3000,decoded)) ++frames;
  CHECK(frames==1);
  std::cout << "PASS protocol: golden bytes, signed reads, limits, resync, wrap, 4335 roundtrips, 200000 noise bytes\n";
}
