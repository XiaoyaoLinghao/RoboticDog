# M0验收与后续覆盖映射

日期：2026-10-23（Asia/Shanghai）。PASS只表示表中列出的检查；不外推为实机能力。

## M0当前证据

| 检查 | 状态 | 证据/说明 |
|---|---|---|
| 原始包完整性 | PASS | 46/46包清单匹配；原ZIP摘要见ENVIRONMENT |
| 原始firmware完整性 | PASS | 27/27源清单匹配；文档工作后再次核验一致 |
| GitHub记录 | PASS | 原始源码快照、项目任务/架构已同步；本地与远程源树核对一致 |
| Claude无操作与工具检查 | PASS | 2.1.308，PowerShell，既有deepseek-v4.1-flash[1m]；Get-Location受限调用成功 |
| 本机CMake配置探测 | FAIL | 找不到CMAKE_CXX_COMPILER，退出1，见logs/environment-probe.txt |
| VS Code配置与构建复验尝试 | FAIL | 三套配置与后续构建均失败；见VSCODE_BASELINE_RESULT与独立审查VSCODE_REVIEW |
| VS Code三套CTest调用 | NOT_RUN（测试用例） | 工具调用完成但均为零测试；protocol/bus/safety/kinematics/dry_run全部未运行，不能记PASS |
| Debug T01–T05 | NOT_RUN | 配置未完成，无主机编译器 |
| Release T01–T05 | NOT_RUN | 同上；不能用O2历史报告替代 |
| ASan/UBSan | NOT_RUN | 同上，支持能力尚未确认 |
| LeakSanitizer | NOT_RUN | 没有执行；历史Linux轮关闭泄漏检测 |
| 源码静态审查 | PASS | 表示18个实现/测试/构建文件已审阅并记录问题，不表示源码无缺陷；见SOURCE_REVIEW |
| 架构文档完整性 | PASS | 有职责、HAL/设备/输入适配、数据、状态、时间、开传/取消/恢复合同和迁移归属；仍待用户确认 |
| M0总门槛 | BLOCKED | 主机Debug/Release基线尚未运行；不得进入M1 |

用户最新指定由VS Code中的相应AI执行编译/调试，本轮不安装工具链。上述FAIL保留为本会话实际探测证据；新环境的测试结果需另外记录源版本、命令和日志，经审查后再更新总门槛。移交不等于测试通过。

## 后续验收归属

| 用例组 | 当前能力状态 | 实施归属 |
|---|---|---|
| T01–T05原有基线 | 已有测试；本机NOT_RUN | M0-03及每次回归 |
| F01–F06 | NOT_IMPLEMENTED（现有简单fake不等于F组完成） | M1-01 |
| D01–D14 | NOT_IMPLEMENTED | M1-02至05 |
| S01–S11 | NOT_IMPLEMENTED（底层已有部分机制，缺集成验收） | M2-01/02/05；S08的fake顺序先在M1-01覆盖 |
| C01–C07 | NOT_IMPLEMENTED（现有SafetyConfig检查不等于导入/版本合同） | M2-03/05 |
| C08拟合工具 | NOT_IMPLEMENTED，阶段外 | 另行安排 |
| L01–L05 | NOT_IMPLEMENTED | M2-04/05 |
| A01/A02/A05最小抽象合同 | NOT_IMPLEMENTED | M1-03/05 |
| A03/A04配置与输入适配合同 | NOT_IMPLEMENTED | M2-03/04/05；M1先做能力拒绝基础 |
| Q01–Q08 | NOT_IMPLEMENTED，阶段外 | 后续四腿/遥控 |
| B01默认host无真实I/O | 静态核对满足；本机运行NOT_RUN | M0 dry_run及后续回归 |
| B02–B05板级构建/适配 | NOT_IMPLEMENTED，阶段外 | 证据齐备后单列任务 |
| B06–B07真实验证 | BLOCKED | 硬件、资料与准入尚未成立 |

原始验收条文见[ACCEPTANCE_TESTS.md](../../ACCEPTANCE_TESTS.md)，新增A组见[架构文档](../architecture/CONTROL_ARCHITECTURE.zh-CN.md)。未实现用例不能通过创建一个空测试或文档复选框记PASS。
