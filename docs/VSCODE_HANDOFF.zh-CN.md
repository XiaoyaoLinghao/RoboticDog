# VS Code接手说明

日期：2026-10-23（Asia/Shanghai）。用途：交给用户在VS Code中使用的AI。用户已指定代码修改、调试、编译及后续烧录由该环境接手；本会话负责方案与独立审查。当前只交接M0基线验证，M1仍待用户讨论确认。

同日后续授权：用户已允许主协调安装Visual Studio Build Tools、MSVC及Windows SDK，再继续M0。委派方仍不得自行安装其他工具。按更新后的全局约定，精确测试命令通过本地Claude Code CLI执行，VS Code会话只用文件工具整理回传；主协调核对实际事件和日志。首次无操作库存检查及无害命令验证必须确认原生PowerShell，启动开关仅设于当次进程，不修改全局配置。

2026-10-24收口更新：安装与主机测试已完成，Debug/Release各5/5 PASS；sanitizer为项目当前MSVC配置限制，未执行。最新结论见[M0总报告](m0/M0_REPORT.zh-CN.md)和[独立审查](m0/MSVC_REVIEW.zh-CN.md)。下文测试步骤保留为可追溯接手规格，不是重新执行指令；当前应停止在M0，等待用户讨论确认。

## 1. 原始源码与阅读入口

- 本机项目：`D:\WorkSpace\RoboticDog`；原始C++17源码：`firmware/`，共27个文件，至本轮移交均未修改。
- 远程仓库：https://github.com/XiaoyaoLinghao/RoboticDog 。从最新main读取当前任务/架构；不要用旧历史报告替代复验。
- 原始导入提交：`acfba31aa167bea6a3a7e3e9a8b709bb4b7541b9`。本轮只读复核/配置探测基线：`48950e156baa039c0aa6e653e29063ad1a5a7403`。两版本的firmware相同；后续新增文档提交不改变此事实。
- `validation/source-manifest.sha256`记录原始firmware摘要；原始交接ZIP仍留本机。源码目录可直接用VS Code打开；Git仓库保留来源与文档历史。
- 先读项目[AGENTS](../AGENTS.md)、[路线任务](ROADMAP.zh-CN.md)、[架构](architecture/CONTROL_ARCHITECTURE.zh-CN.md)、[M0报告](m0/M0_REPORT.zh-CN.md)、[源码审查](m0/SOURCE_REVIEW.zh-CN.md)及[原始验收](../ACCEPTANCE_TESTS.md)。

## 2. 本次先做的M0任务

1. 记录接手源HEAD、分支、工作目录和已有改动；不得覆盖用户未提交工作。有代码修改需求时使用独立任务工作树，给出准确基线。
2. 核实可在当前主机运行的C++17编译器、CMake、构建器和运行库。VS Code是工作入口，具体编译工具仍需核实；`arm-none-eabi-g++`不能运行Windows主机测试。现有工具位置见[环境记录](m0/ENVIRONMENT.zh-CN.md)。不因接手而自动安装工具或改全局设置。
3. 不修改原始源码，先配置、编译并运行Debug与Release全部五项CTest：protocol、bus、safety、kinematics、dry_run。保留完整输出、准确命令、目录、退出码、工具版本和测试源SHA。
4. 用工具链实际支持的ASan/UBSan配置运行同样测试。若不支持，提供具体诊断与限制，不把sanitizer选项开启或配置成功当作运行通过；LeakSanitizer单独说明。
5. 如基线编译/运行失败，先保存原始失败证据并区分环境故障和源码缺陷。提出最小修复范围，必要修复作为单独有边界任务和完整diff交回审查；不得删除测试、放宽检查或顺带实现M1。
6. 回传证据，等待本会话独立审查并由用户确认M0/架构后再启动M1。当前没有可烧录的完整板级工程，不执行任何串口、烧录或运动操作。

### CMake验证模板（未执行）

以下在接手任务工作目录及已就绪的主机编译环境中使用，每行独立执行。接手AI须先把cmake/ctest解析为已核实的路径，记录选用的生成器与编译器；不能把模板记作实际命令。若环境不能自动选定正确主机编译器，在配置命令中显式指定经核实的生成器/编译器，不猜路径。

```powershell
cmake -S firmware -B build-m0-debug -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=OFF -DROBOT_BUILD_HOST_TESTS=ON
cmake --build build-m0-debug --config Debug
ctest --test-dir build-m0-debug -C Debug --output-on-failure -V
cmake -S firmware -B build-m0-release -DCMAKE_BUILD_TYPE=Release -DROBOT_ENABLE_SANITIZERS=OFF -DROBOT_BUILD_HOST_TESTS=ON
cmake --build build-m0-release --config Release
ctest --test-dir build-m0-release -C Release --output-on-failure -V
```

这两套目录分别用于Debug/Release；多配置生成器以`--config`/`-C`选择配置，单配置生成器以CMAKE_BUILD_TYPE选择。sanitizer使用第三个独立构建目录并设置ROBOT_ENABLE_SANITIZERS=ON，只在实际支持的GNU/Clang主机工具链上运行。原始`firmware/tools/test_host.sh`是Bash入口，不直接当作原生PowerShell脚本执行。

## 3. 后续修改建议（当前先阅读，不实施）

| 次序/归属 | 建议与交付 | 验收重点 |
|---|---|---|
| M1-01，R06/R08/R09 | 共用脚本化fake与时钟；记录事件时间、线路完成、缓冲借用；补原测试空缺 | 延迟读取与真实字节超时区分；完整帧/有效stop/非法帧/异常反馈；无真实I/O |
| M1-02，R01/R02 | 分离目标接受与纯校验，加入来源、会话、序号、配置代际 | 重复plan不续看门狗，撤销后缓存不能复活，回绕/未来时间与重配有回归 |
| M1-03，R01/R05 | ControlPort→Dispatcher→唯一Scheduler→ActuatorBackend；LX-16A后端内部使用ServoBus | 应用接口不暴露原始帧；每帧开传前重新检查；设备量化后再校验实际关节值 |
| M1-04，R03 | 逐关节执行状态、部分失败、撤销、受限stop与恢复 | 已上线路不宣称回滚；后续move停止；无自动unload/重试；WriteSent不冒充ACK |
| M1-05，R07/R09 | 统一警告策略并完成集成回归及独立diff审查 | Debug/Release、原五项与F/D/A验收；明确编译器差异，不删除检查 |
| M1合同→M2落实，R04/R08 | 明确HAL RX错误/溢出、时间依据、取消失败与幂等释放；接入遥测调度 | 错误不能当无数据；到达时间不能用poll时间替代；真实板级性质留待B组验证 |
| M2，R05及架构约定 | 配置能力/设备绑定、输入规范化、日志与fake回放 | 换设备使旧验证失效；缺能力拒绝；配置原子更新；旧输入不续命；回放无真实输出 |

R编号的代码位置、触发条件、影响与第二意见裁决详见源码审查。0ms/unload/codec广播是现有低层能力，不在M0擅自改协议；控制访问边界在M1收口。`start(false)`也不得清除正在执行事务的状态。

架构中的平台HAL、执行器设备适配、输入协议适配是三个边界。当前只规划LX-16A位置模式和离线输入；不为证明抽象而新增未知电机、真实遥控、力矩控制或动态插件系统。

## 4. 回传格式与GitHub记录

请写入`docs/m0/VSCODE_BASELINE_RESULT.zh-CN.md`并附精简且完整的测试日志（不得含凭据）：

1. 源HEAD、分支、工作树状态、firmware摘要是否匹配；如有改动，列出全部文件与完整diff所在提交/分支。
2. OS、编译器/CMake/生成器版本及实际路径；实际命令、工作目录、退出码。
3. Debug、Release分别列五项测试结果；sanitizer实际执行项、结果与限制；不可只截取PASS行。
4. 失败原因、最小修复建议/已授权修复和复验结果；仍未运行与未实现项分开列出。
5. 明确“未开始M1、无硬件I/O”；不得由实现者自行批准里程碑。

有实质进展时更新[M0 Issue #1](https://github.com/XiaoyaoLinghao/RoboticDog/issues/1)及[PROGRESS](PROGRESS.zh-CN.md)。未审查代码保留任务分支，不直接合入main；本会话核对后更新总报告。不得上传工具链、ZIP、构建目录、个人配置、凭据或长篇初始化日志。

## 5. 可直接交给接手AI的任务

> 请阅读本项目AGENTS.md及docs/VSCODE_HANDOFF.zh-CN.md，先执行M0原始主机基线验证。核对源版本与已有改动，确认实际主机工具链，不安装工具、不连接硬件。原始firmware先不修改，运行Debug/Release五项测试及工具链支持的sanitizer，保存完整命令、退出码和日志。遇到失败先记录原因与最小修复范围，不绕过检查。按接手文档回传结果和完整diff；R01–R09与架构建议先阅读，不实施M1，等待用户在主协调会话讨论确认。
