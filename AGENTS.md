# AGENTS.md

## 项目概述

Qbsidian 是一款基于 Qt 的本地 Markdown 笔记软件，借鉴 Obsidian 理念，提供双栏实时编辑预览、双向链接、知识图谱可视化、间隔重复复习规划与题目抽查练习等功能。这是大学《程序设计基础》课程的大作业项目，按四个阶段迭代开发。

**当前阶段**：阶段一 — 基础工作区（双栏编辑 + 文件管理）。

## 界面设计基准

### 三栏布局

水平 `QSplitter`（`childrenCollapsible: false`）：
- 左：文件树（`QFileSystemModel` + `QTreeView`）
- 中：`QPlainTextEdit` 编辑器
- 右：`QTextBrowser` 预览区

窗口默认 `showMaximized()`。

### 主题风格

Things 风格（参考 Things 3），支持**浅色/深色双主题**。主题色 `#2E80F2`，用户通过 `视图 > 切换浅色/深色`（`Ctrl+Shift+L`）切换，选择通过 `QSettings` 持久化。

**浅色模式**

| 控件 | 底色 | 关键样式 |
|------|------|----------|
| `QMainWindow` / `QPlainTextEdit` / `QTextBrowser` | `#ffffff` | 蓝青选区 `rgba(46,128,242,0.18)` |
| `QMenuBar` / `QStatusBar` / `QTreeView` | `#f6f7f8` | 底部/顶部分割线 `#ebedf0` |
| `QSplitter::handle` | `#ebedf0` | 1px 宽，hover 变 `#2E80F2` |
| `QMenu` | `#ffffff` | 圆角 8px，选中项蓝青背景，白字 |
| `QScrollBar` | `#f6f7f8` | 滑块 `#d4d4d4`，hover 变 `#bdbdbd` |
| `QPushButton` | `#2E80F2` | 圆角 6px 白字，hover `#1a6fd6` |
| `QLineEdit` | `#f6f7f8` | 边框 `#ebedf0`，focus 变 `#2E80F2` |
| `QTreeView::item` | 透明 | hover `#e2e5e9`，selected `#d4d4d4` |
| `QToolTip` | `#f6f7f8` | 边框 `#d4d4d4`，圆角 4px |

**深色模式**

| 控件 | 底色 | 关键样式 |
|------|------|----------|
| `QMainWindow` / `QPlainTextEdit` / `QTextBrowser` | `#1c2127` | 蓝青选区 `rgba(46,128,242,0.25)` |
| `QMenuBar` / `QStatusBar` / `QTreeView` | `#181c20` | 底部/顶部分割线 `#35393e` |
| `QSplitter::handle` | `#35393e` | 1px 宽，hover 变 `#2E80F2` |
| `QMenu` | `#1c2127` | 圆角 8px，选中项蓝青背景，白字 |
| `QScrollBar` | `#181c20` | 滑块 `#3f3f3f`，hover 变 `#555555` |
| `QPushButton` | `#2E80F2` | 圆角 6px 白字，hover `#1a6fd6` |
| `QLineEdit` | `#2c313c` | 边框 `#35393e`，focus 变 `#2E80F2` |
| `QTreeView::item` | 透明 | hover `#2c313c`，selected `#3f3f3f` |
| `QToolTip` | `#181c20` | 边框 `#35393e`，圆角 4px |

## 技术栈

- **语言**: C++17
- **UI 框架**: Qt 5/6 — Widgets 模块
- **构建系统**: CMake（最低 3.16）
- **文件编码**: UTF-8 无 BOM

## 分工（双人协作）

| 角色 | 负责人 | 职责范围 |
|------|--------|----------|
| **UI & 交互** | 我 | 界面布局、控件事件、信号/槽连接、对话框、标题栏/状态栏、编辑器与预览区操作 |
| **底层逻辑** | 合作者 | Markdown 解析渲染、本地文件读写、新建笔记/文件夹、编码处理、原子写入 |

**原则**：我不碰解析算法与文件 I/O 细节，合作者不碰界面布局与鼠标事件。

### 合作者提供的 API（直接 `#include` 调用即可）

- **`MarkdownParser::parse(QString) → QString`** — 传入 Markdown 源码，返回 HTML 片段（`<body>` 内部内容）
- **`NoteManager::load(path) → QString`** — 以 UTF-8 读取文件
- **`NoteManager::save(path, content) → bool`** — 原子写入保存，失败发射 `errorOccurred` 信号
- **`NoteManager::createNewNote(dir, name) → QString`** — 在指定目录创建 `.md` 文件，自动处理重名
- **`NoteManager::createNewFolder(dir, name) → QString`** — 新建文件夹
- **`NoteManager::exists(path) → bool`** — 检查路径是否存在

详细调用示例和参数说明见 `qbsidian_phase1_api_doc.md`。

## 常用命令

### 构建

```bash
cmake -B Qbsidian/build -S Qbsidian -DCMAKE_BUILD_TYPE=Debug
cmake --build Qbsidian/build
```

### 运行

```bash
open Qbsidian/build/Qbsidian.app   # macOS
```

### 清理

```bash
rm -rf Qbsidian/build
```

## 项目结构

```
Qbsidian/                          # 仓库根目录
├── README.md                      # 功能设计文档
├── qbsidian_phase1_api_doc.md     # 阶段一 API 文档（合作者编写）
└── Qbsidian/                      # 项目源码目录
    ├── CMakeLists.txt             # 构建配置（我来维护）
    ├── main.cpp                   # 入口
    ├── mainwindow.h/.cpp/.ui     # 主窗口（UI + 事件调度，我来写）
    ├── Qbsidian_zh_CN.ts          # 中文翻译文件
    ├── build/                     # 构建输出（gitignore）
    │
    └── 以下是合作者的文件，我 #include 但不应修改：
        ├── markdownparser.h/.cpp
        ├── blockparser.h/.cpp
        ├── inlineparser.h/.cpp
        ├── renderengine.h/.cpp
        └── notemanager.h/.cpp    （尚未创建）
```

### 我负责的 UI 文件（规划中 / 部分已创建）

| 文件 | 说明 |
|------|------|
| `mainwindow.h/.cpp/.ui` | 主窗口：菜单栏、状态栏、中央布局、关闭拦截、快捷键 |
| `fileexplorerpane.h/.cpp` | 文件树视图，绑定 `QFileSystemModel` |
| `editorpane.h/.cpp` | 左侧纯文本编辑区 |
| `previewpane.h/.cpp` | 右侧 Markdown 渲染预览区，通过 `QTextBrowser::setHtml()` 显示 |
| `linenumberarea.h/.cpp` | 编辑器行号区域 |

## 编码规范

- **include 守卫**: `#ifndef FOO_H` / `#define FOO_H` / `#endif // FOO_H`
- **类命名**: PascalCase，如 `MainWindow`、`FileExplorerPane`
- **文件命名**: 全小写，如 `fileexplorerpane.h`
- **Qt 信号/槽**: 使用 `connect(sender, &SenderClass::signal, receiver, &ReceiverClass::slot)` 新式语法
- **UI 文件**: 通过 Qt Designer 编辑 `.ui`，或纯代码构建布局
- **注释**: 不添加注释
- **缩进**: 4 空格
- **字符串**: 面向用户的中文字符串使用 `tr()` 包裹以确保翻译支持
- **主题**: Things 风格浅色/深色双主题，主题色 `#2E80F2`，用户可通过菜单或 `Ctrl+Shift+L` 切换，选择通过 `QSettings` 持久化

## 阶段一关键设计决策

1. **300ms 延迟渲染** — 编辑器 `textChanged()` 信号通过 `QTimer::singleShot` 防抖后再触发预览更新
2. **HTML 包裹** — `MarkdownParser::parse()` 只返回 `<body>` 内部 HTML，需自行包裹 `<style>` + `<div>` 后调用 `setHtml()`
3. **原子保存** — 调用 `NoteManager::save()`（内部已用 `QSaveFile`），只需处理返回值
4. **未保存拦截** — 在 `closeEvent()` 中检查 `m_isModified`，弹出 Save/Discard/Cancel 对话框
5. **Ctrl+S** — 绑定 `QAction`，触发后调用 `NoteManager::save()`

## 注意事项

1. 合作者的文件（`.h`/`.cpp`）只 `#include`，不应修改
2. 新增 `.cpp`/`.h` 文件后需同步更新 `CMakeLists.txt` 的 `PROJECT_SOURCES`
3. 项目兼容 Qt 5 和 Qt 6，避免使用版本专有 API
4. 所有文件操作路径必须使用绝对路径
5. `BlockType` 和 `LogicalBlock` 定义在 `blockparser.h` 中
