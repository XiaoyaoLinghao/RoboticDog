# 原创源码清单与接续入口

`firmware/` 是原始 v0.1 的源码副本。交接打包不改动实现，不附构建二进制，不添加原创许可证，也不伪造 Git 历史。逐文件哈希见 `validation/source-manifest.sha256`，完整包清单见 `validation/package-manifest.sha256`。

## 构建与演示

| 文件 | 实际内容 |
|---|---|
| `CMakeLists.txt` | CMake >=3.16，C++17 静态 robot_core；host tests 默认 ON，sanitizer 默认 OFF；注册5个测试 |
| `tools/test_host.sh` | Bash/GCC 直接构建 plain/sanitizers，4个测试+dry_run，任何失败退出 |
| `.gitignore` | 忽略 build*、对象/静态库、elf/bin/hex/map |
| `README.md` | 当前能力、运行方法、安全边界与源码导航 |
| `examples/dry_run.cpp` | 构造未绑定 Transport/未配置 Gate，确认不会就绪或产生 I/O |

## 核心协议与总线

### include/robot/lx16a.hpp 和 src/lx16a.cpp

命名空间 `robot::lx16a`。

- `Frame`：ID、命令、固定 16 字节参数区及参数计数。
- `Packet`：固定最大 22 字节输出帧和长度。
- `encode`：通用有限帧编码，允许表示广播协议帧，但不是发送授权。
- `make_read/make_move/make_stop/make_unload`：受限单播构造；move 检查 ticks<=1000、duration<=30000，未做实物安全校验。
- `decode_position`：有符号 int16；`decode_voltage_mv/temperature_c/loaded` 类型化解码。
- `read_response_size`：只支持位置、电压、温度、加载状态四种读回复。
- `Parser`：逐字节有界解析，噪声恢复、checksum、长度、字节间超时、拒绝计数。

枚举中存在 IdRead、MoveStart 等常量不代表允许通过驱动发送。不要绕过构造层和总线限制直接拼帧恢复危险功能。

### include/robot/bus.hpp 和 src/bus.cpp

- `Transport` 抽象：verified_ready、begin_tx、poll_tx、enter_rx、receive、clear_rx、abort。
- `ServoBus::start`：单事务、合法 timeout、接口已就绪、有限单播命令子集。
- `ServoBus::poll`：TX→RX→Reply 或 WriteSent；超时/错误/取消需要恢复。
- `recover_after_quiet`：需要调用者确证静默，传 true 本身不证明静默。
- `cancel`、析构中的 abort：要求 Transport 停止 DMA/缓冲引用；对象不可复制/移动。
- 每次 poll 最多接收32字节。request/response 按 ID/command/参数数匹配，没有事务序号。

缺口：这是底层库，外部仍可直发运动；不实现安全执行器、全局调度、自动遥测或同步启动。

## 安全与运动学

### include/robot/safety.hpp 和 src/safety.cpp

- `kJointCount=3`：目前仅单腿，不是12关节通用层。
- `JointCalibration`：ID、零位、方向、tick/rad、关节/原始限位、速度、单次步幅、verified。
- `SafetyConfig`：3 关节、目标/反馈/看门狗时限、动作时长，以及几何、电气、台架/急停确认位。
- `JointFeedback`：3个关节 rad、最旧采样时间、真实有效标志。
- `JointTarget`：3个目标 rad、创建时间、动作时长。
- `JointTargetSource::sample`：目标生产者抽象，尚无具体遥控/步态/策略实现。
- `CommandPlan`：3个 Frame + valid。当前没有计划 ID/代际/时效元数据，复制后不会自动撤销。
- `SafetyGate`：configure、arm、disarm、latch_fault、clear_fault、poll、plan。
- `plan`：检查状态、反馈、目标、时长及量化后限位/步幅/速度，全部通过才输出完整计划；成功会更新 last_valid_target_ms。

注意：现有原子性只意味着“一次生成3条通过校验的计划”，不是3条串口命令原子执行。计划函数对未来发送时间的安全没有保证。

### include/robot/kinematics.hpp 和 src/kinematics.cpp

`Geometry`、`JointAngles`、`FootPosition`，`forward/inverse`，明确 Status 和膝/竖直分支。

纯数学、无 I/O/堆分配/第三方依赖。检查几何、NaN/Inf、可达性、奇异、极端尺度和重建残差。只表达理想串联腿，不表达 RoboDog 的具体齿轮、标定或真实限位。详见原始 KINEMATICS 文档。

## 未绑定配置

- `config/bench_unconfigured.hpp`：返回空 SafetyConfig，不是可复制的真实标定样例。
- `platform/stm32/unbound_transport.hpp`：所有就绪/发送/接收初始化拒绝或空返回，不含 GPIO、UART、DMA、时钟或板卡信息。

当前没有任何真实 HAL、CMSIS、toolchain、startup、linker 或 `.ioc` 文件。加入 board 层之前要等待 HARDWARE_PENDING 的证据。

## 测试

| 文件 | 已有重点 |
|---|---|
| `tests/test_protocol.cpp` | golden bytes、有符号读、构造限制、4335往返、200000噪声字节、回绕 |
| `tests/test_bus.cpp` | 简单 FakeTransport、异步顺序、故障/恢复、超容量返回、无 ACK、对象生命周期 |
| `tests/test_safety.cpp` | synthetic fixture、无效配置、arm/fault、原子计划、限位、时间/回绕、量化、极大有限数回归 |
| `tests/test_kinematics.cpp` | 几何/分支/已知姿态、1000随机姿态、44176检查，始终执行自定义 CHECK |
| `tests/test_util.hpp` | 持续生效 CHECK，失败非零退出，不依赖 assert |

现有 bus fake 在 begin_tx 时复制数据，尚不足以穷尽“真实 DMA 保留指针”的生命周期情况。后续 fake 应覆盖这一合同，不因现在测试通过就忽略。

## 原始文档与日志

- `docs/ARCHITECTURE.zh-CN.md`
- `docs/KINEMATICS.zh-CN.md`
- `docs/PROTOCOL_AND_SOURCES.zh-CN.md`
- `docs/STM32_INTEGRATION.zh-CN.md`
- `docs/TEST_REPORT.zh-CN.md`
- `docs/test-logs/gcc-plain.txt`
- `docs/test-logs/gcc-asan-ubsan.txt`

## 后续新增目录建议

可根据实现调整，不要求机械照搬：

```text
include/robot/dispatcher.hpp      受限运动执行与撤销
include/robot/scheduler.hpp       有界总线调度
include/robot/telemetry.hpp       每关节反馈及汇总
include/robot/mapping.hpp         传动/标定边界
include/robot/events.hpp          有界事件日志
src/                             对应实现
platform/fake/                   可脚本化假 Transport/Clock
platform/stm32/                  条件构建的未来适配器，默认关闭
config/                         schema 和未配置模板
examples/                       纯离线目标/回放演示
 tests/                         新增模块及集成故障测试
```

上面这些新文件在 v0.1 中不存在。不要仅因为文档列了名字就写“已实现”。
