# PetApp 桌宠系统 - 问题分析与改善报告

## 概述

本报告基于对 PetApp 项目源码的全面审查，结合两个参考项目的对比分析：
1. **轻音少女桌宠**（Mascot） - Win32 原生桌宠，帧动画 + 系统托盘
2. **Yosuga 前端**（Misakityan/Yosuga） - C++20 + Qt6.6.3 + Live2D SDK 高端桌宠

---

## 一、核心问题清单

### 问题 1：部件对齐与坐标系统错误（严重）

**现象**：角色部件无法正确对齐，贴图位置错乱。

**根因分析**：

`CharacterPart::setOffset()` 调用 `setPos(m_offset)` 设置部件位置，但 `boundingRect()` 返回的是 `QRectF(0, 0, pixmap.width(), pixmap.height())`，即贴图左上角在 (0,0)。这意味着：

1. **偏移量是部件的左上角坐标**，而不是部件的中心坐标
2. 当贴图缩放后尺寸变化，`boundingRect()` 返回的尺寸与偏移量不匹配
3. 眼睛、嘴巴等小部件的位置偏移是基于固定数值硬编码的，没有考虑贴图实际尺寸

**改善方案**：

```cpp
// 方案：创建部件时根据贴图实际尺寸调整偏移，使部件居中
auto createPartWithPixmap = [&](const QString& filename, PartType type, 
                                 const QPointF& centerOffset) -> CharacterPart* {
    QPixmap pix = loadAndScalePixmap(filename);
    CharacterPart* part = new CharacterPart(type);
    if (!pix.isNull()) {
        part->setPixmap(pix);
        // 根据贴图实际尺寸调整偏移，使部件中心对齐
        qreal adjustedX = centerOffset.x() - pix.width() / 2.0;
        qreal adjustedY = centerOffset.y() - pix.height() / 2.0;
        part->setOffset(QPointF(adjustedX, adjustedY));
    }
    return part;
};
```

---

### 问题 2：窗口拖动功能失效（严重）

**现象**：无法通过鼠标拖动桌宠窗口。

**根因分析**：

`MainWindow::mousePressEvent()` 中判断点击是否在角色区域内，但 `characterBoundingRect()` 返回的是**场景坐标**中的边界，而 `widgetPos` 是**部件坐标**，两者坐标系不同。

**改善方案**：

```cpp
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 直接启动拖动，不判断角色区域
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
        event->accept();
    }
}
```

---

### 问题 3：角色点击事件与窗口拖动冲突（严重）

**现象**：点击角色时可能触发窗口拖动，或者点击角色外部时无法拖动。

**根因分析**：

`QGraphicsView` 会拦截鼠标事件并转发给 `QGraphicsScene`，所以 `MainWindow::mousePressEvent` 可能根本收不到点击角色区域的事件。但当前代码中 `QGraphicsView` 设置了 `setAttribute(Qt::WA_TransparentForMouseEvents, false)`，导致事件处理混乱。

**改善方案**：

让 `QGraphicsView` 处理所有鼠标事件，MainWindow 只处理非角色区域的点击。

---

### 问题 4：气泡定位错误（中等）

**现象**：气泡显示位置不正确。

**根因分析**：

`MainWindow::showBubble()` 中气泡位置硬编码为 `(10, -10)`，而 `BubbleWidget::showMessage()` 中又尝试定位到父窗口底部中间，两处定位逻辑冲突。

**改善方案**：

统一气泡定位逻辑，在 `MainWindow::showBubble()` 中计算相对于角色头部的位置。

---

### 问题 5：表情切换未实际更换贴图（中等）

**现象**：`setState()` 切换表情时，没有实际更换嘴部和眼睛的贴图。

**根因分析**：

`CharacterWidget::setState()` 中表情切换只有注释，没有实际更换贴图的代码。

**改善方案**：

1. 在 `CharacterPart` 中添加动态切换贴图的方法
2. 在 `CharacterWidget` 中加载所有表情变体
3. 在 `setState()` 中实际切换贴图

---

### 问题 6：窗口穿透与点击区域问题（中等）

**现象**：点击桌宠时可能穿透到桌面或其他窗口。

**改善方案**：

设置场景背景透明，让 `QGraphicsView` 忽略空白区域的点击，但让角色部件接收事件。

---

### 问题 7：缺少屏幕边缘吸附功能（低）

**现象**：桌宠可以拖动到屏幕任意位置，不会自动吸附到边缘。

**参考项目特征**：轻音少女桌宠的 `FixedPos` 字符串表明支持位置固定/吸附功能。

**改善方案**：

在 `MainWindow::mouseMoveEvent` 中添加边缘吸附逻辑。

---

### 问题 8：缺少双击交互（低）

**现象**：双击桌宠没有反应。

**改善方案**：

在 `CharacterPart` 中添加双击事件处理，双击切换随机动作。

---

### 问题 9：缺少右键菜单（低）

**现象**：右键点击桌宠没有菜单。

**改善方案**：

在 `CharacterPart` 中添加右键事件处理，显示设置/隐藏/退出菜单。

---

### 问题 10：缺少角色状态持久化（低）

**现象**：关闭程序后，桌宠位置和状态不保存。

**改善方案**：

在 `MainWindow` 关闭时保存位置到 `MemoryModule`，启动时恢复。

---

## 二、与参考项目的对比分析

### 2.1 与轻音少女桌宠对比

| 功能特性 | 轻音少女桌宠 | PetApp（当前） | 差距 |
|---------|-------------|---------------|------|
| 透明窗口 | ✅ Win32 透明窗口 | ✅ Qt 透明窗口 | 基本一致 |
| 系统托盘 | ✅ TrayIcon | ✅ TrayManager | 基本一致 |
| 帧动画 | ✅ 内嵌资源动画 | ✅ 部件贴图动画 | 实现方式不同 |
| 屏幕边缘吸附 | ✅ FixedPos | ❌ 未实现 | 需添加 |
| 位置持久化 | ✅ 推测支持 | ❌ 未实现 | 需添加 |
| 右键菜单 | ✅ 推测支持 | ❌ 未实现 | 需添加 |
| 双击交互 | ✅ 推测支持 | ❌ 未实现 | 需添加 |
| 多角色切换 | ✅ 独立 exe | ❌ 单角色 | 架构差异 |
| 资源打包 | ✅ ARCHIVEX 内嵌 | ❌ 外部文件 | 各有优劣 |
| 部件点击交互 | ❌ 整体点击 | ✅ 分部件点击 | PetApp 更优 |
| 技能插件系统 | ❌ 无 | ✅ 完整实现 | PetApp 更优 |
| 情感系统 | ❌ 无 | ✅ EmotionModule | PetApp 更优 |
| 事件总线 | ❌ 无 | ✅ EventBus | PetApp 更优 |
| 离线自适应 | ❌ 无 | ✅ CapabilityManager | PetApp 更优 |

### 2.2 与 Yosuga 前端项目对比

| 功能特性 | Yosuga（参考） | PetApp（当前） | 差距 |
|---------|---------------|---------------|------|
| 渲染引擎 | ✅ Live2D SDK（OpenGL） | ❌ QGraphicsScene 贴图 | Yosuga 更优 |
| 角色模型 | ✅ Live2D 动态模型 | ❌ 静态 PNG 部件 | Yosuga 更优 |
| 动画系统 | ✅ Live2D 骨骼动画 | ❌ 帧序列/部件动画 | Yosuga 更优 |
| 语音交互 | ✅ TTS + ASR（服务端） | ✅ TTS（本地） | 各有优劣 |
| AI 对话 | ✅ LLM（服务端） | ❌ 预定义问答 | Yosuga 更优 |
| 技能插件 | ❌ 无 | ✅ 独立 exe 插件 | PetApp 更优 |
| 情感系统 | ❌ 无 | ✅ EmotionModule | PetApp 更优 |
| 事件总线 | ❌ 无 | ✅ EventBus | PetApp 更优 |
| 离线自适应 | ❌ 依赖服务端 | ✅ 完全离线 | PetApp 更优 |
| 跨平台 | ✅ Linux/Windows | ❌ Windows only | Yosuga 更优 |
| 嵌入式控制 | ✅ 串口/WebSocket | ❌ 无 | Yosuga 更优 |
| 构建系统 | ✅ CMake 3.30 + C++20 | ✅ CMake 3.22 + C++17 | 基本一致 |
| 第三方库 | ✅ ElaWidgetTools, autogui-cpp | ❌ 无 | Yosuga 更丰富 |

### 2.3 关键架构差异

**Yosuga 的优势**：
1. **Live2D 渲染**：使用专业的 Live2D Cubism SDK，支持骨骼动画、表情融合、物理模拟
2. **AI 驱动**：后端连接 LLM（大语言模型），支持自然语言对话
3. **跨平台**：支持 Windows 和 Linux
4. **嵌入式扩展**：通过串口/WebSocket 控制外部设备
5. **现代化 UI**：使用 ElaWidgetTools 库提供美观的界面

**PetApp 的优势**：
1. **完全离线**：不依赖任何服务端，所有功能本地运行
2. **技能插件系统**：灵活的 exe 插件架构，可扩展性强
3. **模块化设计**：事件总线 + 模块解耦，架构清晰
4. **轻量级**：无需 Live2D SDK 等大型依赖，部署简单
5. **情感系统**：内置三维情感向量，可驱动角色行为

---

## 三、改善优先级建议

### 紧急修复（影响基本使用）

| 优先级 | 问题 | 影响 |
|--------|------|------|
| P0 | 问题 1：部件对齐错误 | 角色显示异常 |
| P0 | 问题 2：窗口拖动失效 | 无法移动桌宠 |
| P0 | 问题 3：点击与拖动冲突 | 交互混乱 |

### 重要改进（提升用户体验）

| 优先级 | 问题 | 影响 |
|--------|------|------|
| P1 | 问题 4：气泡定位错误 | 气泡显示位置不对 |
| P1 | 问题 5：表情切换未实现 | 表情变化无效果 |
| P1 | 问题 6：窗口穿透问题 | 点击体验差 |

### 增强功能（对标参考项目）

| 优先级 | 问题 | 影响 |
|--------|------|------|
| P2 | 问题 7：屏幕边缘吸附 | 缺少桌宠特色功能 |
| P2 | 问题 8：双击交互 | 交互单一 |
| P2 | 问题 9：右键菜单 | 操作不便 |
| P2 | 问题 10：位置持久化 | 每次启动位置重置 |

---

## 四、总结

### 4.1 当前问题

PetApp 在架构设计（事件总线、模块解耦、技能插件系统、情感系统）上优于两个参考项目，但在**基础交互体验**上存在多个严重问题：

1. **部件对齐错误**是最核心的问题，需要优先修复坐标计算逻辑
2. **窗口拖动失效**和**点击冲突**是第二优先级，影响基本使用
3. **表情切换未实现**导致情感系统无法在视觉上体现

### 4.2 与 Yosuga 的定位差异

PetApp 和 Yosuga 的定位不同：
- **Yosuga**：高端 AI 桌宠，依赖服务端，使用 Live2D 专业渲染，适合有 AI 服务器资源的用户
- **PetApp**：轻量级离线桌宠，插件化架构，适合普通用户即装即用

PetApp 不需要追求 Live2D 级别的渲染效果，但需要**修复基础交互问题**，确保桌宠能正常显示、拖动和交互。

### 4.3 建议修复顺序

1. 先修复 P0 问题（部件对齐、窗口拖动、点击冲突）
2. 再修复 P1 问题（气泡定位、表情切换、窗口穿透）
3. 最后添加 P2 增强功能（边缘吸附、双击、右键菜单、位置持久化）

修复 P0 问题后即可让桌宠正常显示和交互。