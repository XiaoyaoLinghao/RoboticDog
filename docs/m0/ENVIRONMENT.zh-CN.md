# M0环境与来源记录

日期：2026-10-23（Asia/Shanghai）。操作系统报告：Microsoft Windows 10.0.26200，X64；PowerShell 7.6.5。

## 来源与版本

- 原包：`codex-robot-handoff-2026-10-23 (1).zip`，111903字节，留在本地、不提交。
- ZIP SHA-256：`c66b9031f816d57d3b5d88a4cb2dcc8b55a274f9368d2839e8eeed1d7d91dc2f`。
- 解压后首次核验：`validation/package-manifest.sha256` 46/46匹配；`validation/source-manifest.sha256` 27/27匹配。
- 原始导入远程提交：`acfba31aa167bea6a3a7e3e9a8b709bb4b7541b9`，树 `683efefc40ddc6ad973a0a5e5846eb64c94472b9`。
- 本地最初导入提交 `4a1af21` 与上述远程提交树完全相同，保留于本地 `archive/local-handoff-import` 分支。连接器生成提交元数据与本地不同，不伪造两个提交SHA相同。
- M0初始工作基线：`48950e156baa039c0aa6e653e29063ad1a5a7403`，包含已确认任务和架构草案；firmware与原包相同。
- 主目录：`D:\WorkSpace\RoboticDog`。
- 独立M0验证/复核工作树：`C:\Users\zhy76\.codex\worktrees\m0-baseline-review\RoboticDog`，基线为上述48950e1。

原始manifest核验对象是原始快照。根目录AGENTS/.gitignore等项目文档后续有经授权修改，不能再用原始包manifest判定整个当前仓库；firmware源manifest仍可用于确认原始源码未改。

## 已核实工具

| 工具 | 实际位置 | 结果 |
|---|---|---|
| Git | `C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\git\cmd\git.exe` | 2.53.0.windows.3 |
| Claude CLI | `C:\Users\zhy76\.local\bin\claude.exe` | 2.1.308；无操作调用SMOKE_OK；受限Get-Location成功 |
| 委派Shell | `C:\Users\zhy76\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\powershell\pwsh.exe` | 初始化元数据确认PowerShell，而非Bash |
| CMake/CTest | `C:\ST\STM32CubeCLT_1.22.0\CMake\bin` | CMake 4.3.1 |
| Ninja | `C:\ST\STM32CubeCLT_1.22.0\Ninja\bin\ninja.exe` | 1.13.2 |
| ARM C++ | `C:\ST\STM32CubeCLT_1.22.0\GNU-tools-for-STM32\bin\arm-none-eabi-g++.exe` | 14.3.1；仅确认版本，不能执行Windows主机测试 |
| Windows主机C++ | PATH、常见安装目录、用户/项目工具目录未找到g++/clang++/cl/clang-cl | 当前BLOCKED |
| WSL | `C:\Windows\system32\wsl.exe --list --quiet` | 退出1，提示需要安装；不作为已有可用Linux测试环境 |

Claude初始化model为 `deepseek-v4.1-flash[1m]`；不传模型覆盖参数，不改变provider/auth；调用成功验证认证可用。CLI输出过unrecognized_model提示，但实际请求成功，不能将其视为认证失败，也不能把输出改称其他厂商模型。未读取或保存凭据。

无操作调用的工具清单包含PowerShell/Read/Glob/Grep等；后续调用按任务收窄。PowerShell smoke仅执行Get-Location且返回正确工作目录；无permission denial。只读复核仅允许Read/Glob/Grep；配置探测仅允许一条完整CMake命令。

## 主机编译器探测及用户后续安排

首次通过Claude执行的CMake配置退出1：`No CMAKE_CXX_COMPILER could be found.`，完整命令与输出见 [environment-probe.txt](logs/environment-probe.txt)。编译和测试均未运行，不能把历史Linux报告记成本机通过。

曾向用户提出从[LLVM-MinGW官方仓库20261020发布](https://github.com/mstorsjo/llvm-mingw/releases/tag/20261020)下载 `llvm-mingw-20261020-ucrt-x86_64.zip`（190612575字节），校验后仅解压到本项目。用户随后指定VS Code中的相应AI承担代码修改、调试、编译和后续烧录。因此本方案不执行，本轮无下载/安装，转交[VS Code接手流程](../VSCODE_HANDOFF.zh-CN.md)。

留档的发布方SHA-256：`fd5462b7f5d93b2ff981fcd3f9c7e427e47f3203c64ea91c27bcc444eebd74cd`。此记录不表示已安装或测试过该工具链。接手方使用其经核实的主机编译环境并记录sanitizer实际支持。

## GitHub同步路径

2026-10-23用户完成Git安装后，主协调核实`C:\Program Files\Git\cmd\git.exe`为`2.56.0.windows.2`，后续优先显式使用此路径。已有进程仍可能解析到旧运行时Git；不更改历史日志中的2.53.0版本。VS Code插件的复验失败证据与后续工具建议见[VSCODE_REVIEW](VSCODE_REVIEW.zh-CN.md)。

仓库metadata确认main为空起点且连接器有写权限。最初README通过连接器创建，使用其已认证作者信息为本仓库设置local Git身份（不改global）。

Git CLI推送在关闭交互的情况下失败：`could not read Username for https://github.com: terminal prompts disabled`。未复制token或改凭据。后续通过已连接GitHub工具创建树/提交并以预期旧HEAD、非强制方式更新main，再fetch核对本地暂存树一致，保留本地旧提交后soft对齐。GitHub同步已成功，不将CLI认证失败冒充整个连接器不可用。
