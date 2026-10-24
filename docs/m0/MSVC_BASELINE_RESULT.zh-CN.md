# MSVC 主机基线验证结果

测试日期：2026-10-23（Asia/Shanghai）。报告整理日期：2026-10-24。下文AWAITING_REVIEW与SYNC_PENDING为VS Code交付时点状态。

主协调后续注记（2026-10-24）：已完成[独立审查](MSVC_REVIEW.zh-CN.md)，接纳主机基线PASS_WITH_LIMITATIONS；M0当前为AWAITING_USER，等待用户讨论确认。远程同步以[当前进度](../PROGRESS.zh-CN.md)及GitHub提交记录为准。以下保留报告交付时口径。

**结论：主机 Debug/Release 基线已实际运行并通过——Debug 5/5、Release 5/5。** sanitizer 因项目当前 CMake 明确只接受 GCC/Clang 而在配置阶段退出 1，ASan/UBSan/LSan 均未运行。因此整体记为 **PASS_WITH_LIMITATIONS**，M0 总状态为 **AWAITING_REVIEW**，等待主协调独立审查与用户确认；本报告不自我批准里程碑。

本报告由 VS Code 会话仅整理既有证据，**未执行任何命令、测试、安装或 Git 操作**。测试由本地 Claude Code CLI 以原生 PowerShell 实际执行，证据见 `docs/m0/logs/msvc-baseline/`。

---

## 1. 源状态与工作树

以下账本由主协调提供；本会话未执行 `git` 命令，未独立复核。

| 项目 | 值 |
|---|---|
| 测试源 HEAD | `93a221a2baee2eb0ae8b58c0f636fddb8bf6455b` |
| 分支 | `main` |
| firmware 完整性 | 27/27 与 `validation/source-manifest.sha256` 匹配 |
| 产品/测试/构建代码 | 无改动 |
| 现有工作树改动 | `AGENTS.md`、`docs/VSCODE_HANDOFF.zh-CN.md` 修改；新增 `docs/m0/logs/msvc-baseline/` 日志 |
| 用户未跟踪文件 | `.vscode/`（保持原样，未修改、不上传） |

**工作树不是干净的。** 主协调此后还有文档改动；本会话只新增本报告并更新 `docs/PROGRESS.zh-CN.md`，不声称工作树清洁。

历史材料保留：`docs/m0/VSCODE_BASELINE_RESULT.zh-CN.md`、`docs/m0/logs/vscode-baseline/`（十份日志）作为上一环境“无主机编译器”的失败历史原样保留，不删除、不改写。

---

## 2. 安装与环境账本（主协调核实）

| 项目 | 值 | 来源 |
|---|---|---|
| 用户授权 | 明确授权安装 Visual Studio Build Tools、MSVC、Windows SDK；不扩展为其他软件 | 用户授权与 `00-environment.txt` |
| 引导程序 | `https://aka.ms/vs/stable/vs_buildtools.exe`；Authenticode 有效（Microsoft Corporation）；SHA256 `994DDAA3C0A5795AD700FAFBC6414C1ED5B580C32EB29BBE46F4E21388EB5DF3` | `00-environment.txt` |
| 安装参数 | `--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.26100` | 同上 |
| 安装退出码 | **0**；完成于 2026-10-23T22:35:16+08:00；无需重启 | 同上 |
| Build Tools | Visual Studio 2026 **18.11.1**（18.11.12315.225），stable | 同上 |
| 安装路径 | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools` | 同上 |
| vswhere 组件检查 | MSVC x64/x86 与 Windows11SDK.26100 存在；`isComplete=true`、`isLaunchable=true`、`isPrerelease=false`、`isRebootRequired=false` | 同上 |
| MSVC 编译器 | **19.51.36260.0** | 配置日志实际识别 |
| cl.exe 路径 | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\cl.exe` | 配置日志 |
| Windows SDK | **10.0.26100.0**（CMake 自动选择） | 配置日志 |
| SDK 头文件 | `C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0` | `00-environment.txt` |
| CMake / CTest | **4.3.1**，`C:\ST\STM32CubeCLT_1.22.0\CMake\bin` | 同上 |
| 生成器 | Visual Studio 18 2026，x64；Debug/Release 经 `--config` / `-C` 选择 | 同上 |
| Git | **2.56.0.windows.2**，`C:\Program Files\Git\cmd\git.exe` | 同上 |
| 测试执行者 | 本地 Claude Code CLI **2.1.308**，既有配置模型 `deepseek-v4.1-flash[1m]`；原生 PowerShell 工具 | 同上 |

说明：`00-environment.txt` 是主协调核实的安装/环境账本，不是安装器原始输出；原始 CLI 与安装器记录保留在被忽略的本地存储中，不随仓库发布。

---

## 3. 执行主体与权限方式

- **执行主体是本地 Claude Code CLI，不是本 VS Code 会话。** 本会话只读取证据并整理报告，不得被理解为执行过编译或测试。
- 启动参数仅在启动进程内设置：`CLAUDE_CODE_USE_POWERSHELL_TOOL=1`、`CLAUDE_CODE_POWERSHELL_RESPECT_EXECUTION_POLICY=1`；未覆盖显式禁用或继承的 Shell 拒绝规则，未更改全局权限、执行策略或模型/provider。
- 无操作初始化清单包含 PowerShell、Read；原生 `Get-Location` 冒烟调用返回 `D:\WorkSpace\RoboticDog`。验证阶段工具清单仅为 PowerShell，共七条精确单命令，未使用文件工具或其他 Shell。
- **两次 `dontAsk` 精确权限规则调用在第一条配置命令执行前即被拒绝并停止**，保留了调用记录。这是**权限拒绝，不是编译器失败**，也未产生编译或测试结果。具体规则匹配原因**未确定**，本报告不编造原因。
- 随后在原生 PowerShell 的交互手动模式下逐条执行：主协调逐条核对命令并选择**一次性 Yes**，从未选择“不再询问”的持久允许，未使用绕过参数，未更改全局工具权限。项目文件夹信任在交互启动时确认。
- 首次配置没有在 `dontAsk` 模式下真正执行；正式证据全部来自交互手动模式下的七条命令。

---

## 4. 实际执行的七条命令

工作目录（全部）：`D:\WorkSpace\RoboticDog`。命令原文取自 `docs/m0/logs/msvc-baseline/commands.json`。

### 4.1 Debug

| 步 | 命令（完整原文） | 证据 |
|---|---|---|
| 1 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S 'D:\WorkSpace\RoboticDog\firmware' -B 'D:\WorkSpace\RoboticDog\build-m0-msvc-debug' -G 'Visual Studio 18 2026' -A x64 '-DCMAKE_GENERATOR_INSTANCE=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools' -DROBOT_BUILD_HOST_TESTS=ON -DROBOT_ENABLE_SANITIZERS=OFF` | `01-configure-debug.log` |
| 2 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --build 'D:\WorkSpace\RoboticDog\build-m0-msvc-debug' --config Debug` | `02-build-debug.log` |
| 3 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --test-dir 'D:\WorkSpace\RoboticDog\build-m0-msvc-debug' -C Debug --output-on-failure -V --no-tests=error` | `03-ctest-debug.log` |

### 4.2 Release

| 步 | 命令（完整原文） | 证据 |
|---|---|---|
| 4 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S 'D:\WorkSpace\RoboticDog\firmware' -B 'D:\WorkSpace\RoboticDog\build-m0-msvc-release' -G 'Visual Studio 18 2026' -A x64 '-DCMAKE_GENERATOR_INSTANCE=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools' -DROBOT_BUILD_HOST_TESTS=ON -DROBOT_ENABLE_SANITIZERS=OFF` | `04-configure-release.log` |
| 5 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' --build 'D:\WorkSpace\RoboticDog\build-m0-msvc-release' --config Release` | `05-build-release.log` |
| 6 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\ctest.exe' --test-dir 'D:\WorkSpace\RoboticDog\build-m0-msvc-release' -C Release --output-on-failure -V --no-tests=error` | `06-ctest-release.log` |

### 4.3 sanitizer

| 步 | 命令（完整原文） | 证据 |
|---|---|---|
| 7 | `& 'C:\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe' -S 'D:\WorkSpace\RoboticDog\firmware' -B 'D:\WorkSpace\RoboticDog\build-m0-msvc-sanitizer' -G 'Visual Studio 18 2026' -A x64 '-DCMAKE_GENERATOR_INSTANCE=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools' -DROBOT_BUILD_HOST_TESTS=ON -DROBOT_ENABLE_SANITIZERS=ON` | `07-configure-sanitizer.log` |

### 4.4 退出码口径（不得推定）

- 第 1–6 条：工具结果 `is_error=false`；**原生工具记录中没有单独的数字退出码**，`commands.json` 对应 `numeric_exit_code` 为 `null`。本报告**不推定填写 0**。
- 第 7 条：工具明确报告 **Exit code 1**，`commands.json` 中 `numeric_exit_code` 为 `1`，`is_error=true`。
- 第 1–6 条的成功判定依据是各自日志的实质内容（配置完成、构建产物生成、CTest 明确 5/5），而不是退出码数值。

---

## 5. Debug 五项测试实际结果

日志：`03-ctest-debug.log`。执行命令含 `--no-tests=error`，即“零测试”会导致工具报错；本次未出现该情况，确有 5 个测试被找到并执行。

| # | 测试 | 结果 | 实际输出要点 | 用时 |
|---|---|---|---|---|
| 1 | `protocol` | **PASS** | `PASS protocol: golden bytes, signed reads, limits, resync, wrap, 4335 roundtrips, 200000 noise bytes` | 2.06 s |
| 2 | `bus` | **PASS** | `PASS bus: unbound disabled, async TX/RX, echo/unrelated rejection, partial RX, deadline wrap, cancellation, faults, no ACK assumption` | 0.79 s |
| 3 | `safety` | **PASS** | `PASS safety: fail-closed defaults, unique IDs, calibration/geometry/electrical gates, atomic plan, limits/NaN/speed/step, stale data, latched faults, watchdog/wrap` | 2.84 s |
| 4 | `kinematics` | **PASS** | `All 44176 kinematics checks passed` | 1.50 s |
| 5 | `dry_run` | **PASS** | `Host dry-run only. Transport ready: no. Configuration accepted: no.` / `No servo IDs, pin assignments, or mechanical limits are configured. No hardware I/O.` | 1.11 s |

汇总行：`100% tests passed, 0 tests failed out of 5`；`Total Test time (real) = 8.30 sec`。

说明：CTest 输出中的 `Added 0 tests to meet fixture requirements` 表示**没有为 fixture 依赖额外添加测试**，不是“总测试数为 0”。同一日志随后明确列出 5 个测试并统计 5/5 通过。

Debug 配置日志确认：`-- The CXX compiler identification is MSVC 19.51.36260.0`、Windows SDK 10.0.26100.0、`Configuring done` / `Generating done`；构建日志生成 `robot_core.lib`、`dry_run.exe`、`test_bus.exe`、`test_kinematics.exe`、`test_protocol.exe`、`test_safety.exe`。

---

## 6. Release 五项测试实际结果

日志：`06-ctest-release.log`。同样使用 `--no-tests=error`。

| # | 测试 | 结果 | 实际输出要点 | 用时 |
|---|---|---|---|---|
| 1 | `protocol` | **PASS** | 与 Debug 相同的 PASS 描述；4335 roundtrips、200000 noise bytes | 1.73 s |
| 2 | `bus` | **PASS** | 与 Debug 相同的 PASS 描述 | 0.93 s |
| 3 | `safety` | **PASS** | 与 Debug 相同的 PASS 描述 | 0.82 s |
| 4 | `kinematics` | **PASS** | `All 44176 kinematics checks passed` | 0.72 s |
| 5 | `dry_run` | **PASS** | `Transport ready: no. Configuration accepted: no.`；明确 `No hardware I/O.` | 0.93 s |

汇总行：`100% tests passed, 0 tests failed out of 5`；`Total Test time (real) = 5.14 sec`。

Debug 与 Release 两套 `kinematics` 各完成 **44176 checks**，与历史 Linux 报告的检查数量一致；这属于数量吻合，不代表历史报告的耗时或环境结果可替代本次运行。

两项配置均为 4 个单元测试（`protocol`、`bus`、`safety`、`kinematics`）加 `dry_run`，共 5 个测试。

---

## 7. sanitizer 实际执行范围与限制

| 项目 | 状态 | 证据/说明 |
|---|---|---|
| sanitizer 配置 | **FAIL**（Exit code 1） | `07-configure-sanitizer.log`：`CMake Error at CMakeLists.txt:16 (message): This sanitizer configuration requires GCC or Clang`，随后 `Configuring incomplete` |
| sanitizer 构建 | **NOT_RUN** | 配置失败后按约定停止，未继续 |
| sanitizer CTest | **NOT_RUN** | 同上 |
| ASan | **NOT_RUN** | 未编译、未链接、未运行 |
| UBSan | **NOT_RUN** | 同上 |
| LeakSanitizer | **NOT_RUN** | 未执行，支持性未验证 |

**限制的性质**：这是**项目当前 CMake 配置对 MSVC 的限制**，不是“MSVC 普遍没有 sanitizer”。`firmware/CMakeLists.txt` 的 sanitizer 分支只在 `CMAKE_CXX_COMPILER_ID` 匹配 GNU/Clang 时添加 `-fsanitize=address,undefined`，否则在 `ROBOT_ENABLE_SANITIZERS=ON` 时报 `FATAL_ERROR`。MSVC 的运行时检查（例如 ASan 的 `/fsanitize=address`）不在本项目当前配置范围内，本轮**未尝试**接入手工 MSVC sanitizer 参数，也未修改构建文件。

因此 sanitizer 覆盖在本轮为 **NOT_RUN**，不能以“配置按设计拒绝”推导为“sanitizer 已通过或不需要”。

---

## 8. 已知限制与未修复项

1. **R01–R09 全部仍未修复。** 本次只是原有源码的主机回归基线，未修改产品代码、测试代码或构建文件；源码审查中登记的执行链、时间戳、部分发送、HAL 合同、测试覆盖等问题保持原状，归属 M1/M2。
2. **R07（构建一致性）仍然存在。** 本次 MSVC 构建使用 MSVC 默认警告策略，`CMakeLists.txt` 的 `-Wall -Wextra -Wpedantic -Werror` 只对 GNU/Clang 生效；MSVC 路径没有对应的警告级别与“警告即错误”配置。本轮没有因警告失败，也**不能**据此声称 MSVC 警告覆盖已达标。
3. **sanitizer 覆盖缺口**：ASan/UBSan/LSan 均为 NOT_RUN，见 §7。
4. **测试环境差异**：本次是 Windows 11 + MSVC 19.51 主机结果，不能外推为 ARM 目标、真实硬件或实机时序验证。`dry_run` 明确 `No hardware I/O`，未打开串口、未连接设备、未烧录、未启用扭矩。
5. **退出码证据形式**：第 1–6 条没有单独数字退出码，成功判定依赖日志实质内容；这是证据形式限制，不改变 5/5 通过的事实。
6. **权限拒绝事件**：两次 `dontAsk` 调用在命令执行前被拒绝，规则匹配原因未确定；相关记录保留，不影响本次结果有效性。
7. **历史失败报告保留**：VS Code 环境“无主机编译器”的报告与十份日志作为历史证据原样保留，不覆盖、不删除；它们与本次结果属于不同时间点、不同工具状态。

---

## 9. 证据与日志索引

目录：`docs/m0/logs/msvc-baseline/`

| 文件 | 内容 | 对应步骤结果 |
|---|---|---|
| `00-environment.txt` | 主协调核实的安装与环境账本、权限执行方式、退出码口径 | — |
| `commands.json` | 七条命令原文、工作目录、`is_error`、`numeric_exit_code`、日志映射 | 1–7 |
| `01-configure-debug.log` | Debug 配置：MSVC 识别、SDK 选择、配置完成 | 1（工具报告成功，数字退出码未单列） |
| `02-build-debug.log` | Debug 构建：库与 5 个可执行文件生成 | 2（同上） |
| `03-ctest-debug.log` | Debug CTest：5/5 通过，8.30 s | 3（同上） |
| `04-configure-release.log` | Release 配置 | 4（同上） |
| `05-build-release.log` | Release 构建 | 5（同上） |
| `06-ctest-release.log` | Release CTest：5/5 通过，5.14 s | 6（同上） |
| `07-configure-sanitizer.log` | sanitizer 配置：Exit code 1，需要 GCC/Clang | 7（**Exit code 1**） |

日志不含凭据、令牌或完整会话转录；仅包含本机绝对路径与构建输出。

---

## 10. 本轮文件变更

| 类型 | 路径 | 说明 |
|---|---|---|
| 新增 | `docs/m0/MSVC_BASELINE_RESULT.zh-CN.md` | 本报告 |
| 修改 | `docs/PROGRESS.zh-CN.md` | 主机基线改为 PASS_WITH_LIMITATIONS；M0 总状态 AWAITING_REVIEW；补充报告链接 |

- 本会话**未修改** `firmware/`、测试代码、`CMakeLists.txt`、`tools/test_host.sh`、`AGENTS.md`、`.vscode/`、`docs/m0/logs/` 下任何日志，也未修改其他汇总/审查文件（由主协调并行更新）。
- 构建目录 `build-m0-msvc-debug/`、`build-m0-msvc-release/`、`build-m0-msvc-sanitizer/` 由测试执行者创建，属构建产物，被 `build*/` 忽略；本会话未删除、未改动。
- 未提交、未推送、未合并、未创建分支；Git/GitHub 操作由主协调负责。交付时点远程同步状态记为 **SYNC_PENDING**。

---

## 11. 状态总表

| 检查项 | 状态 | 证据 |
|---|---|---|
| 源版本记录（HEAD `93a221a`、main、firmware 27/27） | PASS（主协调提供） | §1、`00-environment.txt` |
| 工具安装（Build Tools 2026 / MSVC 19.51 / SDK 26100） | PASS（安装 exit 0） | §2、`00-environment.txt` |
| Debug 配置 | PASS（工具报告成功） | §4.1、`01-configure-debug.log` |
| Debug 构建 | PASS（工具报告成功，产物齐全） | §4.1、`02-build-debug.log` |
| Debug 五项测试 | **PASS 5/5** | §5、`03-ctest-debug.log` |
| Release 配置 | PASS（工具报告成功） | §4.2、`04-configure-release.log` |
| Release 构建 | PASS（工具报告成功，产物齐全） | §4.2、`05-build-release.log` |
| Release 五项测试 | **PASS 5/5** | §6、`06-ctest-release.log` |
| sanitizer 配置 | FAIL（Exit code 1，项目 CMake 限定 GCC/Clang） | §7 |
| ASan / UBSan / LeakSanitizer | NOT_RUN | §7 |
| 主机基线总体 | **PASS_WITH_LIMITATIONS** | Debug/Release 5/5；sanitizer 未覆盖 |
| M0 总状态 | **AWAITING_REVIEW** | 等待主协调独立审查与用户确认 |
| M1 / M2 | 未开始 | — |
| 远程同步 | SYNC_PENDING（交付时点） | 由主协调处理 |

**主机 Debug/Release 基线 5/5 通过（PASS_WITH_LIMITATIONS）；sanitizer 未运行；M0 待主协调审查与用户确认；M1 未开始。**
