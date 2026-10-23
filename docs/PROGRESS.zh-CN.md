# 当前开发进度

更新时间：2026-10-23（Asia/Shanghai）。当前：M0 **BLOCKED**（VS Code 接手环境已回传：本轮探测范围内未找到可用的 Windows 主机 C++ 编译器；三套 CMake 配置 FAIL，构建尝试 FAIL，Debug/Release/sanitizer 五项测试全部 NOT_RUN）；M1/M2 TODO；硬件未到，真实输出禁用。本轮未安装工具链；主协调已完成回传审查，VS Code任务已停止。

| 任务 | 工作状态 | 证据/下一步 |
|---|---|---|
| M0-01 | DONE | 原ZIP SHA-256 `c66b9031f816d57d3b5d88a4cb2dcc8b55a274f9368d2839e8eeed1d7d91dc2f`；46/46包文件、27/27源文件匹配；原始快照已同步 |
| M0-02 | BLOCKED | VS Code 接手环境本轮探测范围内未找到 MSVC/MinGW/Clang 主机编译器；ARM 交叉工具链（arm-none-eabi-g++ 14.3.1、st-arm-clang 21.1.1 仅 arm/thumb 目标）不能替代 Windows 主机测试；STM32 Toolkit `hostTest` 对本仓库不可用属插件能力限制。待用户决定安装 |
| M0-03 | BLOCKED | 同一台电脑的主协调执行环境与 VS Code 插件执行环境中均未找到可用的主机 C++ 编译器：VS Code 插件环境三套 CMake 配置退出1（`No CMAKE_CXX_COMPILER`），构建退出1（无 build.ninja），三套 CTest 均为 `No tests were found`（0 测试；工具报告成功但数字退出码未单列，不构成通过）；Debug/Release/sanitizer 五项测试全部 NOT_RUN；见[基线阻塞报告](m0/VSCODE_BASELINE_RESULT.zh-CN.md) |
| M0-04 | DONE | 主协调完整静态审查及Claude只读第二意见核对完成，9项问题/合同缺口已归属后续任务；见[M0源码审查](m0/SOURCE_REVIEW.zh-CN.md) |
| M0-05 | AWAITING_USER | 架构候选已完成，包含平台HAL、设备、输入协议及状态/数据合同；待基线阻塞解除后一起讨论确认 |

M0完成后向用户汇报基线、源码审查和架构结果，等待讨论确认，不自动开始M1。

记录入口：[M0报告](m0/M0_REPORT.zh-CN.md)、[验收矩阵](m0/ACCEPTANCE_MATRIX.zh-CN.md)、[VS Code 基线阻塞报告](m0/VSCODE_BASELINE_RESULT.zh-CN.md)、[独立审查与工具建议](m0/VSCODE_REVIEW.zh-CN.md)（日志在 `m0/logs/vscode-baseline/`）。GitHub待办：[M0 #1](https://github.com/XiaoyaoLinghao/RoboticDog/issues/1)、[M1 #2](https://github.com/XiaoyaoLinghao/RoboticDog/issues/2)、[M2 #3](https://github.com/XiaoyaoLinghao/RoboticDog/issues/3)。报告与证据由主协调审查后纳入本次版本记录；插件报告的SYNC_PENDING保留为交付时点的历史状态，当前远程版本以GitHub提交记录为准。

接手入口：[VS Code接手说明](VSCODE_HANDOFF.zh-CN.md)。基线结果已回传但受阻（[阻塞报告](m0/VSCODE_BASELINE_RESULT.zh-CN.md)）：本轮探测范围内未找到主机 C++ 编译器，五项测试 NOT_RUN，待用户决定安装后由主协调下发已核实单条命令重跑。R01–R09为已登记建议，不代表已授权开始M1实现。
