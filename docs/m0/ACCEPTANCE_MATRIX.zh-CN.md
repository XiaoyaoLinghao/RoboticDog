# M0验收与后续覆盖映射

更新：2026-10-24（Asia/Shanghai）。PASS只表示表中列出的检查；不外推为实机能力。测试执行日为2026-10-23。

用户已接受当前sanitizer覆盖限制；相应测试状态仍NOT_RUN。当前等待讨论确认[M1方案](../M1_EXECUTION_PLAN.zh-CN.md)，未开始实现。

## M0当前证据

| 检查 | 状态 | 证据/说明 |
|---|---|---|
| 原始包完整性 | PASS | 46/46包清单匹配；原ZIP摘要见ENVIRONMENT |
| 原始firmware完整性 | PASS | 27/27源清单匹配；文档工作后再次核验一致 |
| GitHub记录 | PASS | 原始源码快照、项目任务/架构已同步；本地与远程源树核对一致 |
| Claude无操作与工具检查 | PASS | 2.1.308，PowerShell，既有deepseek-v4.1-flash[1m]；Get-Location受限调用成功 |
| 安装前配置/构建/CTest尝试（历史） | FAIL / NOT_RUN | 当时缺主机编译器、CTest零测试；保留environment-probe及VSCODE_BASELINE_RESULT/VSCODE_REVIEW，不改为PASS |
| 主机工具链就绪 | PASS | 用户授权安装；Build Tools18.11.1、MSVC19.51.36260.0、SDK10.0.26100.0；安装退出0 |
| MSVC Debug/Release配置与构建 | PASS | 原生工具均报告成功；数字退出码未单列，见msvc-baseline/commands.json及01/02/04/05日志 |
| Debug T01–T05 | PASS 5/5 | logs/msvc-baseline/03-ctest-debug.log；4个单元测试+dry_run |
| Release T01–T05 | PASS 5/5 | logs/msvc-baseline/06-ctest-release.log；/O2 /Ob2 /DNDEBUG，自定义CHECK有效 |
| sanitizer配置 | FAIL（能力限制） | 当前CMake拒绝MSVC sanitizer配置，退出1；见07-configure-sanitizer.log |
| ASan/UBSan | NOT_RUN | sanitizer配置失败后停止；未修改构建配置或安装其他工具链 |
| LeakSanitizer | NOT_RUN | 没有执行；历史Linux轮关闭泄漏检测 |
| 源码静态审查 | PASS | 表示18个实现/测试/构建文件已审阅并记录问题，不表示源码无缺陷；见SOURCE_REVIEW |
| 架构文档完整性 | PASS | 有职责、HAL/设备/输入适配、数据、状态、时间、开传/取消/恢复合同和迁移归属；仍待用户确认 |
| 主协调独立复核 | PASS_WITH_LIMITATIONS | 完整文档差异、原生事件、日志、缓存与27项摘要核对，见MSVC_REVIEW；R01–R09未修复 |
| M0总门槛 | AWAITING_USER | 基线PASS_WITH_LIMITATIONS；sanitizer限制已获接受，架构/M1方案仍待确认，开发未开始 |

本次测试由本地Claude CLI实际执行，VS Code会话仅整理报告，主协调独立审查。源HEAD为93a221a，firmware与原始清单一致。前6条命令没有独立数字退出码，不将工具成功推定写成0；实际CTest逐项及总计输出支持5/5结论。上述sanitizer配置FAIL不等于测试FAIL，也不等于sanitizer已通过。

## 后续验收归属

| 用例组 | 当前能力状态 | 实施归属 |
|---|---|---|
| T01–T05原有基线 | 本机MSVC Debug/Release各5/5 PASS | M0-03及每次回归 |
| F01–F06 | NOT_IMPLEMENTED（现有简单fake不等于F组完成） | M1-01 |
| D01–D14 | NOT_IMPLEMENTED | M1-02至05 |
| S01–S11 | NOT_IMPLEMENTED（底层已有部分机制，缺集成验收） | M2-01/02/05；S08的fake顺序先在M1-01覆盖 |
| C01–C07 | NOT_IMPLEMENTED（现有SafetyConfig检查不等于导入/版本合同） | M2-03/05 |
| C08拟合工具 | NOT_IMPLEMENTED，阶段外 | 另行安排 |
| L01–L05 | NOT_IMPLEMENTED | M2-04/05 |
| A01/A02/A05最小抽象合同 | NOT_IMPLEMENTED | M1-03/05 |
| A03/A04配置与输入适配合同 | NOT_IMPLEMENTED | M2-03/04/05；M1先做能力拒绝基础 |
| Q01–Q08 | NOT_IMPLEMENTED，阶段外 | 后续四腿/遥控 |
| B01默认host无真实I/O | 静态核对满足；MSVC Debug/Release dry_run均PASS | M0 dry_run及后续回归 |
| B02–B05板级构建/适配 | NOT_IMPLEMENTED，阶段外 | 证据齐备后单列任务 |
| B06–B07真实验证 | BLOCKED | 硬件、资料与准入尚未成立 |

原始验收条文见[ACCEPTANCE_TESTS.md](../../ACCEPTANCE_TESTS.md)，新增A组见[架构文档](../architecture/CONTROL_ARCHITECTURE.zh-CN.md)。未实现用例不能通过创建一个空测试或文档复选框记PASS。
