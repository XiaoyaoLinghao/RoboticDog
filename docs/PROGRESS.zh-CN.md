# 当前开发进度

更新时间：2026-10-23（Asia/Shanghai）。当前：M0 BLOCKED（缺主机编译器）；M1/M2 TODO；硬件未到，真实输出禁用。

| 任务 | 工作状态 | 证据/下一步 |
|---|---|---|
| M0-01 | DONE | 原ZIP SHA-256 `c66b9031f816d57d3b5d88a4cb2dcc8b55a274f9368d2839e8eeed1d7d91dc2f`；46/46包文件、27/27源文件匹配；原始快照已同步 |
| M0-02 | BLOCKED | 工具清点与Claude验证完成；缺主机C++编译器，已请求便携工具链授权；不使用ARM编译器冒充主机验证 |
| M0-03 | BLOCKED | CMake配置实际退出1；Debug/Release/sanitizer均NOT_RUN，不能沿用云端PASS |
| M0-04 | DONE | 主协调完整静态审查已形成，7项问题/合同缺口已归属后续任务；见[M0源码审查](m0/SOURCE_REVIEW.zh-CN.md) |
| M0-05 | AWAITING_USER | 架构候选已完成，包含平台HAL、设备、输入协议及状态/数据合同；本机测试完成后一起讨论确认 |

M0完成后向用户汇报基线、源码审查和架构结果，等待讨论确认，不自动开始M1。

记录入口：[M0报告](m0/M0_REPORT.zh-CN.md)、[验收矩阵](m0/ACCEPTANCE_MATRIX.zh-CN.md)。GitHub待办：[M0 #1](https://github.com/XiaoyaoLinghao/RoboticDog/issues/1)、[M1 #2](https://github.com/XiaoyaoLinghao/RoboticDog/issues/2)、[M2 #3](https://github.com/XiaoyaoLinghao/RoboticDog/issues/3)。
