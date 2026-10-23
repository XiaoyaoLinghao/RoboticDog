# 当前开发进度

更新时间：2026-10-23（Asia/Shanghai）。当前：M0 IN_PROGRESS；M1/M2 TODO；硬件未到，真实输出禁用。

| 任务 | 工作状态 | 证据/下一步 |
|---|---|---|
| M0-01 | IN_PROGRESS | 原ZIP SHA-256 `c66b9031f816d57d3b5d88a4cb2dcc8b55a274f9368d2839e8eeed1d7d91dc2f`；46/46包文件、27/27源文件匹配；同步版本记录中 |
| M0-02 | IN_PROGRESS | Claude 2.1.308、PowerShell及既有deepseek-v4.1-flash[1m]已验证；CMake/Ninja存在；未找到Windows主机C++编译器，已请求便携工具链授权 |
| M0-03 | BLOCKED | 等待主机编译器。Debug/Release/sanitizer均NOT_RUN，不能沿用云端PASS |
| M0-04 | IN_PROGRESS | 审查核心、测试和构建；结果将写入docs/m0/ |
| M0-05 | IN_PROGRESS | 架构新增平台HAL、执行器和输入协议适配；候选契约见architecture目录 |

M0完成后向用户汇报基线、源码审查和架构结果，等待讨论确认，不自动开始M1。
