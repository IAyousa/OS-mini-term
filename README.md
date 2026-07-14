# 操作系统模拟器 — 课程设计

多道程序设计方法的单用户操作系统模拟，包含**进程管理、存储管理、设备管理、文件管理**四大模块，基于 EasyX 图形界面交互。

## 环境要求

| 依赖 | 说明 |
|------|------|
| **操作系统** | Windows 7+（EasyX 仅支持 Windows） |
| **编译器** | MinGW-w64 (g++ 4.9+) |
| **图形库** | [EasyX](https://easyx.cn/) 2022 以上版本（MinGW 移植版） |
| **构建工具** | GNU Make（MinGW 自带 `mingw32-make`） |

> 💡 本项目使用 Dev-C++ 自带的 MinGW64 工具链开发。
> 如果没有 EasyX，可自行从官网下载安装到 MinGW 的 include/lib 目录。

## 快速开始

```bash
# 1. 进入项目目录
cd OS-mini-term

# 2. 编译（使用 Makefile）
mingw32-make

# 3. 运行
os_simulator/src/main.exe
```

> 也可以直接用 `g++` 手动编译：
> ```bash
> g++ -std=c++11 -g -fpermissive -Ios_simulator/include \
>   os_simulator/src/main.cpp \
>   os_simulator/src/device/device_manager.cpp \
>   os_simulator/src/device/device_graphics.cpp \
>   os_simulator/src/file/disk_scheduler.cpp \
>   os_simulator/src/file/disk_graphics.cpp \
>   os_simulator/src/memory/memory_manager.cpp \
>   os_simulator/src/memory/memory_graphics.cpp \
>   os_simulator/src/process/process_manager.cpp \
>   os_simulator/src/process/process_graphics.cpp \
>   -o os_simulator/src/main.exe -leasyx
> ```

## 操作方式

| 操作 | 功能 |
|------|------|
| `↑` `↓` | 在菜单项间移动选择 |
| `Enter` | 确认选择 |
| `1`~`4` / `0` | 数字快捷键（主菜单/子菜单） |
| `ESC` | 返回上级菜单 / 退出 |
| `←` `→` | 步进/回退（动画演示模式） |

## 模块说明

### 1️⃣ 进程管理
- **生产者-消费者问题模拟** — 信号量 PV 操作，可视化缓冲池变化，可步进/自动播放
- **先来先服务 (FCFS)** — 按到达顺序调度，输出甘特图+统计表
- **短作业优先 (SJF)** — 按服务时间调度，输出甘特图+统计表
- **最高响应比优先 (HRRN)** — 动态计算响应比，输出甘特图+统计表
- **动态优先级调度 (时间片轮转)** — 每时间片优先级减 1，可逐步回放 PCB 状态变化

### 2️⃣ 存储管理
- 生成随机指令地址流
- **先进先出 (FIFO)** 页面淘汰算法模拟
- **最近最少使用 (LRU)** 页面淘汰算法模拟
- 可调整物理内存块数（3~7 块）

### 3️⃣ 设备管理
- **银行家算法** 避免死锁
- 支持自定义进程数 M（1~10）和资源种类数 N（1~5）
- 安全检查 + 安全序列输出
- 按 R 重置系统

### 4️⃣ 文件管理 — 磁盘调度
- 四种磁盘调度算法可视化：
  - **FCFS** 先来先服务
  - **SSTF** 最短寻道时间优先
  - **SCAN** 扫描算法（电梯算法）
  - **CSCAN** 循环扫描算法
- 动画逐段绘制寻道轨迹

## 项目结构

```
OS-mini-term/
├── Makefile                          # 编译配置（入口）
├── README.md
├── os_simulator/
│   ├── include/                      # 头文件
│   │   ├── graphics_utils.h          # EasyX 通用工具（drawText, textWidth, clearWhite...）
│   │   ├── device_manager.h          # 银行家算法数据结构
│   │   ├── disk_scheduler.h          # 磁盘调度算法接口
│   │   ├── memory_manager.h          # 存储管理数据结构
│   │   └── process_manager.h         # 进程管理数据结构
│   ├── src/
│       ├── main.cpp                  # 入口 + 主菜单
│       ├── device/                   # 设备管理
│       ├── file/                     # 文件管理（磁盘调度）
│       ├── memory/                   # 存储管理
│       └── process/                  # 进程管理
```

## 开发

```bash
# 编译
mingw32-make

# 清理
mingw32-make clean

# 直接运行
os_simulator/src/main.exe
```

> Makefile 使用 `$(wildcard)` 自动扫描各子目录下的 `.cpp` 文件，新增源文件无需修改 Makefile。
