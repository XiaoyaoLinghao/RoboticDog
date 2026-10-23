# VS Code 接手环境 M0 基线验证结果（阻塞报告）

日期：2026-10-23（Asia/Shanghai）。状态词：PASS / FAIL / NOT_RUN / BLOCKED。

**结论：M0 主机基线未通过。** 三套 CMake 配置均因找不到主机 C++ 编译器而 FAIL；随后构建尝试 FAIL；Debug/Release/sanitizer 共五项测试全部 NOT_RUN；M0 总门槛 BLOCKED。本轮没有编译出任何测试可执行文件，也没有运行任何测试用例，因此不产生任何**测试** PASS 结论（源码摘要核验的 PASS 属于独立检查项，见 §1，与测试结果无关）。M1 未开始。

本报告只整理既有执行记录与已落盘日志，未重跑命令、未补造数据、未覆盖原始日志。凡无法从现存记录确认的值一律标注“未知”。

主协调补记：下文“本会话”及“未提交/推送、SYNC_PENDING”均指VS Code插件交付时点。报告已完成独立审查，并与原始日志一起纳入项目版本记录，见[独立审查](VSCODE_REVIEW.zh-CN.md)与[当前进度](../PROGRESS.zh-CN.md)。

---

## 1. 源版本与工作树状态

| 项目 | 实际值 | 来源 |
|---|---|---|
| 仓库根 | `D:\WorkSpace\RoboticDog` | 本会话工作目录 |
| HEAD | `479c87bdec2ea33ecec6cf86833ef3a46c69c6e8` | 本会话 `git rev-parse HEAD`；主协调独立核实一致 |
| 分支 | `main` | 本会话 `git rev-parse --abbrev-ref HEAD` |
| 工作树（会话开始时） | 仅一个未跟踪项：`?? .vscode/settings.json` | 本会话 `git status --porcelain=v1 -uall` |
| 工作树快照（纠偏消息发送前） | 未跟踪：`.vscode/`、`docs/m0/logs/vscode-baseline/` | 主协调核实；**该快照早于本报告的修订** |
| 本轮文档修订（不计入上述快照） | 新增 `docs/m0/VSCODE_BASELINE_RESULT.zh-CN.md`；修改 `docs/PROGRESS.zh-CN.md` | 见 §9 |
| firmware 相对 HEAD 的差异 | 空 | **主协调核实**（本会话未单独执行 `git diff`） |
| 源码摘要核验 | **PASS**：`validation/source-manifest.sha256` 27 项全部 MATCH，0 缺失/0 不符 | 本会话 `Get-FileHash -Algorithm SHA256` 逐项比对，输出 `MATCH=27 MISMATCH_OR_MISSING=0` |

- `.vscode/settings.json` 是用户未提交文件，内容仅为 `cmake.sourceDirectory` 指向 `firmware`；本轮未修改、未纳入提交。
- `firmware/` 下 27 个文件与原始交接一致；产品源码、测试代码、`CMakeLists.txt`、`tools/test_host.sh` 均未被本轮改动。
- 构建产物目录 `build-m0-debug/`、`build-m0-release/`、`build-m0-asan/` 位于仓库根，被根 `.gitignore` 的 `build*/` 忽略，不进入版本记录。

### Git 口径说明

- 本会话实际调用的 Git 是 `C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\git\cmd\git.exe`，`git --version` 报告 `2.53.0.windows.3`（见日志 `01-tool-versions.txt`）。这是**本会话命令实际使用的版本**。
- 主协调另行核实：新安装的 Git 为 `C:\Program Files\Git\cmd\git.exe`，版本 `2.56.0.windows.2`。该信息来自主协调，**不是**本会话测试所用版本，也不代表本报告的 Git 命令由它执行。后续命令统一使用该新安装路径，此事已确定，不再列为待决策事项。
- 本会话未修改 PATH、未设置持久化环境变量；仅在测试远程可读性时于当次进程内设置过 `GIT_TERMINAL_PROMPT=0`。

---

## 2. 环境与工具（版本、绝对路径、来源）

| 工具/对象 | 版本 | 绝对路径 | 来源与状态 |
|---|---|---|---|
| 操作系统 | Microsoft Windows 11 专业版 10.0.26200 64 位 | — | 日志 `01-tool-versions.txt` |
| PowerShell | 7.6.8 | — | 日志 `01-tool-versions.txt`（另：`docs/m0/ENVIRONMENT.zh-CN.md` 历史记录为 7.6.5，属不同时间点，非矛盾） |
| CMake | 4.3.1 | `C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe` | 日志 `01`；`Get-Command` 确认 |
| CTest | 4.3.1 | `C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe` | 日志 `01` |
| Ninja | 1.13.2 | `C:\ST\STM32CubeCLT_1.22.0\Ninja\bin\ninja.exe` | 日志 `01` |
| Git | 2.53.0.windows.3 | `C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\git\cmd\git.exe` | 日志 `01`；本会话实际使用 |
| ARM C++ 交叉编译器 | 14.3.1（GNU Tools for STM32 14.3.rel1.20251027-0700） | `C:\ST\STM32CubeCLT_1.22.0\GNU-tools-for-STM32\bin\arm-none-eabi-g++.exe` | 日志 `01`；**仅 ARM 目标，不能执行 Windows 主机测试** |
| ST ARM Clang | clang 21.1.1（rel2.20260323-1806） | `C:\ST\STM32CubeCLT_1.22.0\st-arm-clang\bin\starm-clang++.exe` | `--version` 报 `Target: arm-st-none-eabi`；`--print-targets` 仅 `arm/armeb/thumb/thumbeb`，无 x86_64 目标；安装目录内无 Windows 导入库、无 Windows C++ 标准库头文件。**不能用于主机测试** |
| Windows 主机 C++ 编译器（g++/clang++/cl/clang-cl） | — | **本轮探测范围内未找到** | 见 §2.1 |
| 构建器（可执行） | Ninja 1.13.2 | 同上 | 可用；但无编译器可驱动 |
| 运行库 | 未知 | — | 未编译，无运行时证据 |

### 2.1 主机编译器探测（只读探测，未安装任何工具）

探测范围与结果：

| 探测方向 | 结果 |
|---|---|
| PATH 上 `Get-Command g++, clang++, cl, clang-cl` | 未找到（仅有 cmake/ctest/ninja） |
| `C:\Program Files\Microsoft Visual Studio`、`C:\Program Files (x86)\Microsoft Visual Studio` | 不存在 |
| `vswhere.exe` | 不存在 |
| 注册表 `HKLM:\SOFTWARE\Microsoft\VisualStudio\SxS\VS7` | 无该键 |
| `C:\Program Files (x86)\Windows Kits`、`C:\Program Files\Microsoft SDKs`、`C:\BuildTools`、`C:\VS` | 不存在 |
| `C:\Program Files\LLVM\bin\clang++.exe`、`C:\msys64`、`C:\mingw64`、`C:\MinGW`、`C:\cygwin64`、`C:\Qt`、chocolatey/scoop/winget 安装目录 | 均不存在或为空 |
| 用户目录内 `llvm-mingw*` / `mingw64` / `w64devkit*`（深度 3） | 无结果 |
| `C:\` 深度 4 内 `g++.exe`/`clang++.exe`/`cl.exe` | 无结果 |
| `D:\WorkSpace` 深度 4 内 `g++.exe`/`clang++.exe`/`cl.exe`/`gcc.exe` | 无结果 |
| WSL | `wsl.exe --status` 退出码 50，提示未安装任何 Linux 发行版；不作为可用测试环境 |
| STM32CubeCLT 自带工具链 | 仅 ARM（arm-none-eabi-g++ / st-arm-clang），见上表 |

结论：**在本轮探测范围内未找到可用的 Windows 主机 C++17 编译器**（MSVC/MinGW/Clang 均未发现），与上一环境 `docs/m0/ENVIRONMENT.zh-CN.md` 记录一致。用户决定后续是否安装；本轮未下载、未安装、未修改全局配置。

### 2.2 STM32 Toolkit 可行性核对（全局约定核对）

按全局规则先尝试用 STM32 Toolkit 承载，而非直接绕过：

- `stm32_project_context`：`ok:true`，但 `project.kind = unknown`、`build.cmakeListsPresent = false`、`capabilities.hostTest = false`。
- `stm32_test_host_discover`：`ok:false`，`code = PROJECT_NOT_CONFIGURED`，`message = Project is not configured.`

即：本项目不是已配置的 STM32 工程，Toolkit 的主机测试能力对该仓库不可用。此限制仅作为插件所见能力记录，不改变项目形态：用户原任务与接手说明已明确 M0 采用主机 CMake 流程，本轮按该流程执行，不要求另行批准例外，也不为运行主机测试把项目改造成 STM32 工程。

---

## 3. 实际命令、工作目录、退出码与日志

工作目录（所有 CMake/CTest 命令）：`D:\WorkSpace\RoboticDog`。

### 3.1 版本与环境记录

| # | 命令（完整原文） | 结果 | 日志 |
|---|---|---|---|
| 1 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --version *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列（is_error=false） | `01-tool-versions.txt` |
| 2 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --version *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |
| 3 | `& 'C:\ST\STM32CubeCLT_1.22.0\Ninja\bin\ninja.exe' --version *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |
| 4 | `& 'C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\git\cmd\git.exe' --version *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |
| 5 | `& 'C:\ST\STM32CubeCLT_1.22.0\GNU-tools-for-STM32\bin\arm-none-eabi-g++.exe' --version *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |
| 6 | `Get-Host \| Select-Object Version *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |
| 7 | `Get-CimInstance Win32_OperatingSystem \| Select-Object Caption,Version,OSArchitecture *>> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\01-tool-versions.txt'` | 工具报告成功，数字退出码未单列 | 同上 |

说明：上表命令中 `\|` 是管道，属于 §7 记录的流程偏离；版本记录本身只读。这些调用仅显示工具报告成功（`is_error=false`），**没有单独显示数字退出码**；本报告不把推定值写成独立采集的退出码。

### 3.2 Debug 配置（FAIL）

```powershell
& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S firmware -B build-m0-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=OFF -DROBOT_BUILD_HOST_TESTS=ON *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\02-configure-debug.log'
```

- 工作目录：`D:\WorkSpace\RoboticDog`
- 退出码：**1**
- 完整输出（日志 `02-configure-debug.log`）：

```
-- The CXX compiler identification is unknown
CMake Error at CMakeLists.txt:2 (project):
  No CMAKE_CXX_COMPILER could be found.

  Tell CMake where to find the compiler by setting either the environment
  variable "CXX" or the CMake cache entry CMAKE_CXX_COMPILER to the full path
  to the compiler, or to the compiler name if it is in the PATH.


-- Configuring incomplete, errors occurred!
```

### 3.3 Release 配置（FAIL）

```powershell
& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S firmware -B build-m0-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DROBOT_ENABLE_SANITIZERS=OFF -DROBOT_BUILD_HOST_TESTS=ON *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\03-configure-release.log'
```

- 退出码：**1**；输出与 §3.2 逐字相同（日志 `03-configure-release.log`）。

### 3.4 sanitizer 配置（FAIL）

```powershell
& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S firmware -B build-m0-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=ON -DROBOT_BUILD_HOST_TESTS=ON *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\04-configure-sanitizers.log'
```

- 退出码：**1**；输出与 §3.2 逐字相同（日志 `04-configure-sanitizers.log`）。
- 注意：失败发生在 `project()` 的编译器识别阶段，早于 `CMakeLists.txt` 中的 sanitizer 分支判断，因此**本轮既没有验证 sanitizer 支持，也没有得到“该工具链不支持 sanitizer”的结论**。

### 3.5 构建尝试（FAIL）

| # | 命令（完整原文） | 退出码 | 日志 | 输出 |
|---|---|---|---|---|
| 1 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --build build-m0-debug --config Debug *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\05-build-debug.log'` | **1**（工具明确返回 Exit code 1） | `05-build-debug.log` | `ninja: error: loading 'build.ninja': The system cannot find the file specified.` |
| 2 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --build build-m0-release --config Release *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\07-build-release.log'` | **1**（工具明确返回 Exit code 1） | `07-build-release.log` | 同上 |
| 3 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --build build-m0-asan --config Debug *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\09-build-sanitizers.log'` | **1**（工具明确返回 Exit code 1） | `09-build-sanitizers.log` | 同上 |

三套目录只生成了 `CMakeCache.txt` 与 `CMakeFiles/`，没有 `build.ninja`，原因就是配置失败。

### 3.6 CTest 尝试（0 测试，不得计为通过）

| # | 命令（完整原文） | 结果 | 日志 | 关键输出 |
|---|---|---|---|---|
| 1 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --test-dir build-m0-debug -C Debug --output-on-failure -V *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\06-ctest-debug.log'` | 工具报告成功（is_error=false），**数字退出码未单列** | `06-ctest-debug.log` | `Added 0 tests`；`No tests were found!!!` |
| 2 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --test-dir build-m0-release -C Release --output-on-failure -V *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\08-ctest-release.log'` | 工具报告成功（is_error=false），**数字退出码未单列** | `08-ctest-release.log` | 同上 |
| 3 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --test-dir build-m0-asan -C Debug --output-on-failure -V *> 'D:\WorkSpace\RoboticDog\docs\m0\logs\vscode-baseline\10-ctest-sanitizers.log'` | 工具报告成功（is_error=false），**数字退出码未单列** | `10-ctest-sanitizers.log` | 同上 |

**“工具报告成功”不构成任何通过证据，也不把推定值当作独立采集的退出码。** 三份日志均显示没有生成任何测试目标（`Added 0 tests`、`No tests were found`），属于“未执行”。按交接要求，三项均记为 **NOT_RUN**，不得记 PASS，也不得以工具未报错替代测试结果。

### 3.7 日志索引

目录：`docs/m0/logs/vscode-baseline/`

| 文件 | 内容 | 对应命令结果 |
|---|---|---|
| `01-tool-versions.txt` | CMake/CTest/Ninja/Git/ARM g++/PowerShell/OS 版本 | 工具报告成功，数字退出码未单列 |
| `02-configure-debug.log` | Debug 配置 | Exit code 1 |
| `03-configure-release.log` | Release 配置 | Exit code 1 |
| `04-configure-sanitizers.log` | sanitizer 配置 | Exit code 1 |
| `05-build-debug.log` | Debug 构建尝试 | Exit code 1 |
| `06-ctest-debug.log` | Debug CTest（0 测试） | 工具报告成功，数字退出码未单列；0 测试，NOT_RUN |
| `07-build-release.log` | Release 构建尝试 | Exit code 1 |
| `08-ctest-release.log` | Release CTest（0 测试） | 工具报告成功，数字退出码未单列；0 测试，NOT_RUN |
| `09-build-sanitizers.log` | sanitizer 构建尝试 | Exit code 1 |
| `10-ctest-sanitizers.log` | sanitizer CTest（0 测试） | 工具报告成功，数字退出码未单列；0 测试，NOT_RUN |

历史日志 `docs/m0/logs/environment-probe.txt` 未改动。本目录为新增，未覆盖任何既有文件。

---

## 4. 三套配置结果汇总

| 配置 | 目录 | 生成器 | BUILD_TYPE | SANITIZERS | 配置结果 | 构建结果 |
|---|---|---|---|---|---|---|
| Debug | `build-m0-debug` | Ninja | Debug | OFF | **FAIL**（退出 1，无编译器） | **FAIL**（退出 1，无 build.ninja） |
| Release | `build-m0-release` | Ninja | Release | OFF | **FAIL**（退出 1，无编译器） | **FAIL**（退出 1，无 build.ninja） |
| ASan/UBSan | `build-m0-asan` | Ninja | Debug | ON | **FAIL**（退出 1，无编译器） | **FAIL**（退出 1，无 build.ninja） |

生成器选择：接手文档模板未指定 `-G`；本轮显式使用已核实的 `Ninja`，避免 CMake 自动选择不可用生成器。该选择在报告中如实记录，属于对模板的补充说明，不改变模板语义。

---

## 5. 五项测试逐项结果

原始源码与测试代码未修改，`CMakeLists.txt` 中定义的测试名即 `protocol`、`bus`、`safety`、`kinematics`、`dry_run`。

| 测试 | Debug | Release | sanitizer | 说明 |
|---|---|---|---|---|
| `protocol`（test_protocol） | **NOT_RUN** | **NOT_RUN** | **NOT_RUN** | 无可执行文件 |
| `bus`（test_bus） | **NOT_RUN** | **NOT_RUN** | **NOT_RUN** | 同上 |
| `safety`（test_safety） | **NOT_RUN** | **NOT_RUN** | **NOT_RUN** | 同上 |
| `kinematics`（test_kinematics） | **NOT_RUN** | **NOT_RUN** | **NOT_RUN** | 同上 |
| `dry_run`（examples/dry_run） | **NOT_RUN** | **NOT_RUN** | **NOT_RUN** | 同上；未打开任何设备 |

**总门槛：BLOCKED。** 没有一项测试实际执行；不存在可用的通过/失败判定。仓库中历史 Linux 报告（`firmware/docs/TEST_REPORT.zh-CN.md`、`firmware/docs/test-logs/*`）是原始交接附带材料，**不能作为本机基线通过证据**。

---

## 6. sanitizer 实际执行范围与限制

| 项目 | 状态 | 说明 |
|---|---|---|
| ASan | **NOT_RUN** | 三套配置均未通过编译器识别；sanitizer 选项从未被实际传给编译器 |
| UBSan | **NOT_RUN** | 同上 |
| LeakSanitizer | **NOT_RUN** | 未执行，也未验证支持性。另需注意：LSan 主要随 GCC/Clang 提供，MSVC 工具链通常不提供；未来若使用 MSVC，必须单独说明并设计替代验证 |
| sanitizer 支持性结论 | **未知** | 配置在 `project()` 阶段即失败，早于 `ROBOT_ENABLE_SANITIZERS` 分支，不能据此声称“工具链不支持 sanitizer” |
| 独立 sanitizer 构建目录 | 已建立 `build-m0-asan` | 仅含失败的 CMakeCache，无产物 |

`sanitizers` 模式在 `firmware/tools/test_host.sh` 中的 Bash 入口同样**未运行**：本机无 `g++`，且该脚本是 Bash 入口，不能当作原生 PowerShell 脚本执行。未用其他方式绕过。

---

## 7. 新发现的问题、影响与最小修复建议

本轮没有执行到编译阶段，因此**没有产生任何源码级新发现**；R01–R09 的代码位置、触发条件与影响均未被本轮验证或否定，本轮也不要求修复。以下问题均为环境/流程层面，与 R01–R09 不重复。

### N01 缺少 Windows 主机 C++ 编译器（BLOCKED，环境）

- 复现条件：在无 MSVC/MinGW/Clang 的 Windows 主机上执行任一 `cmake -S firmware -B ...` 配置。
- 证据：§2.1 探测结果；`02/03/04-configure*.log` 的 `No CMAKE_CXX_COMPILER could be found.`。
- 影响：Debug/Release/sanitizer 全部无法配置、无法构建、无法测试；M0 总门槛阻塞；R07 提到的 MSVC 警告策略差异、以及 Windows 兼容性均无法评估。
- 最小修复建议：由用户决定并安装一套主机 C++17 工具链（MSVC Build Tools 或 MinGW-w64/Clang），安装后由主协调下发已核实的单条命令重跑本报告 §3 的流程。**本轮禁止自行下载或安装**，报告不推荐具体版本以免替用户做安装决定。

### N02 现有 ARM 工具链不能替代主机测试（环境，已确认）

- 证据：`arm-none-eabi-g++` 14.3.1 仅 ARM 目标；`st-arm-clang` 21.1.1 的 `--print-targets` 仅 `arm/armeb/thumb/thumbeb`，且安装内无 Windows 导入库与 Windows C++ 标准库头文件。
- 影响：任何用 ARM 交叉编译器“跑通”主机测试的说法均不成立；不能作为替代路线。
- 修复建议：无源码修复；等待主机工具链。

### N03 CTest 零测试时工具仍报告成功，存在误记为通过的风险（流程/证据）

- 复现条件：对未成功配置的构建目录执行 `ctest --test-dir ... --output-on-failure -V`。
- 证据：`06/08/10-ctest-*.log` 均输出 `Added 0 tests`、`No tests were found!!!`；工具调用显示 `is_error=false`，但**未单列数字退出码**，本报告不推定该值。
- 影响：若只看“工具没有报错”或“没有失败”就下结论，会把未执行误判为 PASS。
- 最小修复建议：验收记录中显式要求“测试数量 > 0 且有逐项用例输出”，零测试一律记 NOT_RUN；无需改代码。

### N04 STM32 Toolkit 无法承载本仓库的主机基线（工具链限制）

- 证据：`stm32_project_context` 报 `project.kind = unknown`、`capabilities.hostTest = false`；`stm32_test_host_discover` 返回 `PROJECT_NOT_CONFIGURED`。
- 影响：该仓库不是已配置 STM32 工程，Toolkit 的主机测试能力对它不可用。用户原任务与接手说明已明确 M0 采用主机 CMake，因此这里只是插件所见的**能力限制**，不构成需要另行批准的例外。
- 建议：不为运行主机测试把项目改造成 STM32 工程；主机基线继续按 CMake 流程执行。N04 不影响本轮阻塞结论。

### N05 本会话命令形式偏离约定（流程，见 §7.1）

见下节，不计为源码问题。

### 7.1 流程偏离与整改

偏离事实（如实记录，不掩盖）：

1. 本轮使用 PowerShell 执行命令，并多次出现**分号串接**与**管道**，不符合“每次恰好一条已核实的原生命令、不得使用分号/管道/`&&`/控制流包装”的约定。典型例子：
   - 首次 Git 状态采集：`git -C D:\WorkSpace\RoboticDog rev-parse HEAD; git -C D:\WorkSpace\RoboticDog rev-parse --abbrev-ref HEAD; git -C D:\WorkSpace\RoboticDog status --porcelain=v1; git -C D:\WorkSpace\RoboticDog log -1 --format="%H %ad %s" --date=iso`（因 `git` 不在 PATH，全部失败，退出 1）。
   - 源码摘要核验：`$root='D:\WorkSpace\RoboticDog'; $bad=0; $ok=0; foreach($line in Get-Content "$root\validation\source-manifest.sha256"){ if($line -match '^([0-9a-f]{64})\s+(.+)$'){ $exp=$Matches[1]; $rel=$Matches[2].Trim(); $p=Join-Path $root ($rel -replace '/','\'); if(-not (Test-Path -LiteralPath $p)){ Write-Output "MISSING $rel"; $bad++ ; continue }; $act=(Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash.ToLower(); if($act -ne $exp){ Write-Output "MISMATCH $rel expected=$exp actual=$act"; $bad++ } else { $ok++ } } }; Write-Output "MATCH=$ok MISMATCH_OR_MISSING=$bad"`（含分号、循环）。
   - 多条探测命令使用 `| Select-Object` / `| Where-Object` 管道（探测方向见 §2.1）。
   - `$env:GIT_TERMINAL_PROMPT='0'; & 'C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\git\cmd\git.exe' -C 'D:\WorkSpace\RoboticDog' ls-remote --heads origin`。
2. 探测范围较宽（全盘定向搜索、注册表、目录枚举），虽均为只读，但超出“不自行探测路径”的收窄要求。
3. 配置失败后仍执行了构建与 CTest 尝试；虽属记录证据，但按主协调意见，配置失败后应停止依赖步骤，不重复空跑。

整改（已执行/将执行）：

- 自本报告起停止使用 PowerShell/Bash/Python 及其他代码执行工具；本轮只使用 Read/Glob/Grep/Write/Edit 整理文件。
- 原始日志全部保留、未重跑、未覆盖、未补写。
- 未来命令验证由主协调下发已核实的单条命令，文件编辑与命令验证分开；本会话不自行组合命令。
- 不再重复对失败目录执行构建/测试空跑。

---

## 8. 尚未完成、需要主协调判断的事项

1. **主机 C++17 编译器是否安装、安装哪一套**：由用户决定；本会话不下载、不安装、不改 PATH。安装后需重跑 Debug、Release、sanitizer 三套流程与五项测试。
2. **重跑命令的形式**：需主协调下发逐条已核实的原生命令（含生成器与编译器路径）；后续命令使用主协调已核实的 Git 路径 `C:\Program Files\Git\cmd\git.exe`。本会话不再自行拼装命令。
3. **sanitizer 口径**：主机工具链确定后，需确认 ASan/UBSan 实际可用性；LeakSanitizer 是否纳入、以及 MSVC 下的替代方案，需单独说明，不能以“配置成功”代替运行结果。
4. **远程同步**：本轮**未提交、未推送、未操作 GitHub**。只读 `ls-remote` 成功仅证明远程可读，**不证明写权限或推送认证可用**。同步状态：**SYNC_PENDING**，由主协调审查后处理。
5. **构建目录**：`build-m0-debug/`、`build-m0-release/`、`build-m0-asan/` 为失败配置残留（仅 CMakeCache/CMakeFiles，被 gitignore），**保留不删除**；工具到位后使用新的独立构建目录重跑，不复用这些失败目录。
6. **M0 验收判断**：本报告不批准任何里程碑。M0 是否通过、下一步是否进入 M1，由主协调核对并与用户确认。
7. **R01–R09**：保持已登记状态，本轮未修复、未验证、未改动相关代码。

---

## 9. 本轮文件变更清单

| 类型 | 路径 | 说明 |
|---|---|---|
| 新增 | `docs/m0/VSCODE_BASELINE_RESULT.zh-CN.md` | 本报告 |
| 新增 | `docs/m0/logs/vscode-baseline/01-tool-versions.txt` | 工具版本与环境 |
| 新增 | `docs/m0/logs/vscode-baseline/02-configure-debug.log` | Debug 配置失败 |
| 新增 | `docs/m0/logs/vscode-baseline/03-configure-release.log` | Release 配置失败 |
| 新增 | `docs/m0/logs/vscode-baseline/04-configure-sanitizers.log` | sanitizer 配置失败 |
| 新增 | `docs/m0/logs/vscode-baseline/05-build-debug.log` | Debug 构建失败 |
| 新增 | `docs/m0/logs/vscode-baseline/06-ctest-debug.log` | Debug CTest 0 测试 |
| 新增 | `docs/m0/logs/vscode-baseline/07-build-release.log` | Release 构建失败 |
| 新增 | `docs/m0/logs/vscode-baseline/08-ctest-release.log` | Release CTest 0 测试 |
| 新增 | `docs/m0/logs/vscode-baseline/09-build-sanitizers.log` | sanitizer 构建失败 |
| 新增 | `docs/m0/logs/vscode-baseline/10-ctest-sanitizers.log` | sanitizer CTest 0 测试 |
| 修改 | `docs/PROGRESS.zh-CN.md` | 补充本轮阻塞状态与报告链接 |
| 生成（gitignore） | `build-m0-debug/`、`build-m0-release/`、`build-m0-asan/` | 失败配置残留，未纳入版本记录；**保留不删除**，后续重跑使用新的独立目录 |
| 未改动 | `.vscode/settings.json` | 用户未提交文件，保持原样 |

**产品源码与测试代码：未改动。** `firmware/` 27 个文件摘要 27/27 与 `validation/source-manifest.sha256` 一致；`CMakeLists.txt`、`tools/test_host.sh`、`AGENTS.md`、`.vscode/`、全局配置均未修改。未提交、未推送、未合并、未创建分支。

---

## 10. 状态总表

| 检查项 | 状态 | 证据 |
|---|---|---|
| 源 HEAD/分支/工作树记录 | PASS | §1 |
| firmware 源码摘要 27/27 | PASS | §1 |
| 可用主机 C++17 编译器（本轮探测范围） | FAIL（未找到） | §2.1 |
| Debug 配置 | FAIL | §3.2，退出 1 |
| Release 配置 | FAIL | §3.3，退出 1 |
| sanitizer 配置 | FAIL | §3.4，退出 1 |
| Debug/Release/sanitizer 构建 | FAIL | §3.5，退出 1 |
| Debug 五项测试 | NOT_RUN | §3.6、§5 |
| Release 五项测试 | NOT_RUN | §3.6、§5 |
| sanitizer 五项测试 | NOT_RUN | §3.6、§5 |
| ASan/UBSan 实际运行 | NOT_RUN | §6 |
| LeakSanitizer | NOT_RUN | §6 |
| dry_run 无硬件运行 | NOT_RUN | §5 |
| M0 总门槛 | **BLOCKED** | 全部测试未执行 |
| M1 | 未开始 | — |
| 远程同步 | SYNC_PENDING | §8.4 |

配置 FAIL、构建尝试 FAIL、五项测试全部 NOT_RUN、总门槛 BLOCKED。CTest 零测试时工具虽报告成功，但不构成通过证据（数字退出码未单列，见 §3.6）。

**阻塞报告已整理，M0 基线未通过，M1 未开始。**
