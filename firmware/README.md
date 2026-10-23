# 单腿机器人软件预研 v0.1

面向正在准备的 **STM32F407ZGT6（正点原子最小系统板，具体版次待核实）+ 3 个 LX-16A**。这是一份可在电脑上编译测试的软件起点，**不是可直接烧录的固件，也不是可直接运动的机器狗程序**。不包含真实舵机编号、引脚表、实际机械尺寸或已验证的标定值。

## 当前已经有的内容

- 原创 C++17 LX-16A 编解码：115200 baud、校验和、固定容量接收解析、位置/电压/温度/负载状态解码
- 单事务、非阻塞半双工总线状态机：TX/RX、限时、取消、异常后恢复门槛；写操作仅报告“已发出”，不假装收到舵机确认
- 默认未解锁的单腿关节目标安全校验器：3 个唯一且明确配置的 ID、校准/电气/机械确认、关节范围、单次步幅、基于测量反馈的速度约束、时间戳、看门狗和故障锁存
- 与硬件完全分离的理想化 3DOF 单腿 FK/IK，含坐标约定、显式分支及主机测试
- 可替换的 JointTargetSource 接口，为后续手动调试、遥控步态、仿真策略保留统一的“关节目标”边界
- CMake/CTest 构建文件与不依赖 CMake 的 GCC 测试脚本
- STM32 未绑定适配器和未配置标定文件，均拒绝实际 I/O

## 立即运行：不需要硬件

已有 CMake、C++17 编译器时：

```sh
cmake -S . -B build -DROBOT_ENABLE_SANITIZERS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/dry_run
```

只有 GCC 时：

```sh
./tools/test_host.sh plain
./tools/test_host.sh sanitizers
```

某些受跟踪的云环境无法运行 LeakSanitizer，可使用 `ASAN_OPTIONS=detect_leaks=0 ./tools/test_host.sh sanitizers`；这保留 ASan/UBSan 检查，但不测试内存泄漏。

测试使用固定的虚构标定参数，只用于软件验证，不能抄入实物配置。dry_run 预期输出 `Transport ready: no. Configuration accepted: no.`，不连接串口。

实测结果及没有执行的检查见 [测试报告](docs/TEST_REPORT.zh-CN.md)。本次环境没有 CMake，CMake/CTest 文件已提供，但实际运行的是等价的 GCC 主机测试。

## 文件导航

| 路径 | 用途 |
|---|---|
| include/robot/lx16a.hpp / src/lx16a.cpp | 协议数据帧、接收解析、只读解码 |
| include/robot/bus.hpp / src/bus.cpp | 非阻塞 UART 传输边界及事务状态机 |
| include/robot/safety.hpp / src/safety.cpp | 校准配置、目标接口、软件安全门槛 |
| include/robot/kinematics.hpp / src/kinematics.cpp | 理想化数学模型，不连接硬件 |
| config/bench_unconfigured.hpp | 故意无有效配置；不是可用标定示例 |
| platform/stm32/unbound_transport.hpp | 所有硬件操作拒绝的编译模板 |
| examples/dry_run.cpp | 无硬件演示 |
| tests/ | 主机自动化测试 |
| docs/ | 中文协议来源、坐标约定、集成和测试说明 |

## 必须先知道的安全边界

1. **DISARMED 和 FAULT 只阻止生成新的运动计划，不等于舵机停止，更不等于断电。** 已发出的动作可能继续。必须另外准备物理急停/断电，并验证真实停止路径；此版本没有接好这条路径。
2. SafetyGate 只生成全部通过校验的 3 条命令计划，不自动发送。低层 ServoBus 不替代安全门；它可以直接发送原始运动指令，不能作为安全的公开控制入口。已经取出的 CommandPlan 不会在 disarm/fault 后自动失效，未来执行器必须在发送时重新校验解锁状态、故障、时间与反馈。本版缺少这一执行器，因此禁止绑定真实运动输出。
3. 三条 MoveTimeWrite 分别启动，不是原子同步动作。本版本没有多关节调度器、同步起动和部分发送失败恢复，不能直接循环发送计划做步态。
4. 配置默认 ID=255（未设置）、校准无效、各种验证为 false。没有自动扫描、广播动作、改 ID、EEPROM 写入或力矩开启命令。LX-16A 可能上电自带保持力，代码不主动开启力矩不代表硬件无力矩。
5. 理想化 FK/IK 不代表实际 V2/V3 机械传动。RoboDog V2 源码有 2:1 齿轮映射及个体偏置，实物机构、齿轮耦合与零位未确认前，不接通 IK→舵机路径。当前仿射映射只适用于经实测证明独立线性的传动，耦合/非线性机构需另写模型。
6. 不给未确认的 BusLinker 接线。旧版板卡可能存在 5 V 逻辑电平、HC126 与 CH340 TX 冲突问题；实物版次和电路未核实，不可把 STM32 UART 直接接上尝试。
7. 本项目不保证实时性、功能安全或运动安全。程序没有碰过真实硬件；电机、电源、机械限位、温升、电压保护、负载和跌落风险均未实测。

下一步按 [集成检查表](docs/STM32_INTEGRATION.zh-CN.md) 收集板卡照片、原理图、机械尺寸与校准记录，再逐级做单舵机、空载、单腿固定台架验证。

## 参考与授权

本目录协议实现与数学模块为独立编写，没有复制 RoboDog 源码、HAL、CMSIS 或第三方库。[来源说明](docs/PROTOCOL_AND_SOURCES.zh-CN.md) 记录 LX-16A 厂商资料以及 RoboDog V2/V3 的固定提交与 MIT 许可证。该模板尚未替用户决定公开发布时采用哪种许可证，也没有发布到远程仓库。
