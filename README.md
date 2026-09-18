# Bamboo

一个用 C++ 从零写的 **2D 游戏引擎**（学习 + 实用双目标），Windows / MSVC / OpenGL。

> **当前状态：早期开发中，渲染主链路可用，其余子系统多为骨架。**
> 下面「当前能力」一节逐项说明哪些能用、哪些不能用——**请先看那张表再动手**，避免踩空。

| | |
|---|---|
| 平台 | Windows only（MSVC / Visual Studio 2022） |
| 语言 | C++17 |
| 图形 | OpenGL（通过 glad 加载） |
| 构建 | CMake 3.20+ |
| 第三方 | 全部 vendored 在 `Source/ThirdParty/`（GLFW / glad / entt / spdlog / stb / imgui） |

---

## 1. 当前能力（诚实版）

| 模块 | 状态 | 说明 |
|---|---|---|
| 窗口 / 上下文 | ✅ 可用 | GLFW 窗口 + OpenGL 上下文；关闭/缩放事件可用 |
| ECS | ✅ 可用 | 基于 `entt`；`Entity` + 组件 + `SystemRegistry`（四阶段路由） |
| 渲染（精灵） | ⚠️ 可用但有坑 | 2D 批次渲染，精灵按 ZOrder 排序。**已知缺陷**见第 5 节 |
| 渲染（三角/四边形/圆） | ⚠️ 有缺陷 | 三角/四边形的绘制**忽略传入的位置与尺寸**；圆尚未实现 |
| 相机 | ⚠️ 部分 | 像素相机（1 世界单位 = 1 像素）。**`SetOrthographic` 传入的 size 会被内部覆盖**，无法缩放 |
| 输入 | ⚠️ 部分 | `A`~`Z` 键 + 鼠标位置；**没有**鼠标按键/滚轮/方向键/数字键 |
| 资源加载 | ⚠️ 部分 | 贴图/着色器能从 `BambooAssets/` 加载；加载失败只打日志 |
| 物理 | ❌ 未实现 | `PhysicsSystem::Update` 是空函数；碰撞体组件无人读取 |
| 场景序列化 | ❌ 未实现 | `SceneSerializer` 是空实现，场景无法存盘 |
| 场景管理 | ❌ 未实现 | `SceneManager::LoadScene(name)` 忽略参数，永远新建空场景 |
| 编辑器 | ❌ 空壳 | `Editor.exe` 能启动窗口，但**绕过引擎**且 ImGui 未接 GL3 后端，实际渲染不出内容 |
| UI 系统 | ❌ 空壳 | `Source/Bamboo/UI/` 大部分未实现 |
| 音频 | ❌ 无 | |
| C# 脚本 | ❌ 无 | 计划中（见 `docs/games_and_editor.md`） |

**结论**：现在能做的只有**用 C++ 直接写渲染/ECS 代码**跑出一个画面；做不了「在编辑器里搭关卡」或「用脚本写逻辑」。

---

## 2. 快速开始

### 2.1 前置要求

- Windows 10/11
- **Visual Studio 2022**，安装时勾选「使用 C++ 的桌面开发」工作负载
- **CMake 3.20+**（VS 自带一份，或单独安装）

### 2.2 构建（命令行）

```bat
cd Source
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

产物位置：

```
Source/build/bin/Debug/Sandbox.exe     ← 示例程序
Source/build/lib/Debug/Bamboo.lib      ← 引擎静态库
```

运行 `Sandbox.exe` 需要工作目录为它的所在目录（资源路径依赖见第 4 节）：

```bat
cd Source\build\bin\Debug
Sandbox.exe
```

### 2.3 构建（Visual Studio）

用 VS 打开 `Source/` 目录（或 `Source/build/BambooEngine.sln`），选 `Sandbox` 为目标，F5 运行。`Sandbox` 的调试工作目录已由 CMake 自动设为输出目录。

### 2.4 构建（VS Code）

仓库已包含 `.vscode/` 配置，直接用 VS Code 打开**仓库根目录**即可：

- 需要扩展：`ms-vscode.cpptools`（C/C++）、`ms-vscode.cmake-tools`（CMake Tools）——打开时会自动提示安装
- `Ctrl+Shift+P` → `CMake: Configure` → `CMake: Build`
- F5 选择 `Sandbox (Debug)` 直接调试

> **如果 IntelliSense 报「找不到头文件」**，见第 4.4 节。

---

## 3. 项目结构

```
Bamboo/
├── Source/
│   ├── CMakeLists.txt            # 顶层构建脚本
│   ├── Bamboo/                   # 引擎静态库（核心）
│   │   ├── Core/                 # Ref / Log / Assert / Time / UUID / Input / KeyCodes
│   │   ├── Math/                 # Vector / Matrix / Color / AABB / Rect
│   │   ├── ECS/                  # Entity / Component / System / SystemRegistry
│   │   ├── Scene/                # Scene / SceneManager（+ 空实现的序列化器）
│   │   ├── Graphics/             # Renderer2D / Camera / Shader / Texture / ...
│   │   ├── GraphicsAPI/OpenGL/   # 上述接口的 OpenGL 实现
│   │   ├── Physics/              # 物理（未实现）
│   │   ├── Assets/               # 资源加载
│   │   ├── UI/                   # UI（空壳）
│   │   ├── Game/                 # Application / Window
│   │   └── Platform/Windows/     # 平台实现
│   ├── BambooAssets/             # 引擎资源（Shaders/ Texture2d/）
│   ├── Sandbox/                  # 示例程序（Breakout）
│   ├── Editor/                   # 编辑器（空壳）
│   ├── ThirdParty/               # 第三方库（vendored，请勿修改）
│   └── build/                    # 构建输出（不入库）
├── docs/                         # 文档（见第 6 节）
├── AGENTS.md                     # 给 AI 协作者的约定说明
└── .gitignore
```

架构现状（分层、启动顺序、系统阶段、关键不变量）见 **[`docs/architecture.md`](docs/architecture.md)**。

---

## 4. 四个必须知道的坑

### 4.1 资源路径是**编译期烤进去的绝对路径**

`Source/Bamboo/Config.in.h` 会被 CMake `configure_file` 成 `Source/build/generated/Config.h`，把 `BAMBOO_ASSET_ROOT` **写死成绝对路径**指向 `Source/BambooAssets/`。

**后果**：把仓库移动/克隆到别处后，**必须重新 configure CMake**，否则资源加载失败（而且只会打一条日志，表现是黑屏或白块）。

### 4.2 新增源文件后必须**重新 configure**

`Source/Bamboo/CMakeLists.txt` 和 `Sandbox/CMakeLists.txt` 用 `file(GLOB_RECURSE ...)` 收集源文件，**不会**自动感知新文件。加了新的 `.cpp`/`.h` 后要重新跑一次 `CMake: Configure`（或 `cmake -S . -B build`）。

> 这条属于已知设计缺陷，计划改用 `CONFIGURE_DEPENDS` 或显式列文件（见 `docs/refactor_plan.md` P4-4）。

### 4.3 构建目录是 `Source/build/`，不是仓库根的 `build/`

所有命令都要在 `Source/` 下执行。`.gitignore` 已忽略 `Source/build/`。

### 4.4 VS Code IntelliSense 报「找不到头文件」

`.vscode/c_cpp_properties.json` 已经配好了 MSVC + 所有第三方 include 根。如果你仍然看到红色的 `#include errors detected`，按顺序检查：

1. **`Source/build/generated/Config.h` 是否存在** —— 它只在 CMake 配置成功后生成。没配置过就先跑 `CMake: Configure`。
2. **`Source/ThirdParty/entt/include` 等目录是否存在** —— 确认第三方库完整。
3. **`C_Cpp.default.configurationProvider` 是否生效** —— 需要装 CMake Tools 扩展。装了之后建议用 `CMake: Configure` 走一遍，让扩展拿到真实的编译参数（配置里已开启 `CMAKE_EXPORT_COMPILE_COMMANDS`）。
4. **MSVC 版本路径** —— `c_cpp_properties.json` 里的 `compilerPath` 指向 `14.44.35207`。如果你装的 VS 版本不同，改成自己的路径（在 `C:\Program Files\Microsoft Visual Studio\2022\<版本>\VC\Tools\MSVC\` 下）。
5. **打开的是仓库根目录** —— 不是 `Source/`。配置里的 `${workspaceFolder}` 按仓库根解析。

改完执行一次：`Ctrl+Shift+P` → `C/C++: Reset IntelliSense Database`。

---

## 5. 已知重要缺陷

引擎目前有一批**会让渲染结果不正确**的问题，动手前请先读 `docs/refactor_plan.md`（按 P0~P5 分级，逐项带 `文件:行`）。最需要知道的几条：

| 问题 | 影响 |
|---|---|
| `DrawQuad` / `DrawTriangle` 忽略传入的位置与尺寸 | 传什么参数都一样，画在固定位置 |
| `DrawTriangle` 里硬编码 `/1280`、`/720` | 换分辨率就错位 |
| Quad 的索引缓冲用未初始化数据上传 | 一旦用到 Quad 就可能越界读取 |
| 精灵数量超上限（100）无检查 | 超过就静默写穿内存 |
| `Camera::SetOrthographic` 的 size 被内部覆盖 | 无法缩放/变焦 |
| `Entity` 默认构造后调用组件方法会崩 | `FindEntityByName` 找不到时的返回值直接崩 |
| `BAMBOO_ASSERT` 失败只打日志**不中断** | 等于没有断言 |
| 渲染系统在渲染期**回写** ECS 数据 | 破坏"渲染只读"契约 |

---

## 6. 文档导航

| 文档 | 内容 | 什么时候看 |
|---|---|---|
| [`docs/architecture.md`](docs/architecture.md) | **现状架构**：分层、启动/帧循环、系统阶段与执行顺序、组件清单、所有权、关键不变量 | 想知道"代码现在是什么样" |
| [`docs/refactor_plan.md`](docs/refactor_plan.md) | **缺陷体检报告**（P0~P5）+ S0~S8 修复阶段 | 修 bug、判断改动是否安全 |
| [`docs/upgrade_plan.md`](docs/upgrade_plan.md) | **施工蓝图**：七大模块怎么建 + R0~R7 阶段 + 验收标准 | 决定"下一步做什么" |
| [`docs/games_and_editor.md`](docs/games_and_editor.md) | **游戏计划 + 编辑器设计**：G1~G5 游戏阶梯、Unity 式编辑器布局与工作流 | 决定"先做哪个游戏 / 编辑器怎么做" |
| [`docs/research_csharp_embedding.md`](docs/research_csharp_embedding.md) | C# 嵌入技术调研（.NET 版本、NativeAOT 限制、hostfxr、可回收 ALC 护栏） | 做脚本系统之前 |
| [`docs/roadmap.md`](docs/roadmap.md) | 学习路线（每个 bug 对应哪个 C++ 知识点）。⚠️ 行号已过期 | 想学东西的时候 |

---

## 7. 开发约定

- **注释与提交信息用中文**（与既有代码保持一致）。
- **构建输出不入库**：`Source/build/` 已在 `.gitignore` 中。构建产物也不要手动拷进仓库。
- **资源必须入库**：`Source/BambooAssets/` 下的着色器与贴图是引擎运行必需的，不要忽略。
- **不要修改 `Source/ThirdParty/`**：那是 vendored 的第三方源码。要加新依赖请先在 `Source/ThirdParty/CMakeLists.txt` 注册。
- **提交信息**建议用 `类型(模块): 描述` 格式，例如 `fix(renderer): 修复 Quad 索引缓冲未初始化`。
- **分层约束**：新代码不要让渲染层依赖应用层（`Graphics/` 不应 include `Game/`）。详见 `docs/architecture.md` §2 的违规清单。

---

## 8. 路线图（摘要）

按"游戏驱动引擎"的方式推进：**每个游戏验收一个引擎版本**。

| 版本 | 发布条件（= 某个游戏走通） |
|---|---|
| v0.1.0 | Breakout 真能玩（游戏逻辑用 C# 写） |
| v0.2.0 | 能只用鼠标在编辑器里摆出关卡并试玩 |
| v0.3.0 | 幸存者类：1,500 实体 60 FPS |
| v0.4.0 | 开屏/抽卡游戏：整个界面在 UI 编辑器里搭出来 |
| v1.0.0 | SLG 战棋打包发布 |

详细的阶段、任务与验收标准见 [`docs/games_and_editor.md`](docs/games_and_editor.md) 与 [`docs/upgrade_plan.md`](docs/upgrade_plan.md)。
