# RoboDog 本地 Codex 交接：来源、固定版本与复用边界

核查日期：**2026-10-23 UTC**。GitHub 分支头、完整递归目录树、根 LICENSE 和以下重点文件在本次核查中实际读取；仓库快照核验约为 11:14–11:17 UTC。网页没有固定提交，以访问日期为准。

本文件只给出可追溯的资料和移植线索。**没有下载整仓、Raspberry Pi 系统镜像、APK、模型权重或 STL；没有安装、编译或运行第三方项目，没有连接、扫描、写入、烧录或驱动硬件。** 文件树验证证明路径存在，不等于实物、网格尺寸、构建或运行通过验证。

补充阅读：本文件记录来源审计阶段的核验范围。随后完成的 [PRINT_INVENTORY.zh-CN.md](PRINT_INVENTORY.zh-CN.md) 另行核查了装配文字及部分 STL 几何；打印数量、V2 髋壳体、第三髋齿轮候选和轴承冲突请以该更具体的核验为准。这里的 V1“底座”指需分辨的机身基础，不表示给 V2 单腿额外加 V1 髋壳体。

## 1. 先看结论

1. 当前机械路线的主来源是 **RoboDog_v2 + RoboDog_v1.0 的底座**。V2 README 明确说底座沿用 V1.0，因此只看 V2 的 STL 目录会漏掉机身基础零件。依据：[V2 原作者说明](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/README.md)。
2. V2 与 V3 的控制软件是 **Linux/ROS/catkin 主机程序**。V3 支持 V2/V3 两种机器人配置，但并不是 STM32F407 可直接烧录固件，也不是“把 ROS 的 main 改名就能移植”。当前原创单腿工程需要独立的 F407 BSP、UART/半双工适配、安全状态机和实物标定。
3. **上游程序不能作为只读诊断程序直接启动。** V2 默认入口会执行调姿；V3 的机器人初始化会写舵机偏移和保护阈值。具体证据在第 4 节。
4. 上游 V2 映射含 **2:1 传动比例、左右方向和按 ID 写死的零点**。当前数学 FK/IK 是独立的理想串联腿模型，尚未证明与实际 RoboDog V2 齿轮/轴线/连杆一致。
5. Microduck 两个仓库只用于未来学习“仿真、执行器建模、策略输出、安全边界”的组织方法。它是 Dynamixel 驱动的双足机器人，现有模型、ONNX 策略、串口协议和标定均不能直接交给 LX-16A 四足。
6. 许可证并不统一：RoboDog V1/V2/V3 根许可 MIT；Tips 根许可 GPL v3；RemoteControl 根许可 MIT，但内置第三方 LCD 库需要单独追溯；Microduck 根许可 Apache-2.0，且 microduck_rl 指定 STL 采用 CC BY-NC-SA 4.0。

## 2. 固定快照总表

“当前”表示本次核查时默认分支的 HEAD，不是保证未来仍然最新。提交时间来自 GitHub commit 元数据。以下每个仓库目录树的 `truncated` 均为 false。

| 仓库与用途 | 默认分支 | 本次固定提交 | 提交时间 UTC | 已读根许可证 |
|---|---|---|---|---|
| [RoboLabHub/RoboDog_v2](https://github.com/RoboLabHub/RoboDog_v2)：主机械升级件、原始 ROS 控制 | `master` | [`5fc34529ef98724d4e69e926aad69a42ca956683`](https://github.com/RoboLabHub/RoboDog_v2/commit/5fc34529ef98724d4e69e926aad69a42ca956683) | 2020-11-03 14:59:36 | [MIT](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/LICENSE) |
| [RoboLabHub/RoboDog_v1.0](https://github.com/RoboLabHub/RoboDog_v1.0)：V2 沿用的机身/底座 | `master` | [`823c5632672111b463e4616f731a1d90a12511b4`](https://github.com/RoboLabHub/RoboDog_v1.0/commit/823c5632672111b463e4616f731a1d90a12511b4) | 2020-06-11 12:27:27 | [MIT](https://github.com/RoboLabHub/RoboDog_v1.0/blob/823c5632672111b463e4616f731a1d90a12511b4/LICENSE) |
| [RoboLabHub/RoboDog_v3](https://github.com/RoboLabHub/RoboDog_v3)：可控制 V2 的较新 ROS 架构 | `main` | [`fbb5a0af437bcb052eeace2f8dad04a81746972c`](https://github.com/RoboLabHub/RoboDog_v3/commit/fbb5a0af437bcb052eeace2f8dad04a81746972c) | 2020-11-03 19:00:46 | [MIT](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/LICENSE) |
| [RoboLabHub/Tips](https://github.com/RoboLabHub/Tips)：Pi 镜像与遥控配套说明 | `master` | [`bdcf34139afd2a9b18752e9a16257f4f9d57a79d`](https://github.com/RoboLabHub/Tips/commit/bdcf34139afd2a9b18752e9a16257f4f9d57a79d) | 2020-11-03 20:42:16 | [GPL v3 文本](https://github.com/RoboLabHub/Tips/blob/bdcf34139afd2a9b18752e9a16257f4f9d57a79d/LICENSE) |
| [RoboLabHub/RemoteControl](https://github.com/RoboLabHub/RemoteControl)：原作者链接的独立遥控器 | `master` | [`194b08a5b0b70c626d7265b89c56dc3630a3864b`](https://github.com/RoboLabHub/RemoteControl/commit/194b08a5b0b70c626d7265b89c56dc3630a3864b) | 2020-11-06 12:21:59 | [MIT；第三方子库需分开查](https://github.com/RoboLabHub/RemoteControl/blob/194b08a5b0b70c626d7265b89c56dc3630a3864b/LICENSE) |
| [pollen-robotics/microduck](https://github.com/pollen-robotics/microduck)：未来：机器人运行时架构 | `main` | [`942e58e3aa6dbb28e7037203fff11866122c7567`](https://github.com/pollen-robotics/microduck/commit/942e58e3aa6dbb28e7037203fff11866122c7567) | 2026-10-19 14:18:36 | [Apache-2.0；有第三方目录例外](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/LICENSE) |
| [pollen-robotics/microduck_rl](https://github.com/pollen-robotics/microduck_rl)：未来：训练与 sim-to-real | `develop` | [`b02fdea4902c2b93b7d34a83a6f05da4ececb7d3`](https://github.com/pollen-robotics/microduck_rl/commit/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3) | 2026-10-12 11:58:55 | [Apache-2.0；指定 STL 为 CC BY-NC-SA 4.0](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/LICENSE) |

旧工程 `docs/PROTOCOL_AND_SOURCES.zh-CN.md` 固定的 V2 和 V3 提交与此次 HEAD 一致。其余仓库的提交由本次独立核验补充，不能用 V2/V3 的 MIT 结论概括其他仓库。

## 3. 当前机械与装配资料

### 3.1 RoboDog_v2：主机械路线

- [README：型号、构建来源、V1 底座与 Pi 镜像关系](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/README.md)
- [STL 目录](https://github.com/RoboLabHub/RoboDog_v2/tree/5fc34529ef98724d4e69e926aad69a42ca956683/STL)：本次核实存在 `base_up.stl`、`base_down.stl`、`frame_leg_up.stl`、`frame_leg_down.stl`、`gear_base.stl`、`gear_leg_base.stl`、`gear_foot.stl`、`gear_foot_cup.stl`、`servo_gear.stl`、`Foot.stl`、`_overall_.stl`。
- 原作者 README 链接的装配入口：[Thingiverse 4557938](https://www.thingiverse.com/thing:4557938)、[DIY Spot Like Quadruped Robot](https://www.instructables.com/DIY-Spot-Like-Quadruped-Robot/)。
- 原作者链接的 LX-16A 使用说明：[How to Configure and Use Smart Servos LewanSoul LX-16A](https://www.instructables.com/id/How-to-Configure-and-Use-Smart-Servos-LewanSoul-LX/)。

可用：追踪模型组成、相互关系、命名、上游装配说明与代码对应关系。

不能据此认定：打印比例、齿轮模数/齿数、孔距、公差、轴承配合、装配方向、实际杆长和受力已经验证。本次没有解析 STL 的坐标或量取网格；`_overall_.stl` 的存在不代表它是可独立打印的完整机构。打印与装配前还要做尺寸和零件清单核对。

### 3.2 RoboDog_v1.0：V2 依赖的基础件

- [README](https://github.com/RoboLabHub/RoboDog_v1.0/blob/823c5632672111b463e4616f731a1d90a12511b4/README.md)
- [STL 目录](https://github.com/RoboLabHub/RoboDog_v1.0/tree/823c5632672111b463e4616f731a1d90a12511b4/STL)：`body_frame.stl`、`body_center_tube.stl`、`body_plane_L.stl`、`body_plane_R.stl`、`wire_holder.stl` 等；目录也包含 V1 腿部/齿轮，不能把 V1/V2 全部零件无区别混用。
- [实物照片目录](https://github.com/RoboLabHub/RoboDog_v1.0/tree/823c5632672111b463e4616f731a1d90a12511b4/Photos)，[装配图 scheme_1.jpg](https://github.com/RoboLabHub/RoboDog_v1.0/blob/823c5632672111b463e4616f731a1d90a12511b4/scheme_1.jpg)，[装配图 scheme_2.jpg](https://github.com/RoboLabHub/RoboDog_v1.0/blob/823c5632672111b463e4616f731a1d90a12511b4/scheme_2.jpg)。
- 原作者装配入口：[Thingiverse 4445119](https://www.thingiverse.com/thing:4445119)。

本次核对了路径及 V2 README 的继承关系，未对照片/装配图逐像素审图，也未确认每件 V1 零件都适用于 V2。

### 3.3 外部装配页的核验状态

Instructables 页面能识别标题和原作者仓库中的链接，但本次网页读取没有返回可审阅的完整正文。Thingiverse 与视频内容未作逐步核验。因此这些是“作者关联的后续阅读入口”，本文件不会把其未读取的 BOM、打印参数、CAD 尺寸或装配步骤当成已验证结论。GitHub 原始 README 和文件路径的可追溯性已独立确认。

## 4. ROS 软件：有用路径、移植价值与启动副作用

### 4.1 RoboDog_v2

| 已核验文件 | 参考用途 | 不可直接照搬部分 |
|---|---|---|
| [sources/README.md](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/README.md) | Ubuntu/ROS Kinetic/Melodic、catkin、serial 依赖与标定入口 | 旧系统安装步骤、udev 规则、运行命令；未在当前平台验证 |
| [sources/IK.cpp](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/IK.cpp) | 原作者三关节逆解与机身旋转 | 坐标轴、零位、逆解分支、平方根可达域；没有证明与本机一致 |
| [sources/LegV2.cpp](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/LegV2.cpp) | 关节角到 LX-16A ticks 的映射 | `kWheelRatio=2.0`、ID 1–12 的方向和零位、写偏移行为 |
| [sources/Services/RobotHW.cpp](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/Services/RobotHW.cpp) | 机器人装配、几何、ID 与默认入口 | 写死偏移/尺寸；打开串口后直接执行调姿 |
| [sources/Trajectory/MovePrimitives.cpp](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/Trajectory/MovePrimitives.cpp) | walk、walk_2、侧移、转向等原语组织 | 步幅、速度、时序、抬腿高度、跳跃/演示动作不适用于新硬件 |
| [sources/GCODE/test.gcode](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/GCODE/test.gcode) | 原作者 G-Code 语法样例 | 样例不是安全动作集，不能直接发送 |

**确定存在的副作用：**

- `RobotHW.cpp` 默认打开 `/dev/ttyUSB0`，建立四条腿，并在 main 中对 1–4 腿调用 `tune_pos_1(false)`；并非仅打印状态。
- 该文件建立 ID 1–12 与四组三舵机偏移值，然后调用 `set_servo_offsets`。这些偏移属于原作者装配，不能作为本机标定。
- 任何移植都应从纯数学/纯封包层入手，先删除或隔离初始化写入，再做无串口测试，不要运行上游入口试探是否兼容。

### 4.2 RoboDog_v3：可借鉴较清晰的软件层次

- [README](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/README.md) 明确这是用于 RoboDog V2/V3 的 ROS 示例，且说明缺少详细教程。
- [sources/Robots/RobotV2.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Robots/RobotV2.cpp)：V2 几何、ID、方向、2:1 映射与对象组装。**选择 V2 时应研究这个文件，而不是把 RobotV3 的标定拿来用。**
- [sources/Services/RobotHW.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Services/RobotHW.cpp)：当前固定提交默认执行 `CreateRobotV3`，`CreateRobotV2` 行被注释。README 的“支持 V2”不代表默认配置就是 V2。
- [sources/Leg.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Leg.cpp)：腿级目标队列与插值组织。
- [sources/Trajectory/MovePrimitives.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Trajectory/MovePrimitives.cpp)：增加 walk_3 / walk_4 等原语；它们是研究材料，不是已验证步态。
- [sources/Servos/ServoController.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Servos/ServoController.cpp)、[sources/Servos/ServoLX16a.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Servos/ServoLX16a.cpp)：舵机队列/周期调用、反馈和驱动分层。
- [sources/Servos/LX16a.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Servos/LX16a.cpp)：Linux serial 接口、115200 串口、数据封包和有符号位置反馈。
- [sources/CMakeLists.txt](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/CMakeLists.txt)：ROS/catkin 与 `libserial.so`、主机路径依赖。
- [sources/RemoteControl/RemoteController.cpp](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/RemoteControl/RemoteController.cpp)：独立遥控链路和命令解释；该链路不能当作 LX-16A 总线协议。

**V3 的初始化同样有真实写操作：** `CreateRobotV2` 先打开串口，再逐只调用 `Factory::CreateLX16a(..., offset)`。该工厂调用 `set_offset`；底层 `LX16a::set_offset` 发出命令 17，再发命令 18（保存偏移）。随后设置最大温度，并初始化腿位置。请结合以上三处固定源码阅读，不能将其当成无写入风险的构造函数。

**许可证元数据不完整：** V3 根 LICENSE 和已读取主要 .cpp 头部写 MIT，但 [sources/package.xml](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/package.xml) 的 `<license>` 仍是 `TODO`。这不是“无许可证”，也不是可以忽略文件头和第三方依赖的理由；下游打包时应补足准确的归属清单。

### 4.3 为什么它们不是 F407 固件

上游需要 ROS 时间/服务、catkin、Unix 串口、线程和标准库对象，目录中没有为本项目 F407 板验证过的启动文件、链接脚本、HAL/CMSIS 工程、时钟/引脚配置、半双工收发实现或烧录产物。本次也没有做目标板交叉编译。

可复用的层次是“算法思路、纯计算/协议逻辑、模块职责与测试案例”，不是完整运行程序。若实际复制 MIT 代码，应保留适用版权与完整许可，记录原始提交和所改文件。若保持当前原创实现，应继续明确它不是上游经过认证的移植版。

## 5. 机构与标定：已知不一致必须保留

### 5.1 原作者代码中的真实约定

- V2 `LegV2.cpp` 与 V3 `RobotV2.cpp` 都有 `kWheelRatio = 2.0`，并对每个 ID 单独映射符号/偏移。公式中的 500、750、250、1200、−200 等常数是映射参数，不是全部都能作为安全目标发送。
- 原作者设置几何为 `0.043, 0.165, 0.187` 米，并注释提及足球半径 20 mm。**这只证明代码采用这些数值，不证明用户打印件/装配的有效杆长相同，也不能自行再加一次 20 mm。**
- V2 设置机身腿间距为 `0.330, 0.223` 米；V3 的 V2 配置使用腿安装点 `x=±0.165, y=±0.112` 米。后者横向跨度为 0.224 米，与前者相差 1 mm，进一步说明不能跨版本无核对复制尺寸。
- 原作者 IK 在当前坐标定义下使用 `sqrt(x²+y²−L1²)` 和 `sqrt(1−D²)`，并通过 `PI−Q3` 输出第三角。与“相对膝角、零位向下、roll 绕 +x”模型的角变量并不直接等价。

以上依据：[V2 几何与安装参数](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/Services/RobotHW.cpp)、[V2 IK](https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/IK.cpp)、[V3 内的 V2 映射与几何](https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Robots/RobotV2.cpp)。

### 5.2 对当前单腿工程的约束

当前 `robot-dog-firmware/docs/KINEMATICS.zh-CN.md` 已明确：其 FK/IK 是理想串联模型；实际 STL 自由度、齿轮、轴线、尺寸和传动尚未核实。本文件不推翻该边界。

后续本地 Codex 应先产出：
1. 实际机械的关节/传动拓扑，以及每个舵机输出轴与机构关节轴的关系。
2. 实测/CAD 复核后的坐标系、有效杆长、轴向偏置、左右腿镜像规则。
3. 每个舵机的 ID、方向、零位、传动比、软件限位和物理干涉限位。
4. “足端坐标 → 机构关节角 → 传动/耦合变换 → 舵机轴角 → ticks”分层说明与测试；不能只凭 2:1 常数跳过中间层。
5. 不可达、奇异、非有限输入、分支跳变与越限处理；所有测试先在无硬件传输的环境中运行。

只有数学往返测试通过，不能说明齿轮机构或舵机映射正确。当前未配置参数应保持未配置，不能拿仓库常数补成“可运行示例”。

## 6. 原作者配套材料：只在需要时阅读

### 6.1 Tips/RoboDog_image

[镜像说明](https://github.com/RoboLabHub/Tips/blob/bdcf34139afd2a9b18752e9a16257f4f9d57a79d/RoboDog_image/README.md) 说明目标是 **Pi 4、4GB 内存、32GB microSD**，包含 Ubuntu 20.04、ROS Noetic、V2/V3 控制器；还说明 V3 控制 V2 的启动入口与遥控连接，链接到 RemoteControl。

这是系统镜像的说明，不是源代码供应链证明或 F407 固件。本次只读 README，未验证外部 Google Drive 镜像的可下载性、内容、校验和、安全状态或完整源码对应关系，也未下载/执行 APK。老镜像存在公开默认登录配置，后续若研究它必须另行处理账号与网络安全，不应在本交接中传播默认口令或直接部署。

Tips 根目录 [LICENSE](https://github.com/RoboLabHub/Tips/blob/bdcf34139afd2a9b18752e9a16257f4f9d57a79d/LICENSE) 是 GPL v3 文本。**不能把它当成 RoboDog 仓库的 MIT 附件，也不能认为该根许可证一并替 Ubuntu、ROS、镜像中所有软件或 APK 解决了许可问题。** 当前只引用资料链接，不引入 Tips 源码或二进制。

### 6.2 RemoteControl

作者关联链：V3 README 与 RemoteControl README 指向同一个 Thingiverse 4586188；Tips/RoboDog_image README 直接链接 [RemoteControl 仓库说明](https://github.com/RoboLabHub/RemoteControl/blob/194b08a5b0b70c626d7265b89c56dc3630a3864b/README.md)。因此它是可核实的配套来源，不是凭名字猜测的无关遥控项目。

有用路径：
- [_Arduino/_Arduino.ino](https://github.com/RoboLabHub/RemoteControl/blob/194b08a5b0b70c626d7265b89c56dc3630a3864b/_Arduino/_Arduino.ino)：Arduino 遥控程序、串口报文和蓝牙配置流程。
- [_Arduino/Commands.h](https://github.com/RoboLabHub/RemoteControl/blob/194b08a5b0b70c626d7265b89c56dc3630a3864b/_Arduino/Commands.h)、[_Arduino/Joystick.h](https://github.com/RoboLabHub/RemoteControl/blob/194b08a5b0b70c626d7265b89c56dc3630a3864b/_Arduino/Joystick.h)：命令布局和摇杆输入组织。
- [遥控器外壳 STL](https://github.com/RoboLabHub/RemoteControl/tree/194b08a5b0b70c626d7265b89c56dc3630a3864b/STL)。

这不是 F407 主控固件，更不是直接输出 LX-16A 舵机帧的替代驱动。当前单腿离线阶段不需要引入它。根 LICENSE 是 MIT；`_Arduino/Libraries/LiquidCrystal_I2C/` 是内置第三方 LCD 库，目录树没有独立 LICENSE，README 和文件头指向 DFRobot/其他作者。**尚未完成该库的上游许可链核验；不要把整目录及其中 .o 二进制按 MIT 一并复制。**

## 7. 未来概念参考：Microduck，不进入当前硬件控制链

### 7.1 pollen-robotics/microduck

[README](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/README.md) 描述的是小型双足机器人运行时，Linux/RK3566 上运行 Rust 服务和神经策略；不是 LX-16A/F407 固件。

优先阅读：
- [docs/design/architecture.md](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/docs/design/architecture.md)：控制进程、命令与设备所有权。
- [duck-control/src/safety.rs](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/duck-control/src/safety.rs)：把设备写权限集中在安全层；非有限值拒绝和命令过期处理。
- [duck-control/src/io.rs](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/duck-control/src/io.rs)：运行时与机器人 I/O 的接口边界。
- [duck-control/src/bus.rs](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/duck-control/src/bus.rs)：**Dynamixel XL330 / Protocol 2.0** 总线实现，用于理解差异，不用于 LX-16A 协议实现。
- [docs/design/simulation.md](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/docs/design/simulation.md)、[docs/policy-manifest.md](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/docs/policy-manifest.md)：仿真替身和模型接口契约。

可学其分层，不能复用其安全数值：`safety.rs` 明确区分执行器行程与真实关节机械限位，且其失联/跌倒行为为该双足原型设计，不能直接应用到当前单腿或四足。ONNX 的输入输出定义、关节数量和执行器反馈能力均不同。

根许可 Apache-2.0；并非所有 vendored 文件自动采用根许可，例如 [tof/vendor/LICENSE.txt](https://github.com/pollen-robotics/microduck/blob/942e58e3aa6dbb28e7037203fff11866122c7567/tof/vendor/LICENSE.txt) 有独立条款。当前不引入其 vendor、模型或部署脚本。

### 7.2 pollen-robotics/microduck_rl

[README](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/README.md)：MuJoCo/mjlab、PPO、BAM 执行器建模、域随机化、齿隙模型、ONNX 导出与部署契约。

优先阅读：
- [src/mjlab_microduck/robot/microduck_constants.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/robot/microduck_constants.py)：机器人/关节/执行器配置组织。
- [src/mjlab_microduck/actuator/friction_dr_bam.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/actuator/friction_dr_bam.py)：执行器摩擦与反馈侧齿隙建模。
- [src/mjlab_microduck/tasks/backlash.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/tasks/backlash.py)：齿隙变体组织。
- [src/mjlab_microduck/tasks/microduck_velocity_env_cfg.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/tasks/microduck_velocity_env_cfg.py)、[src/mjlab_microduck/tasks/mdp.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/tasks/mdp.py)：环境、观察、奖励和随机化职责。
- [src/mjlab_microduck/robot/microduck/robot_walk.xml](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/src/mjlab_microduck/robot/microduck/robot_walk.xml)：MJCF 结构示例；是 Microduck 模型。
- [scripts/export.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/scripts/export.py)、[scripts/infer_policy.py](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/scripts/infer_policy.py)：导出/仿真验收工作流。

未来迁移需先为自己的机构重建质量、惯量、碰撞、足端接触、齿轮/齿隙、舵机延迟与饱和模型，再重新定义观测/动作、训练和验证；不能把 XL330 的模型或训练权重直接迁给 LX-16A。

许可按 [LICENSE-MODELS.md](https://github.com/pollen-robotics/microduck_rl/blob/b02fdea4902c2b93b7d34a83a6f05da4ececb7d3/LICENSE-MODELS.md) 逐项区分：
- 代码及该文件指定的 XML、JSON、.part 等：Apache-2.0。
- `src/mjlab_microduck/robot/microduck/assets/*.stl` 与 `src/mjlab_microduck/robot/xl330_test_bench/assets/*.stl`：CC BY-NC-SA 4.0，含非商业与相同方式共享限制。
- 外部下载的策略权重/数据另查其模型卡与许可，不能从仓库代码许可推定。

不把 ODRI/Solo12 添加成当前依赖。它属于不同执行器/动力学路线；若以后决定研究该方向，再开独立来源与硬件匹配核验，避免混淆当前方案。

## 8. LX-16A 与 BusLinker：制造商资料及易混淆项

### 8.1 本阶段采用的协议依据

- [Hiwonder LX-16A 官方产品页](https://www.hiwonder.com/products/lx-16a)：115200 baud、0–240°、ID 0–253、额定工作电压 6–8.4 V。产品页可能出现不同重量/扭矩表述，本交接不以营销规格作为持续负载设计证明。
- [Hiwonder Bus Servo Communication Protocol PDF，Purdue 托管副本](https://engineering.purdue.edu/477grp4/Team/journal/img%20-%20Juho/week7/servo%20bus%20protocol.pdf)：制造商署名协议，14 页；这是非制造商网站托管的副本，不能把 Purdue 误写成作者或把该网址称为官方托管。
- [Hiwonder MaxArm 官方硬件示例](https://docs.hiwonder.com/projects/MaxArm/en/latest/docs/5.Hardware_Basic_Learnig_formatted.html)：旧工程用它交叉核对同类 Lobot 示例的 8N1 配置。本次沿用旧协议核验说明，不声称该示例机器人/舵机型号就是 LX-16A。

协议的 0–1000 位置刻度、半双工、帧长/校验和、有符号当前位置、读回复与写操作无 ACK，应以本包 `robot-dog-firmware/docs/PROTOCOL_AND_SOURCES.zh-CN.md` 的逐项证据和计算测试向量为准。协议允许的全部命令不等于 v0.1 授权发送的子集；本交接不扩张广播、ID、EEPROM、加载扭矩或同步启动权限。

### 8.2 BusLinker 版本必须先确认

- [旧款 Hiwonder TTL/USB Debugging Board 产品页](https://www.hiwonder.com/products/hiwonder-ttl-usb-debugging-board)：提供主机/MCU 串口连接说明；问答中的板卡供电范围不能代替舵机电压或 MCU 逻辑电平规格。
- [BusLinker V3.0 官方手册](https://docs.hiwonder.com/projects/BusLinker/en/latest/docs/1_BusLinker_V3.0_Servo_Debugging_Board_User_Manual.html)：列出 USB/TTL 选择跳线；同时包含磁编码器和标准舵机两套说明。文档外层“v1.0”是文档版本标签，硬件章节标题是 V3.0。
- [BusLinker 官方 STM32 示例](https://docs.hiwonder.com/projects/BusLinker/en/latest/docs/3_STM32_Development.html)：目标是 **STM32F103RBT6**，代码包含 `HX_30HM.h`；调试串口 115200，舵机侧串口 **1000000**。它不是 F407 + LX-16A 的直接适配。

**严禁套用不同型号的参数：** V3.0 文档中的 11.1V/12V、1Mbps、0–4095 位置、HX-30HM 固件更新或扫描例程，不是 LX-16A 的供电/协议授权。V3 板能支持多种连接，也不代表所有章节可混用。

实物板卡版本、跳线、逻辑电平、引脚、是否自动换向、是否回显、总线释放时机和电源路径目前未验证。不能凭产品名推断 MCU TX/RX 可短接、3.3V 可直接连接或 USB 可为舵机提供足够电流。进入实机阶段前必须查匹配版本原理图/手册并测试接口；在此之前保持 transport 未绑定、不可发送。

## 9. 本地获取命令：只拉取资料，不执行项目

以下命令供本地 Codex 在用户选定的工程目录使用。**本次没有执行这些 clone。** `--filter=blob:none --no-checkout` 先获取 Git 元数据，不默认检出大型模型/镜像；读取单文件时 Git 可能按需下载该 blob。若服务器不支持过滤或提示将大批下载，应停下报告，不自动退化成整仓下载。

只取当前需要的仓库；Microduck、Tips、RemoteControl 都是可选阅读。不要自动执行 README 中的安装、sudo、udev、rosrun、roslaunch、串口、烧录、模型下载或训练命令。

```bash
mkdir -p external
git clone --filter=blob:none --no-checkout https://github.com/RoboLabHub/RoboDog_v2.git external/RoboDog_v2
git clone --filter=blob:none --no-checkout https://github.com/RoboLabHub/RoboDog_v1.0.git external/RoboDog_v1.0
git clone --filter=blob:none --no-checkout https://github.com/RoboLabHub/RoboDog_v3.git external/RoboDog_v3
git clone --filter=blob:none --no-checkout https://github.com/RoboLabHub/Tips.git external/Tips
git clone --filter=blob:none --no-checkout https://github.com/RoboLabHub/RemoteControl.git external/RemoteControl
git clone --filter=blob:none --no-checkout https://github.com/pollen-robotics/microduck.git external/microduck
git clone --filter=blob:none --no-checkout https://github.com/pollen-robotics/microduck_rl.git external/microduck_rl
```

先验证固定提交可读，再用 `git show` 阅读；这些命令不运行项目源码：
```bash
git -C external/RoboDog_v2 show -s --format='%H %cI %s' 5fc34529ef98724d4e69e926aad69a42ca956683
git -C external/RoboDog_v2 show 5fc34529ef98724d4e69e926aad69a42ca956683:LICENSE
git -C external/RoboDog_v1.0 show -s --format='%H %cI %s' 823c5632672111b463e4616f731a1d90a12511b4
git -C external/RoboDog_v1.0 show 823c5632672111b463e4616f731a1d90a12511b4:LICENSE
git -C external/RoboDog_v3 show -s --format='%H %cI %s' fbb5a0af437bcb052eeace2f8dad04a81746972c
git -C external/RoboDog_v3 show fbb5a0af437bcb052eeace2f8dad04a81746972c:LICENSE
git -C external/Tips show -s --format='%H %cI %s' bdcf34139afd2a9b18752e9a16257f4f9d57a79d
git -C external/Tips show bdcf34139afd2a9b18752e9a16257f4f9d57a79d:LICENSE
git -C external/RemoteControl show -s --format='%H %cI %s' 194b08a5b0b70c626d7265b89c56dc3630a3864b
git -C external/RemoteControl show 194b08a5b0b70c626d7265b89c56dc3630a3864b:LICENSE
git -C external/microduck show -s --format='%H %cI %s' 942e58e3aa6dbb28e7037203fff11866122c7567
git -C external/microduck show 942e58e3aa6dbb28e7037203fff11866122c7567:LICENSE
git -C external/microduck_rl show -s --format='%H %cI %s' b02fdea4902c2b93b7d34a83a6f05da4ececb7d3
git -C external/microduck_rl show b02fdea4902c2b93b7d34a83a6f05da4ececb7d3:LICENSE
```

示例：只看 V2 的数学和映射，不检出/执行整个工程：
```bash
git -C external/RoboDog_v2 show 5fc34529ef98724d4e69e926aad69a42ca956683:sources/IK.cpp
git -C external/RoboDog_v2 show 5fc34529ef98724d4e69e926aad69a42ca956683:sources/LegV2.cpp
git -C external/RoboDog_v3 show fbb5a0af437bcb052eeace2f8dad04a81746972c:sources/Robots/RobotV2.cpp
```

需要将少数文件纳入下游时，先确认其具体许可和依赖，再以固定提交取出并建立来源记录；不要把所有仓库加入子模块、自动构建或烧录流程。若固定提交将来不可取，报告原 URL/提交和失败原因，不悄悄换成分支最新版本。

## 10. 下游复用与验收清单

- [ ] 来源表记录仓库、固定提交、文件路径、许可、是否复制/修改。
- [ ] MIT 代码复制/实质性改写时保留完整许可和版权，包括已读文件头的 RoboLab19 与根许可 RoboLab 等适用声明。
- [ ] Apache-2.0 代码分发时带许可证、保留适用归属声明；修改文件标明变更；若原分发含 NOTICE 则保留相关 NOTICE。第三方目录另核验。
- [ ] Tips GPL、microduck_rl 的非商业 STL、RemoteControl 内置第三方库、外部镜像/APK/权重不以根许可一概处理。当前只引用，不打包这些内容。
- [ ] 上游自带初始化动作、偏移保存、阈值写入、广播/扫描、调姿和演示动作均从离线研究流程隔离。
- [ ] 机械几何与传动未验证时，不把 IK 输出映射到真实电机，不填入“猜测标定”。
- [ ] ROS 主机编译成功、纯数学测试成功与 STM32 目标板/实机验证分开报告。
- [ ] 后续若复制来源文件，应在项目中另建第三方声明；本 SOURCES 是证据目录，不是替代各源代码许可证的统一授权书。

**本次未解决事项：** 精确 STL 机构/尺寸与传动验证；用户实际 BusLinker/F407 板卡资料；原作者外部装配正文与 Pi 镜像的完整性/可用性；RemoteControl 第三方 LCD 库许可链；任何第三方项目的构建、运行和实机效果。以上均保持未验证，不写成已完成。

