# 本交接包核验记录

核验日期：2026-10-23 UTC。对象：本包内 `firmware/` 原创 v0.1 源码和中文交接资料。没有操作用户电脑，没有硬件连接、供电、扫描、烧录或运动。

## 源码完整性

- 从原始 v0.1 工程复制 27 个源码、配置、构建、文档和历史日志文件。
- 逐文件比较，27 个文件字节完全一致；本交接工作未修改协议、总线、安全门、运动学或测试实现。
- 不打包 `build*` 目录、可执行程序、`.git` 或 MCU 二进制；包中无原创项目远程仓库或 Git 提交历史。
- `validation/source-manifest.sha256` 记录该 27 文件的 SHA-256。
- `validation/package-manifest.sha256` 记录最终包内全部文件（不包含该清单自身）的 SHA-256。
- 未给原创代码新增许可证，也未将第三方仓库许可证套到原创工程。

## 本包实际重跑的测试

环境：Linux 6.18.55 x86_64；`g++ (Debian 14.2.0-19) 14.2.0`。

工作目录为 `firmware/`，实际命令：

```sh
bash tools/test_host.sh plain
ASAN_OPTIONS=detect_leaks=0 bash tools/test_host.sh sanitizers
```

两轮均退出 0；每轮 4 个测试程序与 1 个 dry_run，合计 10 次程序运行。普通轮使用 C++17、O2 和完整警告；sanitizer 轮使用 O1、ASan、UBSan。核心与测试均由脚本重新编译。

| 项目 | 普通 GCC | ASan + UBSan |
|---|---|---|
| protocol | PASS | PASS |
| bus | PASS | PASS |
| safety | PASS | PASS |
| kinematics | PASS，44176 项检查 | PASS，44176 项检查 |
| dry_run | PASS，ready no / config no | PASS，ready no / config no |

记录：
- [本包普通轮日志](validation/handoff-gcc-plain.txt)
- [本包 ASan UBSan 日志](validation/handoff-gcc-asan-ubsan.txt)

LeakSanitizer 已关闭，本包没有重新验证内存泄漏。现有随机/噪声测试不等于模糊测试器或完备证明。

## 没有执行的验证

- 此环境没有 CMake，因此 CMake/CTest 描述按源码核对，未实际运行。
- 未在 Windows/MSVC、macOS/Apple Clang、WSL 或 ARM 工具链上运行。
- 本包这轮没有额外运行 `-DNDEBUG`/独立 Release 组合；普通直接脚本为 O2，但不是完整 CMake Release 验证。
- 无代码覆盖率报告、ThreadSanitizer、专用 fuzzing、长时间可靠性或真实实时性测量。
- 无 STM32 HAL/CMSIS/BSP、完整目标板链接、烧录、UART 电气或电机测试。
- 无真实标定、机械强度、几何/传动一致性、急停、跟踪精度、供电和温升验证。
- DEVELOPMENT_PLAN 与 ACCEPTANCE_TESTS 中新增执行器/调度/日志/四腿功能仍是本地待开发项，不包含在现有通过结果里。

## 资料核对范围

- 原创代码与原始文档已逐一阅读，构建命令对照 CMakeLists.txt 和直接脚本。
- `SOURCES.zh-CN.md` 独立核对了7个相关仓库固定提交、根许可证、目录和高风险初始化入口；其中60个固定文件/目录链接对照完整目录树核验。
- `PRINT_INVENTORY.zh-CN.md` 随后补充装配文字与 STL 几何核查。其关于11个可先打印刚性件、第三髋齿轮候选、轴承文字/照片冲突和当前STL尺寸的结论，优先于早期泛化的 V1/V2 互用假设；不是实物装配通过报告。
- 本包只包含原创源码与参考资料索引，不包含第三方仓库完整代码镜像、STL、系统镜像、APK或策略权重。来源文件给出固定版本和本地只读获取办法。
- 包内相对 Markdown 链接做文件存在性检查；外部来源链接的核对范围由各来源文件说明，不声称所有外部页面永久有效。

## 如何读“通过”

本次复验说明未修改的 v0.1 主机软件路径仍能运行，通过结果与原始报告一致。它没有补齐真实发送安全执行器，也没有降低硬件门槛。本地下一轮开发应按模板写新的结果报告并保留这些历史证据。
