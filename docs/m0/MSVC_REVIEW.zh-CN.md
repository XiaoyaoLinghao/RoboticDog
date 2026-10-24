# M0 MSVC基线独立审查

日期：2026-10-24（Asia/Shanghai）。审查人：主协调；测试执行者：本地Claude Code CLI；报告整理者：VS Code Claude Code会话。主协调未修改产品、测试或构建代码。

**结论：接纳Debug/Release各5/5通过的基线证据，主机基线为PASS_WITH_LIMITATIONS。M0材料已齐备，总状态AWAITING_USER；用户尚未确认M0验收或允许进入M1。** sanitizer配置失败、ASan/UBSan/LSan未运行；R01–R09保持未修复。

## 审查范围与源状态

- 测试源HEAD：`93a221a2baee2eb0ae8b58c0f636fddb8bf6455b`，分支main；记录本轮文档的后续提交不是另一份产品源码。
- 主协调重新逐项核验 `validation/source-manifest.sha256`：27/27匹配、0差异。与原始导入 `acfba31aa167bea6a3a7e3e9a8b709bb4b7541b9` 比较，firmware完整差异为空。
- 审阅VS Code新增报告和PROGRESS完整差异；审阅本轮AGENTS、README、接手说明、环境、验收矩阵、源码审查更新和M0总报告，以及全部9份新增证据文件。历史失败报告与10份日志保持原样。
- `.vscode/settings.json`是已有用户未跟踪文件，保持原样，不纳入本次发布；`.local/`、工具链、安装器、构建目录和完整会话记录不发布。

## 执行证据核对

1. 对照本地CLI持久记录的7对原生PowerShell `tool_use` / `tool_result`，与[commands.json](logs/msvc-baseline/commands.json)逐条比较：命令7/7完全一致，错误标记7/7一致；输出在统一换行和忽略末尾换行后7/7一致。没有将文本模拟调用、进程成功或未执行步骤当作测试证据。
2. Debug和Release的CTest日志均明确列出protocol、bus、safety、kinematics、dry_run共5项且100%通过；两个构建目录中的 `Testing/Temporary/LastTest.log` 各有5项Test Passed，与发布日志一致。`Added 0 tests to meet fixture requirements`只是没有添加fixture依赖测试。
3. 两套kinematics各44176项检查通过；protocol各4335次roundtrip、200000字节noise。dry_run明确Transport未就绪、配置不接受、No hardware I/O。
4. Release缓存为 `/O2 /Ob2 /DNDEBUG`，生成器Visual Studio 18 2026、x64、绑定已核实Build Tools实例。`test_util.hpp`使用自定义CHECK及std::exit，不因NDEBUG失效。
5. 第1–6条工具记录均 `is_error=false` 且日志有实际成功结果，但没有单列数字退出码，账本保留null；第7条明确Exit code 1。未将推定0写为实采退出码。
6. 第7条sanitizer配置在CMakeLists.txt:16主动拒绝当前MSVC路径；随后没有构建或测试。该项目当前组合sanitizer接入限于GNU/Clang，不能推导成MSVC普遍无sanitizer或sanitizer通过。
7. VS Code报告整理轮实际工具记录仅Read、Glob、Write、Edit，写入仅MSVC_BASELINE_RESULT和PROGRESS两个文档；未执行Shell、测试、安装、Git或产品修改，最终已停止等待审查。报告中的AWAITING_REVIEW/SYNC_PENDING保留为交付时点状态，当前以本审查和PROGRESS为准。

完整测试命令和输出链接见[MSVC基线报告](MSVC_BASELINE_RESULT.zh-CN.md)。安装环境和签名/组件核对见[环境记录](ENVIRONMENT.zh-CN.md)及[安装核验账本](logs/msvc-baseline/00-environment.txt)。原始CLI会话保留本地供追溯，未上传整段转录。

## 权限与历史失败的处理

两次dontAsk调用在第一条配置执行前遭拒绝，均立即停止，不产生编译结果；精确允许规则未匹配的原因尚未确定。之后由主协调使用原生PowerShell交互手动模式，核对每条命令并单次批准；没有持久允许、权限绕过参数或全局权限更改。原生注册开关仅设置在启动进程中，模型/provider保持既有配置。安装经过用户明确授权，安装器退出0、vswhere核实完整且无需重启。

早期缺少编译器导致的配置失败、无生成文件导致的构建失败和零测试结果继续保留；本次通过不改写当时事实。本轮仅复核既有测试证据，不为补数字退出码重复执行已通过的测试。

## 限制与验收建议

- 接纳范围是指定Windows/MSVC主机环境的原有五项离线测试；不包括sanitizer、覆盖率、ARM完整链接、HAL实机、机械或停止能力验证。
- 本轮构建输出未见警告诊断，但当前MSVC警告级别和“警告即错误”策略未统一，R07仍成立；不声称通过严格警告审查。
- [源码审查R01–R09](SOURCE_REVIEW.zh-CN.md)仍有效，尤其是旧计划撤销/开传复核、重复目标看门狗、部分发送和接收时间戳等缺口。现有基线通过不能替代这些尚未实现的验收场景。
- [三层适配架构](../architecture/CONTROL_ARCHITECTURE.zh-CN.md)是候选合同；平台HAL、执行器设备、输入协议的新增实现尚未开始。用户确认M0及架构细化方案后，再安排有边界的M1任务。

建议接受“已复现、限制明确”的M0基线作为讨论材料，保留sanitizer覆盖缺口，讨论确认后决定是否进入M1。当前不会自动推进。
