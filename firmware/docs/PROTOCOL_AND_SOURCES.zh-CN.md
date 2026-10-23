# LX-16A 协议核对与参考来源

核查日期：2026-10-23（UTC）。本文件服务于单腿固件 v0.1 的离线开发与审阅，不是接线图或已经通过实机验证的驱动说明。

## 1. 实现与验证边界

- 本项目根据公开协议事实独立实现；未复制或移植下列 RoboDog 项目的代码。它们仅作为架构、运动学和轨迹组织的参考。
- 软件参考项目的 MIT 许可证已经实际读取；不以“仓库公开”代替许可证核验。制造商产品页和协议文档属于技术资料，不据此声称它们采用 MIT 许可证。
- 本次没有连接、扫描、供电、烧录或驱动任何舵机。下列数据包是按协议计算的测试输入，不是逻辑分析仪抓包，也不是实机成功记录。
- 协议完整能力与 v0.1 可发送的子集不同。列出某个命令不表示本版本允许发送它。

## 2. 串口、封包与反馈

### 2.1 已核查参数

LX-16A 官方产品页给出 **115200 baud**、ID **0–253**、控制角度 **0–240°** 和工作电压 **6–8.4 V**。[S1]

制造商协议说明采用半双工 UART。帧结构如下：[S2]

```text
55 55 ID LENGTH COMMAND PARAMETERS... CHECKSUM

N = 参数字节数
LENGTH = N + 3
完整帧字节数 = LENGTH + 3 = N + 6
CHECKSUM = (~(ID + LENGTH + COMMAND + 参数各字节之和)) & 0xFF
```

校验和不包含两个 `0x55` 帧头，也不包含校验和字段自身。16 位参数采用低字节在前的顺序。

固件串口设置为 **115200、8 数据位、无校验、1 停止位（8N1）**。证据区分如下：LX-16A 产品页与协议 PDF 明确给出波特率；Hiwonder 官方 Lobot 总线舵机示例明确使用 `SERIAL_8N1`，作为同类协议的配置交叉核对，并非声称 LX-16A 产品页逐项列出了 8N1。[S1][S2][S3]

### 2.2 命令核对表

下表的“长度”均指帧内 `LENGTH`，不是完整帧字节数。[S2]

| 功能 | 命令值（十进制） | 请求长度 | 参数或返回内容 | 返回长度 |
|---|---:|---:|---|---:|
| 立即移动 | 1 | 7 | 位置 uint16 + 时间 uint16 | 无 ACK |
| 读取已设移动参数 | 2 | 3 | 返回位置、时间 | 7 |
| 暂存移动 | 7 | 7 | 位置、时间；等待启动命令 | 无 ACK |
| 读取暂存参数 | 8 | 3 | 返回暂存位置、时间 | 7 |
| 启动暂存移动 | 11 | 3 | 无参数 | 无 ACK |
| 停止移动 | 12 | 3 | 停在当前位置 | 无 ACK |
| 读取 ID | 14 | 3 | 返回 ID 字节 | 4 |
| 读取温度 | 26 | 3 | 返回 uint8，单位 °C | 4 |
| 读取电压 | 27 | 3 | 返回 uint16，单位 mV | 5 |
| 读取当前位置 | 28 | 3 | 返回 **int16 有符号位置** | 5 |
| 加载／卸载 | 31 | 4 | 参数 0：卸载；1：加载 | 无 ACK |
| 读取加载状态 | 32 | 3 | 返回 0 或 1 | 4 |

移动目标的协议范围为 **0–1000**，对应 0–240°；时间为 **0–30000 ms**。这些协议极限不等于装配后的关节安全范围。[S2]

读响应使用相同帧格式，并重复原读命令。**当前位置响应必须解码为 int16，可能为负数**；不要把 `FF FF` 解释为 65535。出站移动目标仍须独立检查 0–1000。[S2] RoboDog V3 的读取实现也采用 `int16_t`。[R3]

写命令没有协议 ACK。“串口发送完成”只表示字节已经发出，不证明舵机收到、执行或到位。位置和加载状态需单独读取验证。发送后应立即准备接收；底层须确认最后一个停止位已经发出，再释放发送驱动，不能把 DMA 搬运结束直接当作线路发送结束。[S2；后半句为本项目的驱动接口要求]

停止（12）与卸载（31，参数 0）是不同操作。停止不等于关闭扭矩；卸载会失去支撑力，机械结构可能下落。串口失效时软件命令不能保证生效，台架应具备独立断电手段和机械支撑。

## 3. v0.1 允许的子集

本版本只面向已配置、已核对的单播 ID。计划支持的出站功能为：受安全状态机约束的立即移动（1）、停止（12）、位置／电压／温度／加载状态读取（28/27/26/32），以及卸载（31，参数只能为 0）。

本版本**不提供**：

- 广播发送，包括广播停止和广播卸载
- ID 探测、读取或改写入口
- EEPROM／掉电保存参数写入，包括偏移保存、角度限位和保护阈值配置
- 扭矩开启（加载命令参数 1）
- 暂存移动与同步启动（7/8/11）
- 电机连续旋转模式配置

协议确实定义广播 ID `0xFE`，但这不是普通舵机地址。通常广播不回复；广播 ID_READ 是例外，且要求总线上仅有一只舵机，否则可能发生响应冲突。[S2] 此处仅说明协议差异，不能据此绕过 v0.1 的禁止项。枚举常量的存在也不等于可发送权限。

## 4. 计算测试向量（非实机记录）

以下十六进制字节按第 2 节公式独立计算。它们适合编码器／解析器测试；不会自动发送到硬件。

| 含义 | 完整帧 |
|---|---|
| ID 1，移动到 500，1000 ms | `55 55 01 07 01 F4 01 E8 03 16` |
| ID 1，停止 | `55 55 01 03 0C EF` |
| ID 1，卸载 | `55 55 01 04 1F 00 DB` |
| ID 1，读取位置请求 | `55 55 01 03 1C DF` |
| 假设 ID 1 返回位置 500 | `55 55 01 05 1C F4 01 E8` |
| 假设 ID 1 返回位置 −1 | `55 55 01 05 1C FF FF DF` |
| ID 1，读取温度请求 | `55 55 01 03 1A E1` |
| ID 1，读取电压请求 | `55 55 01 03 1B E0` |

负位置向量用于验证解码类型，不表示允许将 −1 作为移动目标。应另外测试错误校验和、截断帧、超时、错误 ID／命令、参数长度不符、噪声与连续帧；这些是本项目的测试要求，不是制造商给出的实测保证。

## 5. 电气与资料风险

### BUSLINKER 与接线

**实际 BUSLINKER 的型号、版本、工作模式和电气状态尚未确认。** 本文件不假定其完成了电平转换、自动换向、接收回显过滤或总线释放，也不假定任何 MCU 引脚能直接连接舵机信号。

进入实机阶段前，必须核对板卡资料、接口电平、引脚定义、供电与接地、USB/TTL 模式、收发方向控制及回显行为。不得根据本文件将 MCU TX/RX 直接短接，不得假定 USB 能给舵机提供足够电流。底层适配未完成验证时应保持不可发送状态。

### 文档不一致与型号混淆

- 协议 PDF 的偏移读取段落存在命令值／长度笔误；其总表与响应表使用命令 19。v0.1 不暴露该命令，不能依靠有歧义的段落新增写入行为。[S2]
- PDF 的通用电压阈值设置／读取段落存在范围和单位描述不一致。**不能将可编码的保护阈值范围当作 LX-16A 的供电额定值**；以该型号官方 6–8.4 V 规格为界，并核对手中硬件。[S1][S2]
- Hiwonder 的 HX-30HM 磁编码器协议资料可能显示 1 Mbps 默认值。它不是本项目的 LX-16A 协议来源；不得将其波特率、寄存器表或位置尺度混入本驱动。[S4]
- RoboDog V2 的映射包含固定 2:1 传动比和按舵机 ID 区分的零点；不能直接作为另一台机器的标定。[R2]
- 两个 RoboDog 版本的 IK 都使用涉及工作空间边界的平方根。新实现需要独立检查几何参数、可达性、有限数值和关节限制；不能把参考公式的输出直接发送给舵机。[R1][R4]

## 6. 可核查来源与许可证

以下链接于 **2026-10-23 UTC** 实际读取或通过 GitHub 文件接口核查。RoboDog 链接使用固定提交，避免分支变动后来源漂移。

### 制造商资料

- **[S1] Hiwonder LX-16A 官方产品页**：型号参数、115200 baud、ID、行程和电压。<https://www.hiwonder.com/products/lx-16a>
- **[S2] Hiwonder Bus Servo Communication Protocol**：制造商署名协议 PDF，由 Purdue 托管的副本；协议与命令内容见第 1–14 页。此链接不是制造商网站，不将托管方误称为协议作者。<https://engineering.purdue.edu/477grp4/Team/journal/img%20-%20Juho/week7/servo%20bus%20protocol.pdf>
- **[S3] Hiwonder MaxArm 官方开发示例**：Lobot 总线示例中的 115200 / SERIAL_8N1 设置。该示例的机器人和舵机型号不同，只用于 UART 配置交叉核对。<https://docs.hiwonder.com/projects/MaxArm/en/latest/docs/5.Hardware_Basic_Learnig_formatted.html>
- **[S4] Hiwonder 官方 HX-30HM 协议页，仅用于排除型号混淆**：本项目未复制其代码或使用其寄存器协议。<https://github.com/Hiwonder-official/hiwonder-servo-sdk/blob/main/docs/protocol_en.md>

### RoboDog V2：MIT 参考

仓库：RoboLabHub/RoboDog_v2。已核查 `master` 指向提交 `5fc34529ef98724d4e69e926aad69a42ca956683`，提交时间 2020-11-03 14:59:36 UTC。

- **[R1] IK 与坐标旋转**：<https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/IK.cpp>
- **[R2] V2 单腿角度到舵机位置映射**：<https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/LegV2.cpp>
- **步态与轨迹原语**：包含 `walk`、`walk_2`、摆腿、侧移与转向。<https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/Trajectory/MovePrimitives.cpp>
- **构建与标定说明**：这是 ROS/catkin 主机端项目，不是即用型 MCU 固件。<https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/sources/README.md>
- **已读取的 MIT LICENSE**：<https://github.com/RoboLabHub/RoboDog_v2/blob/5fc34529ef98724d4e69e926aad69a42ca956683/LICENSE>

### RoboDog V3：MIT 参考

仓库：RoboLabHub/RoboDog_v3。已核查 `main` 指向提交 `fbb5a0af437bcb052eeace2f8dad04a81746972c`，提交时间 2020-11-03 19:00:46 UTC。

- **[R3] LX-16A 主机端串口实现**：<https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Servos/LX16a.cpp>
- **[R4] IK 与坐标旋转**：<https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/IK.cpp>
- **步态与轨迹原语**：在早期原语外提供 `walk_3`、`walk_4` 等组织方式。<https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Trajectory/MovePrimitives.cpp>
- **单腿轨迹队列与插值**：<https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/sources/Leg.cpp>
- **已读取的 MIT LICENSE**：<https://github.com/RoboLabHub/RoboDog_v3/blob/fbb5a0af437bcb052eeace2f8dad04a81746972c/LICENSE>

两个仓库的根许可证均署名 `Copyright (c) 2020 RoboLab`；已查看的源文件头部另署名 `Copyright (c) 2020 RoboLab19`。如果以后实际复用其代码，应保留适用的版权与许可声明。本版本仅引用其架构和协议核对结果，不据此将原始固件描述为 RoboDog 的移植版。
