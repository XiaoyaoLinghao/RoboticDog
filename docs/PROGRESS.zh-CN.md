# 当前开发进度

## 2026-10-25 打印准备补充

用户确认使用 PETG。已整理 [P1S 单腿 3D 打印说明](3D_PRINTING_GUIDE.zh-CN.md)，包含固定版本下载、11 件清单、PETG 切片起始参数、逐件候选方向、分批顺序、断电试配和记录模板。温度参考核对 Bambu Studio 官方 Generic PETG 继承配置；具体耗材品牌、打印板与喷嘴仍待实物核对。工艺参数尚未试印，髋主动齿轮版本和轴承配合仍按原清单保持待确认。

本次为文档工作：本地生成 12 页 PDF 并逐页检查排版，Markdown 作为仓库维护版本。未安装工具，未操作打印机或机器人，未修改产品代码或开展 M1 实现及测试。

## 软件状态沿用 2026-10-24

更新时间：2026-10-24（Asia/Shanghai）。当前：M0 **AWAITING_USER**。Debug 5/5、Release 5/5 PASS；sanitizer因项目当前CMake仅接受GCC/Clang而配置退出1，ASan/UBSan/LSan NOT_RUN；主机基线为 **PASS_WITH_LIMITATIONS**。主协调已完成[独立审查](m0/MSVC_REVIEW.zh-CN.md)，等待用户讨论确认。M1/M2未开始；硬件未到，真实输出禁用。安装由用户授权、主协调执行并核实；测试由本地Claude Code CLI执行，VS Code会话仅整理报告并已停止等待。

同日最新用户决定：**已接受当前sanitizer覆盖限制**，不要求现在补装工具或修改构建。当前待确认事项为M1执行链与HAL方案，M0架构细化随该方案一并讨论；不把接受覆盖限制解释为批准M1开发。M1状态 **DESIGN_REVIEW / AWAITING_USER**，全部实现任务仍TODO。

| 任务 | 工作状态 | 证据/下一步 |
|---|---|---|
| M0-01 | DONE | 原ZIP SHA-256 `c66b9031f816d57d3b5d88a4cb2dcc8b55a274f9368d2839e8eeed1d7d91dc2f`；46/46包文件、27/27源文件匹配；原始快照已同步 |
| M0-02 | DONE | 用户授权后已安装并核实：Build Tools 2026 18.11.1、MSVC 19.51.36260.0、Windows SDK 10.0.26100.0，安装 exit 0、无需重启；CMake/CTest 4.3.1、Git 2.56.0.windows.2。测试执行者为本地 Claude Code CLI（原生 PowerShell），VS Code 会话仅整理报告；见[MSVC基线报告](m0/MSVC_BASELINE_RESULT.zh-CN.md) |
| M0-03 | PASS_WITH_LIMITATIONS | MSVC 主机基线实际运行：Debug 5/5 PASS（8.30 s）、Release 5/5 PASS（5.14 s），4 个单元测试加 dry_run，两套 kinematics 各 44176 checks，dry_run 明确无硬件 I/O；sanitizer 配置 Exit code 1（项目 CMake 限定 GCC/Clang），ASan/UBSan/LSan NOT_RUN。前 6 条命令仅 `is_error=false`、数字退出码未单列，不推定填写 0；见[MSVC基线报告](m0/MSVC_BASELINE_RESULT.zh-CN.md)与`m0/logs/msvc-baseline/` |
| M0-04 | DONE | 主协调完整静态审查及Claude只读第二意见核对完成，9项问题/合同缺口已归属后续任务；见[M0源码审查](m0/SOURCE_REVIEW.zh-CN.md) |
| M0-05 | AWAITING_USER | 架构候选已完成，包含平台HAL、设备、输入协议及状态/数据合同；主机基线已通过（含 sanitizer 限制），待与用户在 M0 审查结论中一起讨论确认 |

M0基线、源码审查、架构候选和独立审查已齐备，总状态AWAITING_USER。讨论确认前不开始M1。新增模块仍未实现，原firmware 27/27摘要一致。

M1候选方案已完成：[执行链、HAL、顺序/交付物/验收及R01–R09处置](M1_EXECUTION_PLAN.zh-CN.md)。建议顺序01a测试基建→01b最小HAL/总线→02通用策略与身份→03逐帧执行→04故障/stop→05全量验收。R04/R08最小合同前移，真实驱动和完整遥测仍留后续。本轮仅文档与只读设计复核，没有产品修改或新增测试运行。

记录入口：[M0报告](m0/M0_REPORT.zh-CN.md)、[验收矩阵](m0/ACCEPTANCE_MATRIX.zh-CN.md)、[MSVC 主机基线报告](m0/MSVC_BASELINE_RESULT.zh-CN.md)（日志在 `m0/logs/msvc-baseline/`）、[VS Code 基线阻塞报告](m0/VSCODE_BASELINE_RESULT.zh-CN.md)（历史，日志在 `m0/logs/vscode-baseline/`）、[独立审查与工具建议](m0/VSCODE_REVIEW.zh-CN.md)。GitHub待办：[M0 #1](https://github.com/XiaoyaoLinghao/RoboticDog/issues/1)、[M1 #2](https://github.com/XiaoyaoLinghao/RoboticDog/issues/2)、[M2 #3](https://github.com/XiaoyaoLinghao/RoboticDog/issues/3)。报告与证据由主协调审查后纳入本次版本记录；MSVC 报告的 SYNC_PENDING 为交付时点状态，远程版本以 GitHub 提交记录为准。

接手入口：[VS Code接手说明](VSCODE_HANDOFF.zh-CN.md)。主机基线已完成并经主协调审查，M0待用户确认。R01–R09仍未修复，为已登记建议，不代表已授权开始M1实现。报告交付时的SYNC_PENDING已与独立审查区分，最新同步提交见[M0进度Issue](https://github.com/XiaoyaoLinghao/RoboticDog/issues/1)。
