# M0完整源码审查

日期：2026-10-23（Asia/Shanghai）。审查源：`48950e156baa039c0aa6e653e29063ad1a5a7403`；firmware与原始导入`acfba31aa167bea6a3a7e3e9a8b709bb4b7541b9`一致。

主协调已独立阅读全文；另通过本地Claude CLI进行只读第二意见，最终结论由主协调核对。当前报告为静态审查，不能代替尚未运行的本机基线。M0不修改产品代码，已安排的M1/M2功能缺口在相应阶段修复。

## 范围

已逐一审查全部18个可执行内容/构建源文件：

- `firmware/include/robot/{lx16a,bus,safety,kinematics}.hpp`
- `firmware/src/{lx16a,bus,safety,kinematics}.cpp`
- `firmware/tests/{test_protocol,test_bus,test_safety,test_kinematics}.cpp`、`test_util.hpp`
- `firmware/CMakeLists.txt`、`firmware/tools/test_host.sh`
- `firmware/config/bench_unconfigured.hpp`、`firmware/platform/stm32/unbound_transport.hpp`、`firmware/examples/dry_run.cpp`

同时核对全部交接文档、原始测试记录与新架构。重点为状态撤销、时间/回绕、量化、有限数、缓冲生命周期、总线恢复、协议边界、数学分支与Release测试有效性。

## 需要处理的项目

P1表示接入运动前必须解决的执行链缺口，不表示当前默认禁用路径已经驱动了硬件。P2表示接口或测试能力缺口。下列均未在M0修复。

| ID | 优先级/性质 | 代码证据、触发条件与影响 | 处理任务/验收 |
|---|---|---|---|
| R01 | P1，交接已知 | `safety.hpp:45`的CommandPlan是可复制公开Frame数组；`safety.cpp:48`的disarm不撤销已取出的计划；`bus.hpp:33`允许直接start。先plan再disarm，调用者仍可把旧帧交给底层，缺少开传复核 | M1-02/03；D01–D08/D14 |
| R02 | P1，交接已知 | `safety.cpp:81`每次plan成功都把last_valid_target_ms写成now。例：目标created=0、target timeout=100、watchdog=200，在t=90重验同一目标可将看门狗截止从200延至290；旧目标仍会在100后因target timeout拒绝，因此不是无限保活，但已违反重复目标不能续期 | M1-02；D09/D10；拆分纯校验与接受新输入 |
| R03 | P1，交接已知 | `CommandPlan`不记录每关节执行状态；ServoBus只处理单事务，现有源码没有三帧批次的部分失败处理。第一帧上线路后第二帧失败时，现有模块无法统一撤销剩余计划或记录停止不确定性 | M1-04；D07/D11–D13 |
| R04 | P2，HAL合同细化 | `bus.hpp:17/21`的receive仅返回数量，abort无结果。实现可通过verified_ready失效报告整体故障，但缺少明确RX溢出/错误分类与abort失败通道；未来适配器不能将错误当成暂时无数据或默认取消成功 | M1-01合同、M2-01/02；F05/F06/S08/S10、未来B04 |
| R05 | P2，新可替换性要求 | `safety.hpp:2/9/45`与`safety.cpp:8/79`直接依赖LX-16A ID、tick范围和帧；只有字节Transport不足以更换设备协议。输入source接口也不包含来源/会话/序号 | M1-02/03、M2-03；A01–A05、C组；逻辑关节与设备地址分离 |
| R06 | P2，交接已知测试缺口 | `test_bus.cpp:14`在begin_tx复制字节，`:22/58`只计数abort；未验证驱动保留指针时的释放，也无独立DMA TC/UART TC事件。现有通过不能证明这些时序和生命周期 | M1-01；F02/F06/S08 |
| R07 | P2，构建一致性 | `CMakeLists.txt:10`的警告仅PRIVATE作用于robot_core；直接脚本对核心和测试都使用完整警告。两入口警告覆盖不同，MSVC普通路径也未配置相应警告策略 | M1构建增量明确统一策略；不通过删除检查解决；当前记录为限制 |

上述R01–R03不是本轮新发现的隐藏功能，也不是要求M0提前实现M1。源码与交接对它们的描述一致。R04/R05用于落实新HAL/设备/输入分层要求。

## 已核对的设计性质

- 协议编码参数区固定16字节、包最大22字节；Parser工作有界；有符号位置用明确int32转换，避免依赖unsigned转signed的实现定义行为。
- ServoBus限制单播与命令子集，无自动重试；写结果为WriteSent；每poll接收最多32字节；对象不可复制/移动；Transport寿命和abort合同在头文件注明。
- SafetyGate默认无效配置；检查NaN/Inf、时间窗口、重复ID、量化后的关节限位、步幅和速度。极端有限速度比较已有回归。configure失败会使配置失效，这与新架构提出的“保留上一完整配置”不同，迁移需显式测试。
- kinematics无I/O，区分不可达/奇异/数值错误，使用归一化和重建残差；不把理想几何解释成实际机构。全部测试用自定义CHECK，不依赖NDEBUG会移除的assert。
- UnboundTransport与bench_unconfigured保持拒绝I/O/无有效配置，dry_run不打开设备。

## 限制与尚未证明的内容

当前没有确认新的、必须修改原始代码才能继续尝试主机构建的缺陷。但在编译器到位、实际运行前，不能宣布Windows兼容或基线通过。新增M1/M2架构能力均NOT_IMPLEMENTED。

未做覆盖率、专用fuzzing、线程分析、ARM完整链接、实机时序/机械/停止测试。`std::uniform_real_distribution`在不同标准库间不保证生成相同随机数据集；跨工具链对比应保存构建环境和实际输入，而不只记种子。现有44176检查数是历史报告，等待本机实际结果核对。

新增架构的能力描述、设备量化后复核、输入来源识别、配置绑定及开传同步要求见[架构文档](../architecture/CONTROL_ARCHITECTURE.zh-CN.md)。
