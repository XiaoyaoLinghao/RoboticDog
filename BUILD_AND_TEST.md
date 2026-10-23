# 构建与测试操作指南

工作目录说明：下面的 CMake 命令从交接包根目录执行，源码在 `firmware/`。工具版本及命令基于现有 CMakeLists.txt 和脚本核对；本包实际重新运行的是 Linux GCC 直接构建。CMake、Windows、macOS、ARM 命令没有在这些环境执行过，必须由本地 Codex 记录实际结果。

## 最小依赖

- C++17 编译器。现有云端复验为 GCC 14.2.0。
- CMake 路线要求 CMake 3.16 或更新，以及可用生成器/构建工具。
- GCC 脚本路线要求 Bash 和 g++；不需要 CMake。
- 当前库仅使用 C++ 标准库/数学库；没有 ROS、Python、STM32 HAL、CMSIS、串口库或第三方包依赖。
- 本地缺依赖时先报告现状，优先已有正规工具；不要为了主机测试安装整个 ROS 或启动上游机器人项目。

## Linux 有 CMake

```sh
cmake -S firmware -B build-host-debug -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=OFF
cmake --build build-host-debug --parallel
ctest --test-dir build-host-debug --output-on-failure
./build-host-debug/dry_run

cmake -S firmware -B build-host-release -DCMAKE_BUILD_TYPE=Release -DROBOT_ENABLE_SANITIZERS=OFF
cmake --build build-host-release --parallel
ctest --test-dir build-host-release --output-on-failure
```

GCC/Clang sanitizer 单独构建：

```sh
cmake -S firmware -B build-host-sanitizers -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=ON
cmake --build build-host-sanitizers --parallel
ctest --test-dir build-host-sanitizers --output-on-failure
```

某些跟踪/容器环境的 LeakSanitizer 无法运行，只有确实遇到该限制时使用：

```sh
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-host-sanitizers --output-on-failure
```

关闭泄漏检测必须写进报告；ASan/UBSan 通过不代表泄漏检查通过。

## Linux 只有 GCC 或暂时没有 CMake

```sh
cd firmware
bash tools/test_host.sh plain
ASAN_OPTIONS=detect_leaks=0 bash tools/test_host.sh sanitizers
```

这两条是本包实际重新执行的路线。脚本分别构建并运行 `test_protocol`、`test_bus`、`test_safety`、`test_kinematics`、`dry_run`，任何失败返回非零状态。脚本输出在 `firmware/build-direct-plain/` 与 `firmware/build-direct-sanitizers/`；它们不应提交或打进源码 ZIP。

脚本使用 `CXX` 环境变量指定编译器，默认 `g++`。Sanitizer 模式还含 `-fno-pie -no-pie`，这是当前 Linux GCC 路线的设置，不能当成 macOS 或任意工具链通用脚本。

如要在无 CMake 的 Linux 上确认 Release/NDEBUG 不隐藏断言，可在 `firmware/` 运行：

```sh
mkdir -p build-direct-release
for name in protocol bus safety kinematics; do
  g++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror -Iinclude -I. \
    src/lx16a.cpp src/bus.cpp src/safety.cpp src/kinematics.cpp \
    "tests/test_${name}.cpp" -o "build-direct-release/test_${name}" || exit 1
  "build-direct-release/test_${name}" || exit 1
done
g++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror -Iinclude -I. \
  src/lx16a.cpp src/bus.cpp src/safety.cpp src/kinematics.cpp \
  examples/dry_run.cpp -o build-direct-release/dry_run
./build-direct-release/dry_run
```

## macOS 建议用 CMake 和 Apple Clang

前提是本机已经有相应开发工具。先记录 `clang++ --version` 与 `cmake --version`。不要把系统的 `g++` 别名当作 GNU GCC。

```sh
cmake -S firmware -B build-macos-debug -DCMAKE_BUILD_TYPE=Debug -DROBOT_ENABLE_SANITIZERS=OFF
cmake --build build-macos-debug --parallel
ctest --test-dir build-macos-debug --output-on-failure
./build-macos-debug/dry_run

cmake -S firmware -B build-macos-release -DCMAKE_BUILD_TYPE=Release -DROBOT_ENABLE_SANITIZERS=OFF
cmake --build build-macos-release --parallel
ctest --test-dir build-macos-release --output-on-failure
```

GCC/Clang 路径在 CMake 中启用 ASan/UBSan；Apple Clang 实际支持情况以本机为准，另建 sanitizer 目录后验证。不要直接使用脚本中的 Linux `-no-pie` 组合。当前没有 macOS 通过证据。

## Windows PowerShell 使用 Visual Studio 多配置生成器

前提是已安装可用的 Visual Studio C++ 构建工具和 CMake，并在能找到编译器的终端运行。以下由 CMake 选择本机默认生成器，不固定不存在的 Visual Studio 版本。

```powershell
cmake -S firmware -B build-windows -DROBOT_ENABLE_SANITIZERS=OFF
cmake --build build-windows --config Debug --parallel
ctest --test-dir build-windows -C Debug --output-on-failure
.\build-windows\Debug\dry_run.exe

cmake --build build-windows --config Release --parallel
ctest --test-dir build-windows -C Release --output-on-failure
.\build-windows\Release\dry_run.exe
```

重要：多配置生成器必须给 build 传 `--config Debug/Release`，给 CTest 传 `-C Debug/Release`。若本机使用 Ninja 等单配置生成器，请分别建立 Debug/Release 目录并传 `-DCMAKE_BUILD_TYPE=...`，可执行文件通常直接在构建目录中，不带 `Debug/` 子目录。

当前 CMake 的 sanitizer 配置只支持编译器 ID 为 GNU/Clang；MSVC 启用 `ROBOT_ENABLE_SANITIZERS=ON` 会明确报错。这不代表 MSVC 永远不能用 ASan，只表示本项目还没有实现/验证该构建路径。Windows 普通测试通过后再独立增加适配，不屏蔽错误冒充全绿。

若本机有 WSL，可按 Linux 路线在 WSL 中运行，但报告必须写“WSL/Linux”，不能写成 Windows 原生通过。

## 预期输出与如何判定

CTest 当前登记 5 个测试：protocol、bus、safety、kinematics、dry_run。所有命令退出码为 0 才是通过。GCC 脚本会输出 `PASS all 5 host executables (...)`。

`dry_run` 必须含：

```text
Host dry-run only. Transport ready: no. Configuration accepted: no.
No servo IDs, pin assignments, or mechanical limits are configured. No hardware I/O.
```

若出现 ready yes 或 Configuration accepted yes，不是进度提升，而是默认安全边界被意外改变，应停止并检查配置。

## 新模块加入构建时

1. 将真正的核心 `.cpp` 加入 `robot_core`；把对应测试登记到 CMake/CTest 和直接构建脚本，避免一个入口漏测。
2. 保持测试中的 CHECK 在 Release/NDEBUG 下执行。现有 tests/test_util.hpp 和运动学测试自行实现了持续检查。
3. 核心不依赖真实硬件或串口。新增 CLI/replay 必须默认为模拟，不允许自动枚举串口或后台连接。
4. 不添加 `-ffast-math`。保留有限数与量化后检查，不通过删失败测试解决问题。
5. 当前 CMake 警告选项主要设在 robot_core，直接脚本则对核心与测试共同启用警告；改动构建时如需统一检查，应显式做并报告。
6. 再跑 Debug、Release、sanitizer（支持的平台）和全部新增场景。只跑一个测试不能算全量回归。

## ARM 交叉编译的状态

v0.1 只有 portable core 和 UnboundTransport，不含可烧录工程、toolchain 文件、HAL/CMSIS、启动文件、链接脚本或 `.ioc`。

已存在的 `-DROBOT_BUILD_HOST_TESTS=OFF` 只是不构建 host tests，仍然使用当前选中的编译器。它不是自动切换 ARM 或启用 STM32 的开关。不能运行这条就宣称“STM32 固件编译通过”。

未来需先落实实际工具链与板级配置，再在独立目录进行 cross-configure；可以先测试 portable core 在 ARM 编译器下能否编译，但不等于板级链接、启动、UART 或实机通过。完整命令只有等真实工具链文件和板卡证据存在后再填写。

## 保存可复核结果

每轮保存日期、操作系统、工具版本、源码版本/摘要、完整命令、退出码、日志和未执行项。模板见 `templates/LOCAL_DEVELOPMENT_REPORT.md`。不要覆盖原始 `firmware/docs/TEST_REPORT.zh-CN.md` 而丢失历史；为新增版本增加报告。
