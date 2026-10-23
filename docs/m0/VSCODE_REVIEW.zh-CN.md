# VS Code M0回传的独立审查

日期：2026-10-23，Asia/Shanghai。审查者：主协调。测试源版本：`479c87bdec2ea33ecec6cf86833ef3a46c69c6e8`。

结论：接纳本轮回传作为**配置/构建失败、测试未运行的阻塞证据**；不构成M0验收通过。M1未开始。编译器未就绪前停止重复构建与测试，由用户决定安装工具。

## 核实内容

- 阅读[接手报告](VSCODE_BASELINE_RESULT.zh-CN.md)及`logs/vscode-baseline/`内全部10份日志，对照本次Claude Code插件会话的相关命令和返回记录；不上传整段会话或初始化信息。
- 三次CMake配置均失败于主机C++编译器识别；三次后续构建因缺少`build.ninja`失败。会话工具记录明确给出这六次调用的退出码1。
- 三次CTest输出均为零测试。相关工具记录报告成功完成，但没有单独打印数字退出码；不能将推定的0写成独立采集的数据，更不能将零测试记为PASS。五个测试在三套配置下均NOT_RUN。
- 独立再次核验firmware原始清单：27/27匹配，0项不符；Git对该源版本的firmware差异为空。`.vscode/settings.json`是既有未跟踪用户文件，本轮保留，不上传。
- 主协调环境与VS Code插件运行在**同一台电脑**。进程使用的Shell版本和环境变量可以不同，不能称为两台机器。
- 新安装Git已实际执行版本查询，路径为`C:\Program Files\Git\cmd\git.exe`，版本`2.56.0.windows.2`。此前插件记录用的是运行时附带的`2.53.0.windows.3`，历史证据不改写。

## 纠偏与复核

主协调获用户明确授权后，通过VS Code的Claude Code插件发送要求，插件已确认并执行文件整理：

1. 停止分号、管道、循环包装等不符合约定的委派命令；不换语言或工具重试缺失/被拒绝操作。
2. 配置失败后停止依赖的构建/测试；缺少工具先汇报，不自行下载、安装或改变全局配置。
3. 修复轮只用Read/Glob/Grep/Write/Edit，文件整理与今后实际命令验证分开。会话记录中，纠偏后的工具调用为Read/Glob/Write/Edit，未再调用Shell或测试工具。
4. 回传初稿的“两个环境=两台机器”、数字退出码推定、命令路径缩略、工作树快照时点等问题，已按原始记录修订并复核。保留十份原始日志，不通过重跑补造证据。
5. 已有用户指令明确M0使用主机CMake；插件观察到的STM32 Toolkit `hostTest`不可用不改变这一范围，也不需要为M0把仓库改造成板级工程。真实烧录仍在未来硬件阶段。

## 用户需准备的工具

建议安装**Visual Studio Build Tools稳定版**，选择“使用C++的桌面开发”，包含MSVC x64/x86编译工具与Windows 11 SDK。可以只安装Build Tools并继续使用VS Code；已有CMake/Ninja无需为了本轮重复安装。依据：[VS Code官方MSVC配置说明](https://code.visualstudio.com/docs/cpp/config-msvc)、[微软MSVC安装说明](https://learn.microsoft.com/en-us/cpp/overview/acquire-msvc)。

当前仓库`ROBOT_ENABLE_SANITIZERS`配置仅针对GNU/Clang；MSVC普通Debug/Release路径与该开关分开。安装MSVC不代表现有ASan/UBSan配置可直接运行。工具到位后先核实普通基线，再如实记录sanitizer支持范围；不擅自修改源码或构建文件扩大兼容。

本轮没有安装软件。主机编译器到位后，协调者先核实路径和生成器，再给插件逐条允许命令；使用新的独立构建目录，保留现有失败证据。必须实际发现并运行全部五项测试；配置、构建成功只是前置条件。完成后再进行M0总验收与架构讨论，未经用户确认不进入M1。

## 回传收口

再次检查VS Code：Claude Code已于21:54提交修订结果，界面显示“Ready for your input”，任务已停止。插件报告中的“未提交/推送、SYNC_PENDING”描述其交付时点；主协调将报告、10份日志和本独立审查纳入项目版本记录，并更新M0问题单。产品源码、测试、构建配置及用户个人配置均不在本次提交范围内。
