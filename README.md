# RoboticDog

DIY 12DOF 四足机器人：先固定单腿台架，再实现平整室内地面上的低速遥控行走，之后研究仿真训练与策略部署。

## 当前阶段

2026-10-24：M0 材料完成，待用户讨论确认（AWAITING_USER）。硬件尚未到货。现有 C++17 v0.1 是离线软件原型，尚不能驱动真实设备。

任务清单、包含硬件/执行器/输入协议适配边界的架构设计与源码审查已落档。经用户授权安装 Build Tools、MSVC 和 Windows SDK 后，原始源码在 MSVC 下 Debug、Release 各 5/5 测试通过。当前 CMake 的 sanitizer 开关拒绝 MSVC，ASan/UBSan/LSan 未运行；基线结论为 PASS_WITH_LIMITATIONS。主协调审查与证据见 [M0报告](docs/m0/M0_REPORT.zh-CN.md)。M1 未开始，须经用户讨论确认。

用户已接受当前sanitizer覆盖限制。[M1执行链与HAL方案](docs/M1_EXECUTION_PLAN.zh-CN.md)已细化为六个顺序任务包，列出交付物、验收与R01–R09处置；当前只评审设计，确认前不开始开发。

## 开发方式

主协调负责需求、架构、规格、完整差异审查和验收；后续代码修改、调试由用户指定的 VS Code AI 接手。本次精确测试由本地 Claude Code CLI 执行，VS Code 会话整理报告，文件工作与命令验证分开。保留既有模型/provider；每个有边界的实现任务采用独立工作树。硬件与烧录另按准入流程安排。

文档、测试和审查进度在有实质变化时提交并同步到此仓库。原始交接资料已入库，新的项目文档区分历史证据与当前验证结果。

## 项目文档

- [P1S PETG 单腿 3D 打印说明](docs/3D_PRINTING_GUIDE.zh-CN.md)、[模型版本与打印数量](PRINT_INVENTORY.zh-CN.md)
- [里程碑与M0—M2任务清单](docs/ROADMAP.zh-CN.md)
- [M1执行链与HAL实施方案（待确认）](docs/M1_EXECUTION_PLAN.zh-CN.md)
- [控制架构与硬件、执行器、输入协议抽象](docs/architecture/CONTROL_ARCHITECTURE.zh-CN.md)
- [工作方式与GitHub同步规则](docs/WORKFLOW.zh-CN.md)
- [当前进度](docs/PROGRESS.zh-CN.md)
- [M0报告与实际验证证据](docs/m0/M0_REPORT.zh-CN.md)
- [VS Code接手说明、测试要求与修改建议](docs/VSCODE_HANDOFF.zh-CN.md)
- [原始交接入口](START_HERE.md)、[原始验收用例](ACCEPTANCE_TESTS.md)

根目录旧交接文档与 `firmware/docs/` 描述交接时的v0.1。当前范围、授权和方案以本README及 `docs/` 中注明日期的项目文档为准；历史测试报告不替代本机复验。

默认保持真实输出关闭。不包含已验证的硬件标定、接线或物理停止能力。原创代码许可证尚未选定。
