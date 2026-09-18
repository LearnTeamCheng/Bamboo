# Bamboo 引擎开发路线图

> 目标：以"学透 C++、拆开引擎黑盒"为第一目的，与引擎共同进化。
> 原则：**每修一个 bug 都要想清它对应的 C++ 知识点**；每完成一个阶段都要有可运行的 demo 和性能数据验证；进度慢没关系，卡住的地方就是成长的地方。

---

> ## ⚠️ 状态提示（2026-09-18 核对）
>
> 本文档是**学习路线**，不是当前工作清单。使用前请注意：
>
> 1. **所有 `文件:行` 引用都已过期**（例如文中说的 `Renderer2D.cpp:150` 现在是一个 `{`）。请把它当"知识点索引"看，别按行号跳转。
> 2. **缺陷的权威清单在 `docs/refactor_plan.md`**（编号 P0-x ~ P5-x，逐项带文件:行）。本文提到的 bug 请去那里交叉核对当前状态。
> 3. 已核对**已修复**的项（截至 2026-09-18）：
>    - `Singleton.h` 的 `= delete` 缺分号 —— 已加 ✅
>    - `BallComponent.h` 缺 `#pragma once` —— 已加 ✅
>    - `STB_IMAGE_IMPLEMENTATION` 重复定义（ODR）—— 现在只有 `OpenGLTexture.cpp` 一处 ✅
>    - 精灵数量超上限无检查 / Quad 索引未初始化 —— **仍然存在** ❌（`refactor_plan.md` P0-2、P0-4）
>    - `Camera::ReCalculateProjectionMatrix` 覆盖 `SetOrthographic` 的 size —— **仍然存在** ❌（P1-6）
>    - `BAMBOO_ASSERT` 失败只打日志不中断 —— **仍然存在** ❌（P0-5）
> 4. 架构现状（分层、启动/帧循环、系统阶段与执行顺序）见 `docs/architecture.md`。
> 5. 实际执行顺序（先做什么）见 `docs/games_and_editor.md` §4 与 §12.4。

---

## 目录

- [阶段总览](#阶段总览)
- [Phase 0 — 修复现有 bug（C++ 硬知识）](#phase-0--修复现有-bugc-硬知识)
- [Phase 1 — 工程化基础（职业级习惯）](#phase-1--工程化基础职业级习惯)
- [Phase 2 — 核心子系统（架构能力）](#phase-2--核心子系统架构能力)
- [Phase 3 — C# 脚本系统（高级进阶）](#phase-3--c-脚本系统高级进阶)
- [每个阶段的验证 Demo 与性能分析](#每个阶段的验证-demo-与性能分析)
- [专业级 GitHub 工作流](#专业级-github-工作流)

---

## 阶段总览

| 阶段 | 内容 | 学习收获 | 完成标志 |
|------|------|---------|---------|
| P0 | 修复渲染/内存/空指针 bug | C++ 硬知识、UB、生命周期 | 引擎无已知 UB，可稳定运行 |
| P1 | 测试 + 构建 + CI | 工程化、可测试架构 | Math/Entity 有测试，CI 自动构建 |
| P2 | 物理 + 序列化 + Editor | 架构、碰撞、反射 | Breakout 可玩，Editor 能加载/保存场景 |
| P3 | C# 脚本系统 | 语言嵌入、绑定、热重载 | 能用 C# 写游戏逻辑并热重载 |

> **不要追求"做完"**。每阶段完成即可停下反思，跳入下一阶段。

---

## Phase 0 — 修复现有 bug（C++ 硬知识）

**这一阶段的目的不是"改代码"，而是"从每个 bug 里学到一个 C++ 概念"。** 修每个 bug 前，先问自己：它属于哪类问题？

### 0.1 渲染管线（最高优先级，含 UB）

| 文件:行 | 问题 | 对应 C++ 知识点 |
|---------|------|----------------|
| `Renderer2D.cpp:150` | Quad 索引循环 `i < 2*QuadIndexCount`（此刻为 0）不执行 → 未初始化索引 | **Undefined Behavior**、数组越界 |
| `Renderer2D.cpp:163` | `IndexBuffer::Create(quadIndices, 2)` 应为 6 | 索引缓冲语义 |
| `Renderer2D.cpp:337-357` | `DrawQuad` 忽略传入 position/size，写固定顶点 | 参数使用/函数契约 |
| `Renderer2D.cpp:406-415` | `DrawSprite` 超 `MaxSpriteCount(100)` 无检查 → 越界 | **缓冲区溢出**、防御式编程 |
| `Renderer2D.cpp:401` | `BAMBOO_ASSERT` 条件写反 | 断言语义 |

### 0.2 内存与资源（生命周期）

| 文件:行 | 问题 | 对应 C++ 知识点 |
|---------|------|----------------|
| `OpenGLShader.cpp:105` | `glDeleteShader(kv.first)` 传枚举而非对象 id → 泄漏 | **RAII / 资源生命周期** |
| `Renderer2D.cpp:116/140/183` | 手动 `new[]/delete[]`，依赖显式 `Shutdown()` | 智能指针 vs 手动管理 |
| `ImageAsset.cpp` 依赖 `OpenGLTexture.cpp` 的 `STB_IMAGE_IMPLEMENTATION` | 编译单元耦合脆弱 | 单一定义规则 ODR |

### 0.3 空指针 / 除零（防御式编程）

| 文件:行 | 问题 |
|---------|------|
| `Scene.cpp:87-96` | `GetMainCamera` 找不到相机返回空 `Entity` 后解引用崩溃 |
| `Entity.h:23,39,45` | 默认构造实体（`m_Scene==nullptr`）调用组件方法崩溃 |
| `Camera.cpp:24` | `SetViewportSize` height==0 除零 |
| `Camera.cpp:55-71` | `ScreenToWorldPosition` 用未初始化 `m_CameraData` |
| `WindowsWindow.cpp:30` | `glfwInit` 失败只 `return`，后续用空窗口 |
| `PhysicsSystem.cpp:8` | `Init()` 从未被 `Scene` 调用，`m_PhysicsWorld` 恒为 nullptr |

### 0.4 其他清理

- `Singleton.h:14`：`= delete` 缺分号（编译错误）。
- `Vector2::operator/` 除零 → NaN：修或加测试暴露。
- `Color(int a,int r,int g,int b)` 参数顺序易误用（a 在前）。
- `ShaderDatatType` / `FileUtlis` / Editor `Shoudown` 拼写错误。
- `BallComponent.h` 缺 `#pragma once`。
- 删除所有被注释掉的死代码（`Sandbox/main.cpp:41-109` 等）。
- `Camera::ReCalculateProjectionMatrix` 覆盖 `SetOrthographic` 传入的 size。

### 0.5 验收

- 编译通过、无警告。
- Breakout 场景能跑且无黑屏/崩溃。
- （若有测试）`Math` 除零测试暴露的 NaN 已被记录或修复。

---

## Phase 1 — 工程化基础（职业级习惯）

**这一阶段学习"把代码写成可靠工程"的能力，这是"大型项目"和"脚本"的分水岭。**

### 1.1 引入单元测试（GoogleTest）

- 新建 `Source/Tests/`，独立 `CMakeLists.txt`，**只有测试 target 依赖 gtest**，引擎库不依赖。
- 顶层级联：`option(BAMBOO_BUILD_TESTS "构建单元测试" ON)` → `add_subdirectory(Tests)`。
- 测试对象（先测不依赖 GL 的部分）：
  - `Math`：Vector2/3/4、Matrix3/4、Color、Random。
  - `Entity`：组件增删查（不碰渲染组件）。
  - `Scene`：实体/系统注册。
  - `TransformSystem`：矩阵计算。
- 示例见文末附录 A。

### 1.2 规范化 CMake

- 抽公共函数/变量，消除三个子 CMake 的重复。
- 加 install 规则、统一 compile options。
- 用 CMake presets（`CMakePresets.json`）替代手敲命令行。

### 1.3 引入 CI（GitHub Actions）

- 提交时自动：配置 → 构建 → 跑 `ctest`。
- 让"能编译"从口头约定变成机器强制。

### 1.4 静态检查

- 尝试接入 MSVC `/W4` + `/WX`（警告即错误），或引入 clang-tidy（可选）。

---

## Phase 2 — 核心子系统（架构能力）

### 2.1 物理系统（技术含量最高的挑战）

现状：`PhysicsWorld` 只有重力、`PhysicsSystem::Init` 从不被调用、碰撞全空。

要做：
1. `Scene` 中正确调用 `PhysicsSystem::Init()`。
2. 实现 2D 刚体运动学（位置/速度/加速度积分）。
3. 碰撞检测：先 AABB → 再 SAT（分离轴）。
4. 碰撞响应（反弹、冲量）。
5. 空间分区：把空 `Octree` 或 AABB 网格用起来，优化大量砖块碰撞。

学习收获：**物理积分、碰撞检测数学、空间分区、浮点稳定性**。

### 2.2 场景序列化（学"数据驱动"）

现状：`SceneSerializer.cpp` 是空实现。

要做：
1. 用 JSON（推荐 nlohmann 或项目自定义）序列化实体/组件。
2. 让组件可注册（类型 → 读写函数），为后面 Editor 和 C# 绑定打基础。
3. 实现保存/加载整个场景。

学习收获：**反射思想、数据驱动设计、序列化格式设计**。

### 2.3 Editor 跑起来

现状：Editor 是占位，连 GL 后端都没初始化，且完全绕开引擎。

要做（关键架构修正：**让 Editor 基于引擎的 `Application`，而不是裸调 GLFW**）：
1. 补 `ImGui_ImplOpenGL3_Init` + `glad` 初始化，让 ImGui 能渲染。
2. 实现视口（渲染引擎场景到 ImGui 窗口）。
3. 实现实体层级面板（SceneHierarchyPanel）+ 属性检查器。
4. 实现内容浏览器（ContentBrowserPanel）。
5. 接入 `SceneSerializer`，Editor 内保存/加载场景。

学习收获：**编辑器架构（分层/命令模式）、ImGui 集成、场景与引擎解耦**。

### 2.4 完善输入与相机

- 补齐 `KeyCodes`（数字、功能键、鼠标键）。
- 修 `Input` 空指针检查。
- 支持多相机、相机缩放/旋转。

---

## Phase 3 — C# 脚本系统（高级进阶）

**这是最重的工程，建议在 P2 全部完成后才碰。** 先明确一点：**嵌入 C# 比想象中重得多**，涉及运行时托管、GC、ABI 绑定。务必先做技术调研（可行性验证）再写代码。

技术路线可选：

| 方案 | 说明 | 难度 | 热重载 |
|------|------|------|--------|
| 托管 .NET 运行时（Hostfxr/NativeAOT） | 官方跨语言嵌入，重 | 高 | 复杂 |
| Mono 嵌入 | 老牌方案，文档多 | 中高 | 可 |
| **先用 Lua/sol2 过渡** | 轻量、易嵌入、社区成熟 | **低** | 好 |

**建议：先做 Phase 0 之前用 Lua 验证脚本系统设计，成熟后再考虑 C#。**

要做：
1. 定义脚本接口（实体/组件/系统如何暴露给脚本层）。
2. 实现脚本与引擎的绑定（类似绑定层 / interop）。
3. 实现热重载（改脚本不用重启引擎）。
4. C# 场景：接入 .NET 运行时，管理 GC 与对象生命周期。

学习收获：**脚本语言嵌入、跨语言 ABI、反射/代码生成、热重载机制**。

---

## 每个阶段的验证 Demo 与性能分析

**原则：每个阶段做完，必须有一个可运行的 demo + 一组性能数字，否则不算完成。**

### Demo 清单

| 阶段 | Demo | 验证内容 |
|------|------|---------|
| P0 | Breakout 跑通 | 引擎无崩溃、无黑屏 |
| P1 | MathTest / EntityTest | 单元测试全绿 |
| P2.1 | 物理测试场景：球反弹、砖块碰撞 | 物理正确性 |
| P2.2 | 场景保存 → 加载后一致 | 序列化往返一致 |
| P2.3 | Editor 打开并编辑场景 | 编辑器可用 |
| P3 | 用脚本写一个移动/旋转逻辑 | 脚本系统可用 |

### 性能分析清单

| 场景 | 测什么 | 工具/方法 |
|------|--------|----------|
| 大量精灵（1000/5000/10000） | 帧耗时、draw call 数 | `glDebugMessageCallback`、spdlog、帧计数器 |
| 批处理 vs 逐绘制 | 合并 draw call 的收益 | 对比 draw call 计数 |
| 大量砖块碰撞 | 空间分区 vs 暴力 | 碰撞耗时对比 |
| 内存 | 是否有泄漏/碎片 | 调试器 `_CrtSetDbgFlag`（代码里已有注释）、VS 诊断工具 |

> 性能分析要做**前后对比**（改前 vs 改后），用数字证明优化有效，而不是"感觉快了"。

---

## 专业级 GitHub 工作流

### 现状问题

你现在是 `pull → push` 直接推到主干（`main`/`master`），提交信息全是"提交修改""提交修改..."。这在小项目能跑，但不是职业级，也让你无法回溯/协作。

### 推荐：GitHub Flow（单人项目最合适的专业流程）

```
feature 分支
   │ 开发
   │ 提交（有意义的 message）
   ▼
合并回 main ──> CI 自动构建+测试
```

### 具体规范

1. **分支策略**：每个功能/修复开独立分支 `feature/物理碰撞`、`fix/quad索引`、`refactor/命名`，完成后合并回 `main`。
2. **提交信息规范**（Conventional Commits）：
   ```
   feat(physics): 实现 AABB 碰撞检测
   fix(renderer): 修复 Quad 索引缓冲未初始化
   refactor(core): 统一 ShaderDataType 拼写
   docs(roadmap): 更新开发路线
   test(math): 为 Vector2 除零补测试
   ```
   格式：`类型(模块): 描述`。类型：`feat` `fix` `refactor` `docs` `test` `build` `ci` `perf`。
3. **提交粒度**：一个提交只做一件事，原子化。别一次提交 50 个文件。
4. **Commit 前检查**：
   ```bash
   git status        # 看有哪些改动
   git diff          # 审查改动是否干净
   git log --oneline # 看风格
   ```
5. **不要提交**：`Source/build/`、`.vscode/`、`.vs/`、临时文件——**加 `.gitignore`**（你当前 `Source/build/` 未跟踪，靠手滑会误提交）。
6. **Pull Request**：即使单人项目也开 PR 合并，PR 即"代码评审"的仪式感，养成自己 review 自己的习惯。
7. **CI 集成**（接 P1.3）：push 到 main 自动构建 + 跑测试，红叉立刻发现回归。

### 你现在的 Git 仓库状态

- `Source/build/`、`.vscode/`、`.vs/`、`Code-queries-main/`、`all_*.txt` 都在未跟踪状态 → **先加 `.gitignore` 再统一处理**。
- 历史上大量 "提交修改" → 不必回改，从现在起用规范即可。

---

## 附录 A：测试示例（GoogleTest）

`Source/Tests/Math/Vector2Test.cpp`：

```cpp
#include <gtest/gtest.h>
#include "Math/Vector2.h"

TEST(Vector2, Add) {
    Bamboo::Vector2 a(1, 2), b(3, 4);
    auto c = a + b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}
```

`Source/Tests/CMakeLists.txt`：

```cmake
file(GLOB_RECURSE TESTS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(BambooTests ${TESTS_SRC})
target_link_libraries(BambooTests PRIVATE Bamboo gtest_main)
include(GoogleTest)
gtest_discover_tests(BambooTests)
```

运行：

```
cmake --build build --target BambooTests
ctest --test-dir build --output-on-failure
```