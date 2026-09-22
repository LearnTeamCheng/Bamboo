# Bamboo

一个用 C++ 从零写的 **2D 游戏引擎**（学习与实用双目标），Windows / MSVC / OpenGL。

> **当前状态：早期开发中。渲染主链路可用，其余子系统多为骨架。**
> 完整的项目文档（现状 / 架构问题 / 路线图 / 游戏与编辑器计划 / 验收标准）在
> **[`docs/README.md`](docs/README.md)** —— 动手前请先读那份。

| | |
|---|---|
| 平台 | Windows only（MSVC / Visual Studio 2022） |
| 语言 | **C++20** |
| 图形 | OpenGL（glad 加载） |
| 构建 | CMake 3.20+ |
| 第三方 | 全部 vendored：GLFW / glad / entt / spdlog / stb / imgui |

---

## 1. 能做什么（诚实版）

| 模块 | 状态 |
|---|---|
| 窗口 / 上下文 | ✅ 可用 |
| ECS（entt + SystemRegistry 四阶段） | ✅ 可用 |
| 渲染（精灵，按 ZOrder 排序） | ⚠️ 可用但有坑 |
| 渲染（三角/四边形/圆） | ⚠️ 三角与四边形忽略传入的位置与尺寸；圆未实现 |
| 相机 | ⚠️ 像素相机；`SetOrthographic` 的 size 被覆盖 → 无法缩放 |
| 输入 | ⚠️ 仅 A~Z + 鼠标位置 |
| 资源加载 | ⚠️ 能加载；失败只打日志 |
| 物理 / 序列化 / 场景管理 | ❌ 未实现 |
| 编辑器 | ❌ 空壳（能开窗口，但绕过引擎且 ImGui 未接 GL3 后端） |
| UI 系统 / 音频 / C# 脚本 | ❌ 无 |

**现在只能用 C++ 直接写渲染与 ECS 代码跑出一个画面**；做不了"在编辑器里搭关卡"或"用脚本写逻辑"。

完整缺陷清单（P0~P5）见 [`docs/README.md` §5](docs/README.md#5-缺陷清单)。

---

## 2. 构建

**前置**：Windows 10/11 + Visual Studio 2022（勾选「使用 C++ 的桌面开发」）+ CMake 3.20+

### 命令行

```bat
cd Source
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

产物：

```
Source/build/bin/Debug/Sandbox.exe     ← 示例程序
Source/build/lib/Debug/Bamboo.lib      ← 引擎静态库
```

运行（工作目录必须是输出目录，原因见 §3.1）：

```bat
cd Source\build\bin\Debug
Sandbox.exe
```

### Visual Studio

打开 `Source/`（或 `Source/build/BambooEngine.sln`），选 `Sandbox`，F5。工作目录已由 CMake 自动设置。

### VS Code

仓库已含 `.vscode/` 配置，**打开仓库根目录**即可。

- 需要的扩展：`ms-vscode.cpptools`、`ms-vscode.cmake-tools`（打开时会提示安装）
- `Ctrl+Shift+P` → `CMake: Configure` → `CMake: Build`
- F5 → `Sandbox (Debug)`

**如果 IntelliSense 报「找不到头文件」**：

1. 先确认 `Source/build/generated/Config.h` 存在（它只在 CMake 配置成功后生成）
2. 确认装了 CMake Tools 扩展（配置里已开启 `CMAKE_EXPORT_COMPILE_COMMANDS`）
3. 确认打开的是**仓库根目录**（不是 `Source/`）
4. 如果换过 Visual Studio 版本，改 `.vscode/c_cpp_properties.json` 里 `compilerPath` 的 MSVC 版本号
5. 执行 `C/C++: Reset IntelliSense Database`

---

## 3. 三个必须知道的坑

### 3.1 资源路径是编译期烤进去的绝对路径

`Source/Bamboo/Config.in.h` 经 CMake `configure_file` 生成 `Source/build/generated/Config.h`，把 `BAMBOO_ASSET_ROOT` **写死成绝对路径**指向 `Source/BambooAssets/`。

**移动或克隆仓库后必须重新 configure**，否则资源加载失败（只打一条日志，表现是黑屏或白块）。

### 3.2 新增源文件后必须重新 configure

`Source/Bamboo/CMakeLists.txt` 与 `Sandbox/CMakeLists.txt` 用 `file(GLOB_RECURSE ...)` 收集源文件，**不会**自动感知新文件。加了 `.cpp`/`.h` 后要重跑 `CMake: Configure`。

### 3.3 构建目录是 `Source/build/`，不是仓库根的 `build/`

所有命令都在 `Source/` 下执行。`.gitignore` 已忽略 `Source/build/`。

---

## 4. 开发约定（摘要）

- **注释与提交信息用中文**
- **注释要少但可信**：错误注释比没有注释更糟。不写复述代码的注释，不留注释掉的代码
- **构建输出不入库**；**资源必须入库**（`Source/BambooAssets/`）
- **不要修改 `Source/ThirdParty/`**；加新依赖先在 `ThirdParty/CMakeLists.txt` 注册
- **提交信息**用 `类型(模块): 描述`，如 `fix(renderer): 修复 Quad 索引缓冲未初始化`
- **分层约束**：不要让渲染层依赖应用层（`Graphics/` 不应 include `Game/`）

完整约定见 [`docs/README.md` §15](docs/README.md#15-开发约定)。

---

## 5. 文档

| 文档 | 内容 |
|---|---|
| **[`docs/README.md`](docs/README.md)** | **唯一入口**：项目现状、架构五大根因、缺陷清单（P0~P5）、目标架构与迁移路径、游戏计划（G1~G5）、编辑器设计、C# 脚本约束、测试与验收标准、路线图、学习地图、待决策事项 |
| [`docs/research_csharp_embedding.md`](docs/research_csharp_embedding.md) | C# 嵌入技术调研（91 条引用）：.NET 版本、NativeAOT 限制、hostfxr 序列、可回收 ALC 护栏、可复制代码、未验证清单 |

## 6. 路线图（摘要）

按"游戏驱动引擎"推进：**每个游戏验收一个引擎版本**。

| 版本 | 发布条件（= 某个游戏走通） |
|---|---|
| v0.1.0 | Breakout 真能玩（逻辑用 C# 写） |
| v0.2.0 | 能只用鼠标在编辑器里摆出关卡并试玩 |
| v0.3.0 | 幸存者类：1,500 实体 60 FPS |
| v0.4.0 | 开屏/抽卡：整个界面在 UI 编辑器里搭出来 |
| v1.0.0 | SLG 战棋打包发布 |

详见 [`docs/README.md` §7](docs/README.md#7-游戏计划用游戏验收引擎版本) 与 [§12](docs/README.md#12-路线图与里程碑)。
