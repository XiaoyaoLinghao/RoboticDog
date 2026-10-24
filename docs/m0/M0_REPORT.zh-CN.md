# M0阶段报告

更新：2026-10-24（Asia/Shanghai）；安装与测试发生于2026-10-23。当前结论：**M0文档、完整静态审查、架构候选及主机基线证据已整理并完成主协调审查。Debug/Release各5/5 PASS，sanitizer未运行，主机基线为PASS_WITH_LIMITATIONS；M0总状态AWAITING_USER，等待用户讨论确认。M1未开始。**

## 已完成的工作

- 原始交接包安全解压；46项包文件、27项原始firmware文件摘要匹配。原始源码和历史报告原样留档。
- 将已确认的M0—M2任务、各项交付/验收/依赖和M0后讨论门槛写入项目文档。
- 架构中区分平台HAL、执行器设备适配、输入控制协议适配；定义能力拒绝、量化后复核、设备身份/配置失效及统一控制入口。
- 主协调逐一审查18个实现、测试、配置、示例和构建文件，并核对Claude只读第二意见，记录9项执行链、接口或测试问题；产品代码没有修改。
- Claude CLI实际库存和认证调用可用，限定PowerShell无害命令成功；本轮委派采用只读文件复核和精确CLI验证分离。
- 本地与GitHub建立源版本与进度记录。原始导入、任务/架构文档形成独立可追溯快照。
- 经用户授权完成官方Build Tools、MSVC及Windows SDK安装；通过本地Claude CLI执行精确测试，VS Code Claude会话仅用文件工具整理报告。产品、测试和构建代码未修改。

## 基线测试实际结果

测试源版本：`93a221a2baee2eb0ae8b58c0f636fddb8bf6455b`；firmware与原始导入相同，27/27摘要匹配。工具链为Build Tools 2026 18.11.1、MSVC 19.51.36260.0、Windows SDK 10.0.26100.0、CMake/CTest 4.3.1，生成器Visual Studio 18 2026、x64。

| 步骤 | 实际结果 |
|---|---|
| Debug配置、构建 | PASS；原生工具结果成功，未单列数字退出码 |
| Debug五项测试 | PASS 5/5，8.30秒 |
| Release配置、构建 | PASS；原生工具结果成功，未单列数字退出码 |
| Release五项测试 | PASS 5/5，5.14秒 |
| sanitizer配置 | FAIL，退出1：This sanitizer configuration requires GCC or Clang |
| ASan/UBSan/LeakSanitizer | NOT_RUN；配置失败后未继续构建或测试 |
| ARM构建、硬件验证 | NOT_RUN；本轮范围外 |

每套共五项：protocol、bus、safety、kinematics四个单元测试，加dry_run。两套kinematics均实际输出44176项检查通过；dry_run输出Transport ready: no、Configuration accepted: no、No hardware I/O。Release使用/O2 /Ob2 /DNDEBUG；测试使用自定义CHECK而非会被NDEBUG移除的assert。

主协调核对7对真实原生tool_use/tool_result、完整命令与日志、CTest本地LastTest.log及源码摘要。前6条原生记录没有独立数字退出码，未推定填写0；sanitizer配置有明确退出1。当前CMake仅给GNU/Clang接入组合sanitizer，此限制不能解释成MSVC普遍没有sanitizer，也不能记为sanitizer通过。构建日志没有警告诊断不等于MSVC严格警告策略已达标（R07仍存在）。

最新证据见[MSVC基线报告](MSVC_BASELINE_RESULT.zh-CN.md)、[主协调独立审查](MSVC_REVIEW.zh-CN.md)、[完整命令账本](logs/msvc-baseline/commands.json)、[验收矩阵](ACCEPTANCE_MATRIX.zh-CN.md)。安装前的[环境探测](logs/environment-probe.txt)、[VS Code阻塞报告](VSCODE_BASELINE_RESULT.zh-CN.md)、[历史审查](VSCODE_REVIEW.zh-CN.md)及10份日志原样保留。当时的零测试不是通过，不能与此次5/5混淆。

## 源码审查结论

最大缺口与交接一致：旧计划不会自动撤销、重复plan会更新看门狗、缺少多关节部分发送处理。新增设备可替换要求还需要把SafetyGate里的LX-16A帧/tick/ID依赖移入设备后端，补齐运行期RX错误与取消失败可观察性。现有fake不足以覆盖保留发送指针和DMA/UART完成差异。

补充复核确认RX批量读取缺少实际到达时间依据，以及完整帧、合法stop、异常反馈等回归覆盖缺口；已加入后续任务。没有把低层协议能力与上层策略不同直接认定为缺陷，第二意见的采纳/不采纳理由已留档。

R01–R09均已分配到M1/M2，当前不提前实现。本次无需更改原始代码即可在已记录的Windows/MSVC环境构建并通过原有测试；这不消除执行链或覆盖缺口。完整证据见[源码审查](SOURCE_REVIEW.zh-CN.md)。

## 架构结果与待讨论点

保留单线程轮询、唯一总线所有者、三关节位置模式与独立线性映射。设备适配与输入适配不会改变统一控制状态机；未知能力拒绝，换设备需重新绑定配置与验证。

设计细化建议待讨论：M1占用时拒绝额外目标，M2最多保留一个最新待处理目标；当前批次不与下一批混拼；首输入宽限与持续看门狗分开；配置更新失败保留上一完整配置但仍保持disarmed；未知/缺少必需反馈或停止能力时禁止使能。见[架构文档](../architecture/CONTROL_ARCHITECTURE.zh-CN.md)。

## 当前限制与讨论门槛

缺少主机编译器的阻塞已解除；当前留下sanitizer未覆盖、R07警告策略和R01–R09既有缺口。建议用户讨论是否接受这份带限制的M0基线，以及架构细化建议；只有用户确认后才开始M1。若要求先补sanitizer，应另定工具链/构建修改范围，不自动安装其他工具或把M1修复混入M0。

本轮没有实现HAL/执行器/输入适配新模块，文档合同不等于已验证驱动。后续实现仍由用户指定VS Code AI接手，主协调负责规格和独立审查；精确测试继续采用已核实原生CLI命令。具体入口见[VS Code接手说明](../VSCODE_HANDOFF.zh-CN.md)。

本次未采购、未扫描/连接设备、未烧录或运动、未选择原创许可证。M0满足测试门槛后先汇报，由用户讨论确认后才能进入M1。
