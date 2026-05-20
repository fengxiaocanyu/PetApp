# PetApp 桌宠系统

Windows 原生桌面桌宠应用程序，具备可自定义形象、模块化架构、离线优先、可移植（拷贝即用）等特性。

## 项目结构

```
PetApp/
├── CMakeLists.txt              # CMake 构建配置
├── README.md                   # 本文件
├── src/                        # 源代码
│   ├── PlatformDefines.h       # Windows 宏保护
│   ├── main.cpp                # 入口文件
│   ├── core/                   # 核心模块
│   │   ├── EventBus.h/cpp      # 事件总线（单例）
│   │   ├── MemoryModule.h/cpp  # KV 存储模块
│   │   ├── HeartbeatModule.h/cpp # 心跳模块
│   │   ├── EmotionModule.h/cpp # 情感模块
│   │   ├── CapabilityManager.h/cpp # 能力管理器
│   │   └── SkillManager.h/cpp  # 技能管理器
│   ├── ui/                     # UI 模块
│   │   ├── MainWindow.h/cpp    # 主窗口
│   │   ├── BubbleWidget.h/cpp  # 气泡部件
│   │   └── SettingsDialog.h/cpp # 设置对话框
│   ├── appearance/             # 外观模块
│   │   └── AppearanceManager.h/cpp
│   ├── action/                 # 动作模块
│   │   └── ActionManager.h/cpp
│   └── voice/                  # 语音模块
│       └── VoiceManager.h/cpp
├── skills/                     # 技能插件源码
│   ├── skill_echo/             # 回显技能
│   ├── skill_translate/        # 翻译技能
│   ├── skill_ocr/              # OCR 技能
│   └── skill_chat/             # 对话技能
├── config/                     # 配置文件
│   └── default_settings.json
├── data/                       # 运行时数据
│   └── memory/
├── plugins/                    # 编译后的技能 exe
├── logs/                       # 运行时日志
└── scripts/                    # 工具脚本
    └── package.ps1             # 打包脚本
```

## 编译要求

- **操作系统**: Windows 10/11 (x86_64)
- **编译器**: Visual Studio 2022 (MSVC v143)
- **CMake**: 3.22+
- **Ninja**: 构建系统
- **Qt**: 6.8.3 (msvc2022_64)

## 编译步骤

### 1. 打开开发者命令提示符

```bash
call "D:\software\work\VS2022\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
```

### 2. 配置项目

```bash
cd D:\github\work\PetApp
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="E:/work/QT/6.8.3/msvc2022_64"
```

### 3. 编译

```bash
cmake --build build
```

### 4. 运行

```bash
.\PetApp.exe
```

## 打包

使用 PowerShell 打包脚本：

```powershell
.\scripts\package.ps1
```

脚本将：
1. 编译 Release 版本
2. 使用 windeployqt 拷贝 Qt 运行时
3. 复制 VC++ 运行时 DLL
4. 复制 plugins/、data/、config/ 目录
5. 打包为 PetApp_v1.0.0.zip

## 技能插件协议

技能插件为独立控制台 exe，通过 stdin/stdout 使用 JSON 行协议通信。

### 请求格式

```json
{"method":"execute","id":1,"params":{"input":"...","config":{...}}}
```

### 响应格式（成功）

```json
{"id":1,"result":"处理结果","error":null}
```

### 响应格式（失败）

```json
{"id":1,"result":null,"error":{"code":-1,"message":"错误信息"}}
```

## 模块架构

```
┌─────────────────────────────────────────────────┐
│              表现层（Appearance & Output）       │
│  ┌──────────────┬──────────────┬─────────────┐ │
│  │AppearanceMgr │ VoiceMgr     │ ActionMgr   │ │
│  └──────────────┴──────────────┴─────────────┘ │
└─────────────────────────────────────────────────┘
                        ▲ 事件总线
┌─────────────────────────────────────────────────┐
│              决策与核心逻辑（Core）              │
│  ┌──────────┬──────────┬──────────┬──────────┐ │
│  │MemoryMod │Heartbeat │EmotionMod│Capability│ │
│  └──────────┴──────────┴──────────┴──────────┘ │
└─────────────────────────────────────────────────┘
                        ▲ 事件总线
┌─────────────────────────────────────────────────┐
│              技能插件层（Skills）                │
│  ┌──────────────────────────────────────────┐   │
│  │ SkillManager → 启动子进程 → skill_*.exe   │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
```

## 离线优先

- 系统默认关闭网络功能
- 所有内置技能（翻译、OCR、对话）有本地实现
- 提供明确的网络开关，关闭后禁止任何外部网络请求
- 自适应降级：检测网络状态，自动禁用依赖网络的技能

## 许可证

LGPL v3