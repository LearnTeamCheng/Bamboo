# Bamboo 引擎重构升级评审与实施计划

> 评审对象：`Source/Bamboo`（引擎核心库）、`Source/Sandbox`、`Source/Editor`、`Source/ThirdParty`、`Source/CMakeLists.txt`
> 评审范围：`Source/Bamboo` 下全部 87 个源文件（约 4,600 行）+ 构建脚本 + 资源目录
> 评审方式：逐文件通读源码（非依赖既有文档），并用 Git / 构建产物 / CMakeCache 交叉验证
> 结论依据：所有问题条目均标注 `文件:行`，可直接定位

---

## 目录

- [0. 评审结论摘要](#0-评审结论摘要)
- [1. 验证方法与现状快照](#1-验证方法与现状快照)
- [2. 缺陷清单（按优先级）](#2-缺陷清单按优先级)
  - [P0 阻断级](#p0--阻断级必须先解决)
  - [P1 正确性级](#p1--正确性级会导致渲染错误或未定义行为)
  - [P2 设计级](#p2--设计级架构耦合与职责错位)
  - [P3 体验与可维护性](#p3--体验与可维护性)
  - [P4 工程化与构建](#p4--工程化与构建)
  - [P5 命名与文档](#p5--命名与文档)
- [3. 不合理的地方（设计层面总评）](#3-不合理的地方设计层面总评)
- [4. 要完善的地方（能力缺口）](#4-要完善的地方能力缺口)
- [5. 重构路线图](#5-重构路线图)
- [6. 目标架构](#6-目标架构)
- [7. 每个子系统的验收标准](#7-每个子系统的验收标准)
- [8. 需要你决策的事项](#8-需要你决策的事项)
- [9. 立即可做的第一步](#9-立即可做的第一步)
- [附录 A：命名与拼写统一对照表](#附录-a命名与拼写统一对照表)
- [附录 B：本次评审涉及的 C++ 知识点索引](#附录-b本次评审涉及的-c-知识点索引)
- [附录 C：本次评审未覆盖 / 需要进一步确认的部分](#附录-c本次评审未覆盖--需要进一步确认的部分)

---

## 0. 评审结论摘要

一句话结论：**当前引擎不是"有 bug 的引擎"，而是"渲染主链路刚跑通、其余子系统基本还是骨架"的引擎。** 渲染管线靠"每次重分配显存 + 硬编码 1280×720"能凑出画面，但底层缓冲语义是错的；物理、序列化、UI、编辑器、资源热加载全部是空实现或未接入。

按"投入产出比"排序，最值得先做的四件事：

| 顺位 | 事项 | 为什么先做它 |
|---|---|---|
| 1 | **重建 `Renderer2D` 的批次/缓冲契约** | 这是唯一在正常路径上就会触发 UB 的地方（缓冲越界、未初始化索引、类型混淆分配），且它决定了后面所有渲染相关功能能不能长出来 |
| 2 | **让相机成为参数的唯一来源（消灭硬编码 1280/720）** | 现在 `DrawTriangle` 除以 1280/720、`BreakoutApp` 按 ±640 摆砖块、`Scene` 又写死 1280×720 视口，三者互相不知道对方存在。不解决它，任何分辨率/缩放/编辑器视口都在踩地雷 |
| 3 | **让 `Application::Run` 的帧顺序与初始化顺序变正确** | `RendererCommand::Init()` 依赖静态初始化、`PhysicsSystem::Init()` 从未被调用、事件只在帧尾轮询。这是"引擎能不能被信任"的地基 |
| 4 | **补 `.gitignore` + `/W4` + 测试骨架** | 目前 `Source/build/`、`.vs/` 全部处于未跟踪状态，且**仓库根目录没有 `.gitignore`**；没有任何警告等级设置。工程化欠账越晚补越贵 |

同时必须说清一件事：**你现在的 `Source/build/bin/Debug/Sandbox.exe`（2026/9/16 17:23 构建）是最后一次成功构建。** 2026/9/17 之后工作区里未提交的 `Renderer2D.cpp` 改动把 `CircleVertices` 写成了 `new QuadVertex[4]` 却存进 `CircleVertex*`，**当前工作区这份代码在开启警告的情况下必然报错，且语义上是 UB**。所以"先让它重新干净地编译通过"是 P0 的第 0 步。

---

## 1. 验证方法与现状快照

### 1.1 我做了什么

1. 列出 `Source/` 下全部非 ThirdParty 源文件与行数（153 个文件 / 205,848 字节，其中引擎核心约 4,600 行）。
2. **逐文件通读**了：`Core/`（12 个）、`ECS/`（组件 12 + 系统 8 + Entity）、`Scene/`（5）、`Graphics/`（22）、`GraphicsAPI/OpenGL/`（12）、`Physics/`（4）、`Assets/`（6）、`UI/`（7）、`Math/`（14）、`Game/`（4）、`Platform/`（4）、`Editor/`（3）、`Sandbox/`（10）、以及全部 `CMakeLists.txt`。
3. 交叉验证：
   - `git log --oneline -15`、`git status --short`、`git diff`
   - `Source/build/CMakeCache.txt`（生成器：Visual Studio 17 2022 / x64）
   - `Source/build/generated/Config.h`（确认 `BAMBOO_ASSET_ROOT` 被烤成绝对路径）
   - 构建产物时间戳（确认最后一次成功构建时间）
   - `Source/BambooAssets/` 资源清单
   - 检索全仓库确认：`STB_IMAGE_IMPLEMENTATION` 出现位置、缺失 `#pragma once` 的头文件、`BAMBOO_ASSERT` 实际调用点、`ScreenToWorldPosition`/`PixelSizeToWorldSize` 是否被调用

### 1.2 现状快照

| 项目 | 现状 |
|---|---|
| 语言/标准 | C++17，Windows + MSVC only |
| 构建 | CMake 3.20 + VS 2022 x64；无 CMakePresets、无 CI、无测试、无 install |
| 依赖 | 全部 vendored（GLFW / glad / entt / spdlog / stb / imgui） |
| 代码规模 | 引擎核心 ≈ 4,600 行；Sandbox ≈ 200 行；Editor ≈ 110 行 |
| 引擎核心成熟度 | 渲染主链路"能出画面"；物理/序列化/UI/编辑器为骨架 |
| 最后一次成功构建 | `Sandbox.exe` 2026/9/16 17:23，`Bamboo.lib` 2026/9/16 17:22 |
| 当前分支 | `refactor/architecture`（已开重构分支，方向正确） |
| 未提交改动 | `Source/Bamboo/Graphics/Renderer2D.cpp`（+10/−1，引入了类型不匹配的新代码） |
| 版本控制卫生 | **仓库根目录没有 `.gitignore`**；`Source/build/`、`.vs/`、`.vscode/`、`.VSCodeCounter/`、`Code-queries-main/`、`all_*.txt` 均为未跟踪散落状态 |
| 提交信息 | 近 15 条中 13 条为"提交修改"，不可回溯 |
| 文档 | ~~`docs/project_structure.md`（22KB，已过时）~~ **已于 2026-09-18 删除，由 `docs/architecture.md` 取代**、`docs/roadmap.md`（12KB，有效但偏学习导向）、`AGENTS.md`（准确）、根目录 `README.md`（仍仅 12 字节） |

---

## 2. 缺陷清单（按优先级）

优先级定义：

- **P0 阻断级**：编译失败、必然的 UB、崩溃。必须先解决，否则其他工作没有意义。
- **P1 正确性级**：能跑出画面但结果是错的（矩阵转置、UV、相机参数、颜色通道）。
- **P2 设计级**：架构耦合、职责错位、生命周期不清。不会立刻崩溃，但会让每个新功能都变贵。
- **P3 体验与可维护性**：性能、调试能力、错误处理、可测试性。
- **P4 工程化与构建**。
- **P5 命名与文档**。

---

### P0 — 阻断级（必须先解决）

#### P0-1 `Renderer2D` 圆缓冲类型混淆（**当前工作区的未提交改动**）

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:197`
- **现状**：`s_Data.CircleVertices = new QuadVertex[4];`，而 `CircleVertices` 的声明类型是 `CircleVertex*`（`Renderer2D.cpp:89`）。
- **两个后果**：
  1. `QuadVertex`（`Vector3 + Color` = 28 字节）与 `CircleVertex`（`Vector3 + Color + float` = 32 字节）布局不同，**类型双关 → UB**；且 MSVC 会给出 C2440（无法从 `QuadVertex*` 转换到 `CircleVertex*`）类错误，**当前工作区根本编不过**。
  2. 数组长度也不对：`CircleComponent` 的圆即使走四边形也要 4 个顶点，而这里只分配了按 `QuadVertex` 计算的 4 个元素（大小算错）。
- **修复**：`new CircleVertex[4]`；并在重写 `Renderer2D` 时统一由容器（如 `std::vector` 或正确的常量表达式）管理，禁止裸 `new[]`。

#### P0-2 Quad 索引缓冲是"未初始化 + 长度错误"的双重 UB

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:165-183`
- **现状**：
  ```cpp
  uint32_t *quadIndices = new uint32_t[6 * 2];      // 分配 12 个，但只写 6 个
  for (uint32_t i = 0; i < 2 * s_Data.QuadIndexCount; i += 6)   // QuadIndexCount 此刻是 0 → 循环体一次都不执行
  ...
  Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, 2);  // 却上传 2 个"未初始化"的值
  ```
- **后果**：索引值是完全未初始化的栈/堆垃圾，`glDrawElements` 用它去索引顶点数组 → 越界读取顶点缓冲，画面随机错误或驱动报错。奇怪的是它偶尔"看起来正常"，原因是 `QuadIndexCount` 在 `StartBatch` 里被清零、而 Quad 分支只在 `TriangleComponent`/`QuadComponent` 存在时才绘制——**靠"没人用 Quad"掩盖了这个 bug**。
- **修复**：直接用字面量 `{0,1,2, 2,3,0}`，或按 `MaxQuads` 预生成完整索引表，上传 `MaxQuads * 6`。这个模式在 Sprite 的索引生成（`Renderer2D.cpp:220-235`）里是**正确**的，Quad 分支照抄即可。

#### P0-3 顶点缓冲只按"1 个顶点"分配，却重复写入并上传

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:126`（Triangle）、`153`（Quad）、`204`（Sprite）
- **现状**：`VertexBuffer::Create(sizeof(TriangleVertex))` 只创建 **1 个顶点**大小的缓冲；随后 `Renderer2D::DrawTriangle` 无条件写 3 次 `TriangleVerticesPtr++`（`:357-362`），`Flush` 时按实际写入的 dataSize 调 `SetData`（`:301-303`）。
- **后果**：
  - `OpenGLVertexBuffer::SetData` 用 `glBufferData` **重新分配**（`OpenGLBuffer.cpp:43`），GL 的缓冲大小只由 `SetData` 决定、`m_Size` 根本没被记录（`OpenGLBuffer.h` 无 `m_Size` 成员）→ **构造时的 size 形同虚设**。所以运行期"恰好"不越界，但构造参数是骗人的，任何"先写后扩"或"超出 MaxCount"的调用（如 `DrawSprite` 超过 100 个精灵）都会静默越界。
  - 每帧每批次都 `glBufferData` 重新分配显存（应该用 `glBufferSubData` + 构造期一次性 `GL_DYNAMIC_DRAW` 分配），是明确的性能反模式。
- **修复**：
  1. `VertexBuffer` 接口增加 `GetSize()`/记录 `m_Size`，`SetData` 内部断言 `size <= m_Size`，更新用 `glBufferSubData`。
  2. 三个缓冲一律按 `MaxCount` 分配；`DrawXxx` 边界检查（见 P0-5）。

#### P0-4 精灵数量超过上限无任何检查 → 缓冲越界

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:435-447`（写入循环）、上限定义 `:62`（`MaxSpriteCount = 100`）
- **现状**：`DrawSprite` 直接 `s_Data.SpriteVerticesPtr++` 共 4 次，从不检查 `SpriteCount >= MaxSpriteCount`；只有纹理槽满时才会 `NextBatch()`（`:421-424`），**顶点数满不会触发**。
- **后果**：Breakout 有 50 块砖 + 球拍，一旦加载超过 100 个精灵实体就静默写穿堆。`BreakoutApp.cpp:18` 已经是 50 个，加球、砖块随机化后会很容易越过。
- **修复**：写入前 `if (SpriteCount >= MaxSpriteCount) NextBatch();`，并把上限做成可配置常量；同时按 P0-3 修好缓冲尺寸，让越界变成断言而不是 UB。

#### P0-5 `BAMBOO_ASSERT` 是"只打印不中断"，且格式串有类型错误

- **位置**：`Source/Bamboo/Core/Assert.h:7-14`
- **现状**：
  ```cpp
  #define BAMBOO_ASSERT(check, ...)                       \
      { if (!check) {                                     \
          BAMBOO_CORE_ERROR("Assertion Failed: {0} {1}",  \
              BAMBOO_STRINGIFY_MACRO(check), check);      \
          BAMBOO_CORE_ERROR(__VA_ARGS__); } }
  ```
  问题有三层：
  1. **语义错误**：断言失败后只写日志，**继续往下执行**。它不能保护任何东西。断言的契约是"违背即停止"。
  2. **类型不匹配**：把 `check`（`bool`）作为 `{1}` 传给 spdlog 格式化，`check` 是 `status` 这种 `int` 时打印出 `1` 而不是表达式文本，可读性为零。
  3. **参数用法别扭**：既把 `check` 本身当参数打印，又要求调用者再传一遍消息（`OpenGLContext.cpp:19` 就写成 `BAMBOO_ASSERT(status, "glad init failur")`——注意这里误把 `status` 当条件、消息当 `...`，实际语义是"status 非 0 才算过"，靠 `!check` 取反后**逻辑正好反过来**：非 0 反而报错）。
- **影响面**：全仓库真正调用的只有 `OpenGLContext.cpp:19` 一处，其余（`Entity.h:22`、`Renderer2D.cpp:428/430`、`OpenGLTexture.cpp:160`）**全是注释掉的**。也就是说引擎目前**完全没有生效的断言**。
- **修复**：重写为 `if (!(check)) { log; BAMBOO_DEBUG_BREAK(); }`，区分 Debug（`__debugbreak()`）/Release（`std::abort()`），并支持可变消息。修好后把 `Entity::AddComponent` 的重复组件检查、缓冲边界检查、`Texture2D::SetData` 的大小检查全部接回去。

#### P0-6 `Entity` 空句柄解引用 → 立即崩溃

- **位置**：`Source/Bamboo/ECS/Entity.h:19-53`、`:59-60`
- **现状**：`Entity() = default;` 让 `m_Scene = nullptr`、`m_EntityHandle = entt::null`。但 `AddComponent`/`GetComponent`/`HasComponent`/`RemoveComponent`/`GetUUID`/`GetName` **全部无条件解引用 `m_Scene`**。
- **触发路径**（真实存在）：
  - `Scene::FindEntityByName` 找不到时 `return {};`（`Scene.cpp:78`）→ 调用方任何 `entity.GetComponent<...>()` 都是空指针解引用。
  - `Scene::GetMainCamera`（`Scene.cpp:87-96`）：`FindEntityByName("MainCamera")` 若返回空实体的结果，`entity.HasComponent<CameraComponent>()` 直接崩。
  - `Scene::DestroyEntity(entity)`（`Scene.cpp:81-85`）先 `m_Registry.destroy` 再 `entity.GetUUID()`，对已销毁实体取组件 → entt 断言/UB。
- **修复**：
  1. 所有组件操作加前置检查（`BAMBOO_ASSERT(m_Scene && m_EntityHandle != entt::null, ...)`），Debug 下 assert、Release 下安全返回。
  2. 引入 `bool IsValid() const`，调用方先判断；`FindEntityByName`/`GetMainCamera` 改为返回 `std::optional<Entity>` 或明确判空。
  3. `DestroyEntity` 调整为先取 UUID 再 destroy。

#### P0-7 相机指针悬垂（返回临时实体的内部指针）

- **位置**：`Source/Bamboo/Scene/Scene.cpp:87-96`
- **现状**：
  ```cpp
  Camera *Scene::GetMainCamera() {
      auto entity = FindEntityByName("MainCamera");     // 局部 Entity
      if (entity.HasComponent<CameraComponent>())
          return &entity.GetComponent<CameraComponent>().CurrentCamera;  // 指向 registry 里的组件
      return nullptr;
  }
  ```
  返回的是 registry 内部组件的地址，**只要实体被销毁/视图重建就悬垂**；更严重的是它**按名字查找**，而 `RendererSystem` 却按 `registry.view<CameraComponent, TransformComponent>()` 查找（`RendererSystem.cpp:27-32`），`CameraSystem` 又按 `Primary` 标记（`CameraSystem.cpp:19`）——**三处各用一套"谁是主相机"的判据**。
- **修复**：确立唯一判据（建议 `Primary == true`，且限制至多一个），`Scene` 提供 `Entity GetPrimaryCameraEntity()`；相机通过 ECS 查询获取引用，不缓存裸指针跨帧。

#### P0-8 集合在迭代中被修改

- **位置**：`Source/Bamboo/ECS/System/RendererSystem.cpp:83-86`（渲染期回写组件）、`Source/Bamboo/Graphics/Renderer2D.cpp:336-339`（遍历 `TextureSlotMap` 时可能触发 `NextBatch()`）
- **现状**：
  - `RendererSystem` 遍历 `sprites` 时执行 `sprite->SpriteTexture = Renderer2D::GetNormalTexture();`——**渲染系统在写 ECS 数据**。这不仅破坏"渲染只读"的分层，还让"从未设置纹理"这一信息被永久抹掉（无法再区分"用户没给纹理"和"用户明确要白纹理"）。
  - `Flush` 中 `for (auto texture : s_Data.TextureSlotMap)`（按值拷贝 key，见 P3）内部若因槽满走 `NextBatch()`，会在遍历 `TextureSlotMap` 的同时 `clear()` 它 → 迭代器失效。
- **修复**：渲染期对 registry 只读；缺纹理在 `DrawSprite` 内部回落到白纹理（局部判断，不落盘）；`Flush` 里先收集槽位快照再绑定。

#### P0-9 多个成员变量从未初始化 → 读未初始化值

| 位置 | 未初始化成员 | 触发后果 |
|---|---|---|
| `Renderer2D.cpp:91` | `uint32_t CircleIndexCount;`（无 `= 0`） | `StartBatch` 没有重置它，`Flush` 若将来判断它 → 读到垃圾 |
| `Renderer2D.cpp:197` | `CircleVertices` 已按错误类型分配（见 P0-1） | UB |
| `ECS/Component/RigidbodyComponent.h:9-18` | `Type`、`Mass`、`Force`、`Torque`、`Impulse`、`Friction` 全部无初值 | `PhysicsSystem::Update` 读 `rigidbody.Type` → 随机进入某分支；`Force +=` 从垃圾值开始累加 |
| `ECS/Component/BoxCollider2DComponent.h:8-13` | `Size`、`Offset`、`Friction`、`Density`、`IsTrigger` 无初值（仅带参构造函数有默认值，默认构造不初始化） | 碰撞检测（未来接入）读到未定义尺寸 |
| `ECS/Component/CircleColliderComponent.h:9-15` | `Radius`、`Center`、`Offset`、`IsTrigger` 同上；且 `Center` 默认给的是 `Vector2::Half`（合法但语义可疑，圆心默认 0.5？） | 同上 |
| `ECS/Component/QuadComponent.h:12` | `Color Color;`（`Color` 默认构造给了 (0,0,0,1)，侥幸安全）、`Vector2 Size;`（默认构造 = 0） | 侥幸 |
| `ECS/Component/TriangleComponent.h:9` | `Vector2 Size;` 默认 = 0，安全但从未被使用 | 死字段 |
| `Graphics/Camera.h:59, 73` | `bool m_Orthographic;`、`ProjectionType m_ProjectionType;` 无初值 | `ReCalculateProjectionMatrix` 里 `if (m_ProjectionType == Orthographic)` 读未初始化枚举 → 可能走进空 `else` 分支，**投影矩阵保持单位阵**，画面完全错乱。`Scene` 构造函数里恰好先调了 `SetOrthographic` 才掩盖了它 |
| `Graphics/Camera.h:86` | `CameraData m_CameraData;` 无初值 | `ScreenToWorldPosition` 直接读它 → 未定义（该方法目前无调用者，见附录 C） |
| `UI/UIElement.h:81-85` | `m_Position`、`m_Scale`、`m_Rotation`、`m_Size`、`m_Anchor` | UI 一旦启用立刻读到垃圾 |
| `UI/Canvas.h:27-29` | `m_Width`、`m_Height`、`m_ClearFlag` | 同上 |
| `UI/Button.h:51-52` | `m_Transition`、`m_IsInteractable` | 同上 |
| `Platform/Windows/WindowsWindow.h:35` | `GLFWwindow* m_Window;` 无初值 | `glfwInit()` 失败时 `Initialize` 提前 return（`WindowsWindow.cpp:30-34`），`m_Window` 保持垃圾；随后 `Update()` 调 `m_Context->SwapBuffers()` → `m_Context` 为空 → 崩溃 |
| `GraphicsAPI/OpenGL/OpenGLBuffer.h:29,44` | `uint32_t m_RendererID;` 无初值 | 构造函数里 `glGenBuffers` 会写入，安全；但若 `glGenBuffers` 因无上下文失败则保持垃圾 |
| `GraphicsAPI/OpenGL/OpenGLShader.h:43` | `uint32_t m_RendererID;` 无初值 | 编译失败路径 `return`（`OpenGLShader.cpp:90`）后 `m_RendererID` 保持 `glCreateProgram()` 的值但程序未链接 → 后续 `Bind()` 绑一个坏程序 |
| `Assets/ImageAsset.cpp:8` | 构造函数已初始化 `m_Width/Height/Channels`，但 `LoadFromFile` **失败时不置 `m_IsLoaded = false`** | 状态不自洽 |

**统一修复**：全部改为**类内成员初始化器**（`float Mass = 1.0f;` 等），把"默认构造即有效状态"作为组件的不变量。

#### P0-10 `ThirdParty/CMakeLists.txt` 的 entt 包含路径错误

- **位置**：`Source/ThirdParty/CMakeLists.txt:10`
- **现状**：`${CMAKE_CURRENT_SOURCE_DIR}/entt/include()` —— 结尾是**一对空括号**。这是把函数调用写法的 `include()` 误粘进了路径，实际路径字符串是 `.../entt/include()`。
- **后果**：`#include "entt.hpp"` 找不到头文件。之所以历史构建能过，是因为 `Source/build/` 里残留了旧的缓存/生成结果（`.vs` 与 `x64` 目录都在，说明配置过程被反复重跑且带着旧状态）。**这台机器上 `Source/build` 一旦被删除重建，整个工程立刻配置/编译失败。**
- **修复**：改为 `${CMAKE_CURRENT_SOURCE_DIR}/entt/include`。

#### P0-11 `RendererSystem.cpp` 使用 `std::sort` 但未包含 `<algorithm>`

- **位置**：`Source/Bamboo/ECS/System/RendererSystem.cpp:78`
- **现状**：文件只 `#include <vector>`（`:1`）。`std::sort` 靠标准库的传递包含侥幸可见。
- **后果**：换 STL 版本或调整包含顺序即编译失败；属于典型的"隐式依赖"。
- **修复**：显式 `#include <algorithm>`（并全面排查同类问题，见 P4-3）。

#### P0-12 声明了但从未定义的函数（潜在链接错误）

| 位置 | 声明 | 现状 |
|---|---|---|
| `UI/UIElement.h:14` | `virtual ~UIElement();` | 只有声明，`UIElement.cpp` **没有定义**。任何 `UIElement` 实例化/析构 → 链接错误 |
| `UI/Canvas.h:17-24` | `Canvas()`、`~Canvas()`、`SetSize`、`AddElement`、`RemoveElement` | `Canvas.cpp` **完全为空** |
| `UI/Button.h:24,29,30,35-37` | 两个带参构造、`~Button()`、三个 `SetXxxTexture` | `Button.cpp` **完全为空** |
| `Graphics/Texture.h:56-58` | `Texture2D::Create(...)` 三个静态函数 | 已定义，但 `Create(const std::string&)` 内部依赖 `Application` 单例（见 P2-6） |
| `Scene/SceneCamera.h:6` | `SceneCamera();` | 无定义，且**整个类无人使用** |
| `Math/Quaternion.h:29-30` | `static const Quaternion Identity/Zero;` | 未定义；`Quaternion.cpp` 只实现 `FromAngleAxis`，`Normalize`/`GetNormalized` 也未定义 |
| `Math/Matrix3.h:53-54` | `Orthographic`、`Perspective` | 未定义（`Matrix4` 也没有 `Perspective`） |
| `Math/Rect.h:77` | `RectInt(const Vector2Int&, const Vector2Int&)` | 未定义，且 **`Vector2Int` 这个类根本不存在**（只有 `IntVector2`，`Rect.h:68` 的前向声明写了不存在的名字） |
| `Math/Vector4.cpp` | — | 文件**存在但为 0 行**（纯垃圾文件） |

- **修复**：要么实现，要么删除。**"声明不定义"比"没有"更危险**，因为它会在某个无辜的调用点上以链接错误的形式爆出来，让人以为是调用方的问题。建议这一轮把 `UI/`、`Quaternion`、`SceneCamera`、`Vector4.cpp` 按第 8 节的决策统一处理。

---

### P1 — 正确性级（会导致渲染错误或未定义行为）

#### P1-1 `Matrix3` 内部布局自相矛盾（矩阵→向量乘法等于做了转置）

- **位置**：`Source/Bamboo/Math/Matrix3.h:19-20` vs `Matrix3.cpp:22-30`、`:106-192`
- **现状**：
  - `operator()(row,col)` 返回 `m_data[row + col*3]` → **列主序（column-major）**。
  - `Matrix3::operator*(const Matrix3&)`（`Matrix3.cpp:16`）用 `m_data[i*3 + j]` 索引 → 与 `operator()` 不一致。
  - `Matrix3::operator*(const Vector3&)`（`:26-28`）写的是 `result.x = m_data[0]*x + m_data[3]*y + m_data[6]*z` → **这是"把矩阵当列主序"的写法，等价于乘转置矩阵**。
  - `Matrix3::Scale/RotateX/RotateY/RotateZ/Translate` 的赋值索引也按列主序写；而 `Translate` 把平移量写进 `m_data[2], m_data[5], m_data[8]` → **把平移放进了第三行而不是第四列**，行主序下完全错误。
- **实际影响**：`Renderer2D.cpp:355` 用 `Matrix3::Scale(scale,scale,scale)`（对角阵，侥幸两种约定都对）做三角形缩放；换任何非对角矩阵（旋转/平移）立刻出错。
- **修复**：**统一为行主序**（与 `Matrix4` 一致，也与"传给 GL 时 `GL_FALSE` 不转置"一致——见 P1-2），并把 `operator()` 改为 `m_data[row*3+col]`。改完后补 `Matrix3 * Vector3` 的单元测试。

#### P1-2 `Matrix4::RotateXYZ` 把角度当弧度用

- **位置**：`Source/Bamboo/Math/Matrix4.cpp:255-283`
- **现状**：`float cosX = cos(rotation.x);` 直接对**角度值**求 cos。项目里 `Math::AngleToRadian`（`Math/Math.h:125`）明确存在，说明约定是"对外用角度"，但这里没有转换。
- **后果**：任何非零旋转都得到错误矩阵。目前 `TransformSystem` 把旋转整个注释掉了（`TransformSystem.cpp:21-22`），所以没暴露。
- **修复**：入口统一 `Math::AngleToRadian()`，并明确文档"引擎对外 API 一律角度制，内部矩阵计算一律弧度制"。
- **连带**：`Matrix3::RotateX/Y/Z` 与 `Matrix4::RotateXYZ/RotateZ` 都没有 `std::` 前缀（依赖全局 `cos`/`sin`），且文件里没有 `#include <cmath>`——`Matrix4.cpp` 靠 `Vector3.h → Math.h → <cmath>` 传递包含。**隐式依赖，需显式化。**

#### P1-3 `AABB::Merge` 的 min 被覆盖

- **位置**：`Source/Bamboo/Math/AABB.cpp:41-45`
- **现状**：
  ```cpp
  void AABB::Merge(const AABB &other) {
      m_Min = Vector3::Min(m_Min, other.m_Min);
      m_Max = Vector3::Max(m_Min, other.m_Max);   // ← 用的是刚被改写的 m_Min，不是原来的 m_Max
  }
  ```
- **后果**：合并结果错误（max 会取到 min 与 other.max 的较大者，通常把包围盒压扁）。`Merge` 目前无调用者，但一旦接入 broadphase 就是隐形错误。
- **修复**：先算 `newMin`/`newMax` 再赋值；或一行 `m_Min = Vector3::Min(m_Min, other.m_Min); m_Max = Vector3::Max(m_Max, other.m_Max);`。
- **连带**：`AABB` 成员 `m_Min/m_Max` **无初值**（`AABB.h:29-30`），默认构造出的 AABB 是垃圾值 → 加初始化器。

#### P1-4 `Color` 类：三套通道语义互相矛盾，16 进制解析实际是错的

- **位置**：`Source/Bamboo/Math/Color.h` / `Color.cpp`
- **问题清单**：
  1. **`Color(int a, int r, int g, int b)`（`:21`）参数顺序是 ARGB**，与 `Color(float r, float g, float b, float a)`（`:23`）相反。调用 `Color(255, 0, 0)` 得到的是"alpha=255, r=0, g=0, b=0"=黑色，而不是红色。你必须在脑子里记住这个反直觉顺序——bug 温床。
  2. **`Color(unsigned int hex)`（`:26`）不做归一化**：`r = hex >> 24 & 0xff` 得到 0~255 的**整数**塞进 `float`，而其他构造函数的 `r` 是 0~1。同一个类里 `r` 的量纲不统一。
  3. **`Color(const std::string&)` → `SetColorToHex` 只接受 9 字符**（`str.size() != 9`，`:101`）即必须写全 `#RRGGBBAA`；**常见的 `#RRGGBB`（7 字符）直接静默 return**，颜色保持默认黑色。
  4. **解析失败静默**：`SetColorToHex` 所有错误路径都是 `return`，不报错、不返回 bool。一个 typo 变成"物体隐形"。
  5. 文件名叫 `Color.h` 但实际是 RGBA 四通道；`ToString` 用 `ostringstream`（偏重，可换 `std::format`）。
- **修复**：**只保留一套语义**——建议统一 `Color(float r, float g, float b, float a = 1.0f)` + 显式工厂 `Color::FromRGBA8(r,g,b,a)` / `Color::FromHex(std::string_view)`（支持 `#RGB`/`#RRGGBB`/`#RRGGBBAA`，返回 `std::optional<Color>`）。删除或显式弃用 `Color(int a, int r, int g, int b)`。

#### P1-5 `Vector2` 的除零行为不一致 + `Cross` 语义错误

- **位置**：`Source/Bamboo/Math/Vector2.h:40` vs `:47-54`，`:154-157`
- **现状**：
  - **成员** `operator/(float)`（`:40`）**没有除零检查** → 产生 `inf`/`NaN`。
  - **友元** `operator/(const Vector2&, float)`（`:47-54`）**有**除零检查返回 `Vector2()`。
  - 两者语义不同（一个有保护、一个没有），同一个表达式 `a / b` 会因重载不同而行为不同。
  - `Cross(const Vector2&)`（`:154-157`）返回 `Vector2`，但 2D 叉乘的正确结果是**标量**（z 分量）。当前实现 `(y*other.x - x*other.y, x*other.y - y*other.x)` 两个分量都是错的。**2D 碰撞/转向逻辑会直接算错。**
- **修复**：`operator/` 统一（建议：Debug 断言非零 + Release 返回零向量）；`Cross` 改为 `float Cross(const Vector2&) const`（`IntVector2::Cross` 已经是 `int` 标量，正确，可对齐）。

#### P1-6 `Camera` 参数被自己覆盖，`SetOrthographic` 实际无效

- **位置**：`Source/Bamboo/Graphics/Camera.cpp:28-53`
- **现状**：
  ```cpp
  void Camera::ReCalculateProjectionMatrix() {
      if (m_ProjectionType == ProjectionType::Orthographic) {
          m_OrthographicSize = m_ViewportHeight * 0.5f;   // ← 覆盖了 SetOrthographic 传入的值
          float left  = -m_OrthographicSize * m_AspectRatio;
          ...
  ```
- **后果**：
  1. `SetOrthographic(size, near, far)` 里的 `size` **被彻底丢弃**。`Scene.cpp:28` 传的 `10` 从未生效。**引擎无法缩放/变焦。**
  2. `m_AspectRatio = width/height` 又被同时用作左右半宽，导致视口不是"高度决定视野"，而是 `x∈[-1280,1280]、y∈[-720,720]`——**世界单位 = 0.5 像素**，所有像素坐标要乘 2。这是 `BreakoutApp.cpp:39` 里 `-640` 这个魔数的来源。
  3. `SetViewportSize` 除零保护写成了 `if (width == 0 && height == 0) { /* 空 */ }`（`:16-19`）——**判断了却什么都不做**，且应该是 `||`。height=0 时下一行 `width/height` → `inf`。
  4. `ReCalculateProjectionMatrix` 的 `else` 分支为空（`:50-52`），`ProjectionType::Perspective` 是谎言——声明支持但没有任何实现。
  5. **`m_Orthographic`（`Camera.h:59`）完全没用过**，与真正生效的 `m_ProjectionType` 重复；`m_Forward`/`m_Up`（`:75-76`）也是死字段。
- **修复**：把"世界视野高度"作为唯一显式参数（如 `SetWorldHeight(float)`），投影矩阵只依赖 `size + aspect`；`SetViewportSize` 修正除零；删除 `m_Orthographic`/`m_Forward`/`m_Up`/空 `else`；`Perspective` 要么实现要么删除枚举值。

#### P1-7 `Renderer2D::DrawTriangle` 硬编码分辨率 + 忽略参数

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:348-364`
- **现状**：
  ```cpp
  Vector3 inPos = Vector3(position.x / 1280, position.y / 720, 0);  // inPos 算出来后从未使用！
  float scale = 0.5f;                                               // 硬编码
  Matrix3 m3 = Matrix3::Scale(scale, scale, scale);
  ```
  传入的 `position` 被用于除法后**完全丢弃**；缩放硬编码 0.5。三角形的实际位置永远在原点。
- **修复**：位置/缩放/旋转全部通过变换矩阵参与，绘制函数不再接受裸 `Vector2 position`；分辨率相关的一切转换集中在相机。

#### P1-8 两个 `DrawQuad` 重载忽略 `position` 和 `size`

- **位置**：`Source/Bamboo/Graphics/Renderer2D.cpp:366-386`
- **现状**：两个重载（`Vector2`/`Vector3` 版本）函数体**完全相同**，都把写死的 `s_Data.QuadVertexPosition[i]`（±0.5 的 NDC 方框）写进去，**完全忽略传入的 `position` 与 `size`**。
- **连带**：`RendererSystem.cpp:61` 调用时传的 `Vector2(100,100)` 也没用，是"看起来在工作"的假象。
- **修复**：统一走"模型矩阵变换 4 个 ±0.5 顶点"的路径（和 `DrawSprite` 一致），删除重复重载。

#### P1-9 `DrawSprite` 的参数非 const 引用 + UV 与翻转叠加

- **位置**：`Source/Bamboo/Graphics/Renderer2D.h:27`、`Renderer2D.cpp:388`、`OpenGLTexture.cpp:41`
- **现状**：
  1. `Ref<Texture2D> &texture` 是**非 const 左值引用**，导致无法传入 `GetNormalTexture()` 这类返回右值的临时对象——API 无谓地限制调用方。应为 `const Ref<Texture2D>&`。
  2. `OpenGLTexture2D` 构造函数里 `stbi_set_flip_vertically_on_load(1)` 是**全局状态设置**，而 UV 表（`Renderer2D.cpp:391-396`）又是标准的"左下 (0,0)"顺序 → **翻转与 UV 表叠加，图可能是上下颠倒的**。需要明确"到底谁负责 Y 翻转"，二选一（推荐：stb 不翻转，UV 在渲染器里按需翻转）。
  3. `textureIndex` 用 `float` 存槽位（`:400,414,426`）并作为顶点属性，而顶点布局里声明为 `Float`（`:208`）。语义上应是 `uint32_t` + `glVertexAttribIPointer`。
- **修复**：`const Ref<Texture2D>&`；Y 翻转策略单一化；槽位索引改整型。

#### P1-10 三种图元的"索引计数"语义不一致

- **位置**：`Renderer2D.cpp:363`（`TriangleIndexCount += 3`）、`:374/:385`（`+= 6`）、`:446`（`+= 6`）
- **现状**：`*IndexCount` 在三角形分支里被当作**顶点数**累加（3），在 Quad/Sprite 分支里被当作**索引数**累加（6）。`Flush` 里它被作为 `glDrawElements` 的 count 使用——对三角形来说 count=3 恰好等于"3 个顶点画 1 个三角形"，所以侥幸正确；但语义混乱，任何"一个批次画多个三角形"的演进都会立刻出错。
- **修复**：统一为"索引数"，并让每个形状的顶点/索引生成遵循同一模板。

#### P1-11 `IndexBuffer::Create` 参数是 `uint32_t*` 而非 `const uint32_t*`

- **位置**：`Graphics/RenderBuffer.h:124`、`GraphicsAPI/OpenGL/OpenGLBuffer.h:35`
- **现状**：接受非 const 裸指针，隐含"可能被修改"的语义，且调用方无法直接传 `const` 数组。`VertexBuffer::Create(uint32_t*, uint32_t)`（`RenderBuffer.h:139`）的第二个参数含义其实是**字节数**（见 `Renderer2D.cpp:126` 传 `sizeof(...)`），而 `Create(uint32_t count)`（`:138`）的 `count` 也是字节数——**同一个名字两种含义。**
- **修复**：`IndexBuffer::Create(const uint32_t*, uint32_t count)`；`VertexBuffer::Create(uint32_t sizeInBytes)` / `Create(const void*, uint32_t sizeInBytes)` 明确命名。

#### P1-12 `TransformSystem` 与 `TransformComponent` 的"世界矩阵"是假的

- **位置**：`Source/Bamboo/ECS/System/TransformSystem.cpp:20-31`、`ECS/Component/TransformComponent.h:13-15, 20-36`
- **现状**：
  1. `WorldMatrix = LocalMatrix;`（`TransformSystem.cpp:28`）—— 注释还写着"世界矩阵 = 本地矩阵的逆矩阵"，**完全错误**。
  2. 旋转被注释掉（`:21-22`），`Matrix4 rotation;` 定义后**未使用**。
  3. `Dirty` 标志被无条件设为 `false`（`:31`），检查 `Dirty` 的代码被注释掉（`:13-16`）→ **标志位形同虚设**，每帧全量重算所有实体矩阵。
  4. `TransformComponent::SetLocalToWorldMatrix`（`TransformComponent.h:20-36`）**完全没有被调用**。
  5. 没有 `Parent`/`Children` 字段——**"World"这个词在数据模型里没有任何支撑**。
- **修复**：要么老实改名 `LocalToWorld` 并明确"当前无层级"，要么真正引入 `Parent` 组件 + 世界矩阵递归求解。**不建议保留这个中间状态。**

#### P1-13 相机视图矩阵用"取负角度"求逆，只对纯平移+Z 旋转侥幸成立

- **位置**：`Source/Bamboo/ECS/System/CameraSystem.cpp:23-28`
- **现状**：
  ```cpp
  Matrix4 translation = Matrix4::Translate(-transform.Position);
  Matrix4 rotation = Matrix4::RotateZ(-transform.Rotation.z);
  camera.CurrentCamera.SetView(translation * rotation);
  ```
  - 旋转矩阵的逆是转置，"取负角度"**只对单轴纯旋转**等价，多轴复合立刻错。
  - `Matrix4::Inverse()` 已实现且经 `Determinant` 校验（`Matrix4.cpp:108-151`），却在这里被注释掉不用（`:21`）。
  - `Camera *mainCamera = nullptr;`（`:13`）定义后**从未使用**（死变量，`/W4` 会警告）。
  - `if (camera.Primary)` 存在，但 `RendererSystem` 不检查 `Primary`（见 P0-7），两者行为不一致。
- **修复**：用 `transform.WorldMatrix.Inverse()`（或正确的视图矩阵合成）；删死变量；与 `RendererSystem` 统一 `Primary` 判据。

---

### P2 — 设计级（架构耦合与职责错位）

#### P2-1 `Entity.h` 与 `Scene.h` 循环包含

- **位置**：`ECS/Entity.h:3` `#include "../Scene/Scene.h"`；`Scene/Scene.h:17` 前向声明 `class Entity;`，但 `Scene.h:30,32` 的返回值又是 `Entity`（不完整类型）。
- **现状**：`Entity` 的方法体需要 `Scene` 完整定义（访问 `m_Scene->m_Registry`），所以 `Entity.h` 必须包含 `Scene.h`；而 `Scene.h` 需要 `Entity` 作返回值 → 只能前向声明。**可行但极脆弱**：任何在 `Scene.h` 里对 `Entity` 的按值使用（如 `std::optional<Entity>` 或 `std::vector<Entity>`）都会立刻炸。
- **关联**：`Scene::m_Registry` 是 **public**（`Scene.h:48`），纯为了 `Entity` 能访问。**封装被公开成员打破。**
- **修复**：`Entity` 改为只持有 `entt::registry*` + `entt::entity`（不持有 `Scene*`），或把 `m_Registry` 设 private 并 `friend class Entity;`；更推荐改用 `entt::handle`（entt 官方推荐，正好解决"实体+注册表"打包）。

#### P2-2 系统被切成"内置系统"和"逻辑系统"两条流水线

- **位置**：`Scene/Scene.h:52-53`（`m_Systems` / `m_LogicSystems`）、`Scene.cpp:15-18`、`Scene.cpp:37-48`
- **现状**：
  ```cpp
  for (auto &system : m_LogicSystems) system->Update(...);   // 逻辑先跑
  for (auto &system : m_Systems)      system->Update(...);   // 内置后跑
  ```
  - **逻辑系统跑在 Transform 和 Physics 之前**，读到的是上一帧的变换 → 输入响应延迟一帧、物理后处理顺序错。
  - 顺序被硬编码在构造函数里，无法配置、无法禁用、无法在编辑器里展示。
  - 两类系统没有本质区别，只是插入时机不同，却用两个容器表达。
- **修复**：单一 `std::vector`，每个系统带显式 `SystemPhase`/`Priority`，按阶段稳定排序执行。

#### P2-3 "系统只读注册表"这个契约不存在

- 见 P0-8。建议用类型系统强制：`ISystem::OnUpdate(Scene&, float)`（可写）与 `IRenderSystem::OnRender(const Scene&, ...)`（`const` 让编译器强制只读）。成本极低、收益极高。

#### P2-4 `RendererSystem` 每帧做三件本不该它做的事

- **位置**：`ECS/System/RendererSystem.cpp:21-22`（清屏+硬编码清屏色）、`:24-42`（找相机）、`:66-95`（收集+排序+绘制精灵）
- **问题**：
  - **清屏色硬编码** `{0.2f, 0.3f, 0.3f, 1.0f}`（`:21`）——应来自场景/相机/渲染设置。
  - **找相机的逻辑与 `CameraSystem`/`Scene::GetMainCamera` 重复**（P0-7）。
  - **每帧构造 `std::vector<std::tuple<...>>`**（`:69`，堆分配）——应复用持久缓冲；`tuple` 存裸指针，若遍历中 `emplace` 触发 registry 扩容即全部失效。
  - 精灵大小通过 `WorldMatrix * Matrix4::Scale(sprite->Size)` 现算（`:91`），注释自己也承认应前移到 `TransformSystem`（`:90`）——**承认了但没做**。
- **修复**：拆成 `CameraSystem`（选主相机、更新视图矩阵）+ `RendererSystem`（只提交）；复用排序缓冲；清屏色显式配置。

#### P2-5 `Camera` 同时承担"视图矩阵容器"和"相机参数"两种角色

- **位置**：`Graphics/Camera.h:48-52`
- **现状**：`SetView(Matrix4)` 让外部把视图矩阵**灌进来**，相机不知道自己在哪里；同时 `Camera` 又保存 `m_PrimaryCamera`（"我是不是主相机"是**场景级**语义）。且 `Camera` 被 `CameraComponent` 按值持有（`CameraComponent.h:8`），复制语义下 `Primary` 跟着复制。
- **修复**：`Camera` 只管 `Projection + View`（纯数学）；"谁是主相机"由 ECS 表达。移除 `Camera::m_PrimaryCamera`。

#### P2-6 `Texture2D::Create(path)` 依赖全局 `Application` 单例

- **位置**：`Graphics/Texture.cpp:14`
- **现状**：`CreateRef<OpenGLTexture2D>(Application::GetInstance()->GetAssetManager()->Load<ImageAsset>(path))` —— 三层解引用，任何一层为空即崩（`Application` 未构造时 `GetInstance()` 返回 `nullptr`）。而且**图形层依赖 `Game/Application` = 分层倒置**。
- **另**：`Shader` 走 `ReadFile` 自有路径（`OpenGLShader.cpp:118`），完全不走 `AssetManager` ——**两条资源路径不一致**。
- **修复**：资源加载通过显式 `AssetManager&` 参数或服务定位器注入；`Graphics` 层不得 `#include "Game/"`。

#### P2-7 UI 系统是与 ECS、渲染器完全无关的孤岛

- **位置**：`UI/`（7 个文件）
- **现状**：
  - `UIElement` 用自己的 `m_Components`（`std::vector<Ref<UIComponent>>`）**又实现了一套组件系统**，与 entt 并行存在。
  - `UIElement::OnUpdate`/`OnRender` 声明了但 `UIElement.cpp` **没有实现**（`:64-65`）。
  - `Button`/`Text`/`Canvas` 基本是空实现；`Text::OnRender` 是空函数（`Text.cpp:5-8`）。
  - UI 完全没有被 `RendererSystem` 或任何系统调用；没有字体、布局、命中检测。
  - `UIElement : enable_shared_from_this`（`UIElement.h:10`），`SetParent` 里 `shared_from_this()`（`UIElement.cpp:69`）**要求对象必须由 `shared_ptr` 持有**，否则抛 `bad_weak_ptr`；栈上 `UIElement e; e.SetParent(p);` 直接崩。
- **修复**（第 8 节 D5）：要么**并入 ECS**（`UIElementComponent` + `CanvasComponent` + `UISystem` 走同一渲染路径），要么**先移出编译**（`BAMBOO_BUILD_UI` 开关）。**半成品留在库里只会增加维护面。**

#### P2-8 `AssetManager::AsyncLoad` 的缓存判断反了 + 线程不安全 + `detach`

- **位置**：`Assets/AssetManager.h:47-85`
- **现状**：
  ```cpp
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_Assets.find(asset) != m_Assets.end())   // ← find 的是 asset(Ref<Asset>) 而不是 path(string)！类型都对不上
  {
      m_Assets[path] = asset;
  }
  ```
  - `m_Assets` 是 `unordered_map<std::string, Ref<Asset>>`，`find(asset)` 会尝试把 `Ref<Asset>` 转成 `std::string`——要么编译失败，要么语义完全错误。**这段代码几乎肯定编不过或被侥幸跳过。**
  - 正确语义应是"别人先加载了就复用"，现在是"找不到就覆盖写入"。
  - **`std::thread` + `detach()`**（`:84`）：线程持有 `this` 与 `callback`，`AssetManager` 析构后线程继续运行 → 悬垂。且 `m_AssetFactory` 非线程安全。
  - `Load<T>`（`:19-32`）里 `dynamic_pointer_cast<T>` **不检查结果**：加载失败（`AssetFactory::Create` 返回 `nullptr`）时把 `nullptr` **写进缓存**（`:30`），于是**后续每次 Load 都返回 nullptr 且永不重试**。
  - `Load` **没有加锁**，与 `AsyncLoad` 的锁无保护共享 `m_Assets` → 数据竞争。
- **修复**：缓存键统一 `path`；用 `std::future`/线程池替代 detach（或按第 8 节 D9 先移除异步）；`cast` 失败记日志且**不写缓存**；明确"加载失败"与"未加载"两种状态。

#### P2-9 图形抽象层被高频使用，但抽象本身没有兑现

- **位置**：`Graphics/` vs `GraphicsAPI/OpenGL/`
- **现状**：
  - `Renderer::OnWindowResize` 是空函数（`Renderer.cpp:20-22`）—— 窗口缩放时**既不更新视口也不更新相机**。
  - `RendererAPI::Create()`（`RendererAPI.cpp:9-23`）把 `switch` 全注释掉了，无论 `s_API` 是什么都返回 OpenGL 实现。**抽象的"策略"部分被架空。**
  - `Shader::Create`（`Shader.cpp:10-20`）硬编码 `OpenGLShader`，**完全不走 `RendererAPI::GetAPI()`**；而 `UniformBuffer::Create`（`UniformBuffer.cpp:11-19`）和 `Texture2D::Create`（`Texture.cpp:9`）走了。**同一抽象层里两套风格。**
  - `ShaderLibrary`（`Shader.h:40-52`）的 `Load(filepath)` **没有 Add 进库**（`Shader.cpp:32-36`），半成品。
  - `OpenGLShader::SetInt` 是**空实现**（`OpenGLShader.cpp:42-44`）——接口承诺了但实现是空的，调用方以为设置了。
  - `RenderBuffer.h` 定义了 `Mat3`/`Mat4` 的尺寸，但 `OpenGLVertexArray::AddVertexBuffer` 的 `switch`（`OpenGLVertexArray.cpp:40-59`）**没有处理 `Mat3`/`Mat4`**（静默跳过，顶点属性不设置）。
- **修复**（第 8 节 D1）：要么"诚实降级"（合并 OpenGL 实现，删掉空转抽象），要么"兑现抽象"（所有 `Create` 走 `GetAPI()`、补齐空实现、加 Null 后端用于测试）。

#### P2-10 应用初始化顺序错误

- **位置**：`Game/Application.cpp:12-30`
- **现状**：
  ```cpp
  Log::Init();                                   // 只初始化 CoreLogger（Log.cpp:14）
  m_Window = Window::Create({name});             // 建 GLFW 窗口 + 上下文
  m_Window->SetEventCallback(...);
  m_SceneManager = CreateScope<SceneManager>();  // Scene 构造里就建实体、加系统
  m_AssetManager = CreateScope<AssetManager>();
  Renderer2D::Init();                            // 需要 GL 上下文，OK
  Renderer::Init();                              // → RendererCommand::Init() → glEnable(GL_BLEND)
  ```
  问题点：
  1. **`RendererCommand::s_RendererAPI` 是静态成员并靠 `RendererAPI::Create()` 做静态初始化**（`RendererCommand.cpp:5`）。静态初始化早于 `main`，此时**没有 GL 上下文**。当前 `Create()` 只是 `make_unique`（不调 GL）侥幸安全，但这是**静态初始化顺序地雷**：任何在 `RendererAPI` 构造里调用 GL 的改动都会变成"main 之前崩溃"，极难调试。
  2. **`Log::Init()` 只初始化 `s_CoreLogger`**（`Log.cpp:11-17`），`s_ClientLogger` 保持 `nullptr`。`BAMBOO_CLIENT_*` 宏当前无人使用（全仓库仅 `Log.h:14`），但一旦有人用就是空指针崩溃。**要么初始化，要么删掉这套设计。**
  3. `Scene` 构造函数里**就创建了主相机实体**（`Scene.cpp:20-29`）——场景一被 new 出来就有内容，导致无法创建空场景，也无法用序列化器填充。
  4. **`PhysicsSystem::Init()` 从未被调用**（`PhysicsSystem.h:13`），`m_PhysicsWorld` 恒为 `nullptr`（`PhysicsSystem.cpp:8`）。`Scene::AddSystem<Physics::PhysicsSystem>()`（`Scene.cpp:17`）只构造不 `Init`。**系统的 `Init` 生命周期根本没有被引擎调用。**
  5. `Renderer2D::Init()` 在 `Renderer::Init()` **之前**调用，表达出的依赖顺序是错的。
- **修复**：引入显式 `Engine::Initialize()` 阶段表（`Log → Window → Context → RendererAPI::Init → Renderer2D::Init → AssetManager → SceneManager`）；系统 `Init` 由 `Scene` 在注册时调用；`RendererCommand::s_RendererAPI` 改为函数内 `static`（延迟构造）或由 `Engine` 持有。

#### P2-11 `Application` 的生命周期不完整

- **位置**：`Game/Application.h` / `Application.cpp`
- **现状**：
  - `s_Instance` 是**裸指针静态成员**（`Application.h:40`），构造时赋值（`Application.cpp:14`），析构时**不清空** → 析构后 `GetInstance()` 悬垂。
  - **`~Application()` 不是 virtual**（`Application.h:20`）—— `BreakoutApp` 继承它（`BreakoutApp.h:5`），通过 `Application*` 删除时**不会调用派生类析构** → UB 隐患。**多态基类必须 virtual 析构。**
  - `WindowsWindow::Shutdown()` 无条件 `glfwTerminate()`（`WindowsWindow.cpp:110-112`），即使 `glfwInit` 失败也会调用；**多窗口场景下第一个窗口析构就终止整个 GLFW**。
  - `Application::Run()` 里 **`Time::Update()` 的 dt 没有上限钳制**（`Time.cpp:27`），窗口拖动/断点调试后 `deltaTime` 可能是几秒 → 逻辑"瞬移"。经典需要 `dt = std::min(dt, 1/30.f)` 或固定步长累加器。
  - 循环顺序是 `Scene::Update` → `Window::Update`（`Application.cpp:48-50`），即**先更新再轮询事件**。输入靠 `glfwGetKey` 即时查询（`WindowsInput.cpp:9`）侥幸可用，但"事件回调比游戏逻辑晚一帧"这个语义需要明确或调整顺序。
- **修复**：虚析构；析构时清 `s_Instance`；`glfwTerminate` 加守卫；`dt` 钳制。

#### P2-12 `Scene` 的实体索引与生命周期管理脆弱

- **位置**：`Scene/Scene.cpp:56-85`
- **现状**：
  - `m_EntityMap[uuid] = entity` 存 `Entity` 值拷贝。
  - `DestroyEntity`（`:81-85`）**先 `destroy` 再 `GetUUID()`** → 对已销毁实体取组件（见 P0-6）。
  - 没有 `entt::registry::on_destroy` 钩子 → 任何人绕过 `Scene::DestroyEntity` 直接 `registry.destroy()` 都会让 `m_EntityMap` 残留悬垂 UUID。
  - `FindEntityByName` 是 **O(n) 线性扫描**（`:66-79`），且**没有重名检查**（可产生多个同名实体，查找返回第一个）。`GetMainCamera` 依赖它（P0-7）。
  - `CreateEntityWithUUID` 不检查 UUID 冲突（`m_EntityMap[uuid] = entity` 会静默覆盖，反序列化时冲突即丢实体）。
- **修复**：`m_EntityMap` 改存 `entt::entity`；用 `registry.on_destroy` 自动清理；建立名字索引或明确"线性且不唯一"；UUID 冲突检测并报错。

---

### P3 — 体验与可维护性

#### P3-1 性能问题清单（按收益排序）

| # | 位置 | 问题 | 建议 |
|---|---|---|---|
| 1 | `OpenGLShader.cpp:48,53,58,63` | **每帧每次 Set 都调 `glGetUniformLocation`** | `Compile` 后一次性解析并缓存 `unordered_map<string, GLint>`；更好的是走 UBO 彻底避开字符串查询 |
| 2 | `OpenGLBuffer.cpp:43` | `SetData` 用 `glBufferData`（**每次重新分配显存**） | 构造期 `glBufferData(size, nullptr, GL_DYNAMIC_DRAW)`，更新用 `glBufferSubData`（或 orphan 技巧避免同步点） |
| 3 | `Renderer2D.cpp:336-339` | `for (auto texture : s_Data.TextureSlotMap)` **按值拷贝 key**（`Ref` 拷贝 = 原子引用计数） | `for (const auto& [tex, slot] : ...)`；并清醒认识到 key 的相等语义是"指针地址" |
| 4 | `RendererSystem.cpp:69` | 每帧构造 `std::vector<std::tuple<...>>`（堆分配） | 复用成员缓冲 `clear()` |
| 5 | `Renderer2D.cpp:336` | 每个精灵一次 `unordered_map` 查找 | 小数组线性查找（≤32 个）通常更快更可预测 |
| 6 | `WindowsWindow.cpp:36` | **未开启垂直同步**（无 `glfwSwapInterval`） | 明确 vsync 策略（编辑器通常关、游戏通常开） |
| 7 | `Renderer2D` 整体 | `MaxSpriteCount = 100` 上限过低；`Statistics` 从未输出 | 提高上限、按纹理/容量自动换批、统计接入日志或 HUD |
| 8 | `Scene.cpp:45-48` + `TransformSystem` | 每帧全量重算所有实体矩阵（`Dirty` 被无条件清零） | 恢复 `Dirty` 语义 |
| 9 | `CameraSystem`/`RendererSystem` | 每帧两次全量 `registry.view` 遍历 | 缓存主相机实体句柄，仅组件变更时重查 |
| 10 | `WindowsWindow.cpp:101` | `glfwPollEvents` 满速空转，CPU 占用高 | 视需求用 `glfwWaitEventsTimeout(1/60)` 或保留 poll + vsync |

#### P3-2 错误处理几乎不存在

| 位置 | 现状 | 应该 |
|---|---|---|
| `WindowsWindow.cpp:30-34` | `glfwInit()` 失败只 `return`，**错误日志被注释掉了**（`:32`） | 打 critical 日志 + 让 `Application` 知道失败，不要带着空窗口继续跑 |
| `WindowsWindow.cpp:37-42` | 窗口创建失败日志被注释（`:39`） | 恢复日志 |
| `WindowsWindow.cpp:110-112` | `glfwTerminate()` 无条件调用 | 只在 `glfwInit` 成功时调用；多窗口用引用计数 |
| `OpenGLShader.cpp:85-91,100-105` | 编译/链接失败只打日志后 `return`，**留下未链接的程序**，后续 `Bind()` 静默失败 | 标记 `m_IsValid = false`；`Bind()` 检查并回落；或抛异常 |
| `OpenGLShader.cpp:139-145` | 读文件失败只打日志（且打印的是 `fullPath`，与调用方传入路径不符——见 P3-3） | 返回空串并让上层检查 |
| `OpenGLTexture.cpp:84-87` | 纹理加载失败只打日志，`m_RendererID` 未初始化 | 记录失败状态，`Bind`/`SetData` 前置检查 |
| `ImageAsset.cpp:32-35` | 加载失败只打日志，`m_IsLoaded` 保持原值 | 显式置 false 并保留原因 |
| `AssetManager.h:24,27` | `dynamic_pointer_cast` 不检查 | 检查并记日志 |
| `Camera.cpp:24` | 除零（见 P1-6） | 断言/提前返回 |
| `Vector2.h:40` | 除零（见 P1-5） | 统一策略 |
| `FileUtlis.cpp` 多处 | `RenameFile`/`GetAbsolutePath`/`GetCurrentDirectory`/`SetCurrentDirectory` **不捕获异常**（`CopyFile`/`MoveFile`/`CreateDirectory` 都捕获了） | 统一异常策略；`SetCurrentDirectory` 改进程全局状态，多线程下危险，应标注 |
| `SceneSerializer` | 空实现，无错误路径 | 见 S5 |

#### P3-3 `OpenGLShader::ReadFile` 的路径处理有 bug

- **位置**：`Source/Bamboo/GraphicsAPI/OpenGL/OpenGLShader.cpp:118-148`
- **现状**：
  ```cpp
  std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);  // 只取文件名
  m_Name = fileName.substr(0, fileName.find_last_of("."));                  // 覆写 m_Name
  std::string fullPath = std::string(BAMBOO_ASSET_ROOT) + "/" + "Shaders/" + fileName;  // 强制拼 Shaders/
  ```
- **问题**：
  1. `Renderer2D.cpp:139,185,239` 传的是 `"BambooAssets/Shaders/triangle.vert"` 这种相对路径，但 `ReadFile` 只取文件名再拼 `BAMBOO_ASSET_ROOT/Shaders/` → **恰好能用**。换目录结构立刻失效，且**传入的路径前缀被完全忽略**。
  2. `m_Name` 被**每个** `ReadFile` 调用覆写。`OpenGLShader(name, vert, frag)`（`:18-25`）先设 `m_Name = name`，然后 `ReadFile(vert)` 与 `ReadFile(frag)` 各覆写一次 → **最终 `GetName()` 返回片元着色器的文件名**。`ShaderLibrary` 若按 name 索引就会错乱。
  3. 错误日志打印拼接后的 `fullPath`，与调用者传入的路径不符 → 排错时被误导。
- **修复**：`ReadFile` 只读文件、不改路径；路径解析抽成 `AssetPath` 工具；`m_Name` 只在构造时设置一次。

#### P3-4 资源路径完全依赖**编译期烤入的绝对路径**

- **位置**：`Source/Bamboo/Config.in.h` → `Source/build/generated/Config.h`（当前值 `#define BAMBOO_ASSET_ROOT "D:/project/StudyProject/Bamboo/Source/BambooAssets"`），消费点：`OpenGLShader.cpp:123`、`OpenGLTexture.cpp:45`、`ImageAsset.cpp:16`
- **现状**：路径在 **CMake configure 时**烤死。后果：
  - 仓库移动/克隆到别的机器 → **必须重新 configure**，否则资源静默加载失败（失败路径只有日志，见 P3-2）。`AGENTS.md:24-29` 把这个坑写进文档了——**这是"用文档解释设计缺陷"，应该从设计上消除。**
  - 换工作目录运行也不行。
- **修复**：
  1. 运行期解析：环境变量 `BAMBOO_ASSET_ROOT` → 可执行文件相对路径 → 编译期默认值。
  2. 或构建后把 `BambooAssets/` 拷到输出目录（`add_custom_command(TARGET ... POST_BUILD ...)`），让资源随可执行文件分发。
  3. 无论如何：**加载失败必须可见**。

#### P3-5 `Context`/`RendererAPI`/`Shader` 的静态初始化顺序隐患

- 见 P2-10 第 1 点。`RendererCommand.cpp:5` 的 `static Scope<RendererAPI>` 在 `main` 前构造。`Renderer2D.cpp:119` 的 `static Renderer2DData s_Data;` 也同理。建议改函数内 `static`（Meyers singleton）或延迟到 `Init()` 分配。

#### P3-6 无测试、无警告、无静态检查、无性能基线

- 仓库内没有任何测试框架（`AGENTS.md:33` 确认"验证方式就是构建成功"）。
- 没有 `/W4`（更别说 `/WX`）；当前代码里明显会有未使用变量/参数、隐式转换、类型不匹配等警告（如 `CameraSystem.cpp:13` 的 `mainCamera`、`Camera.cpp:16-19` 的空 if 体、`Renderer2D.cpp:351` 的 `inPos` 未使用、`Renderer2D.cpp:197` 的类型不匹配）。
- `Renderer2D::Statistics` 只有一个 `DrawCalls` 且**从未被读取**；`Time::GetFps()` 从未展示。**没有性能可观测性，就无法做 `roadmap.md` 要求的"前后对比"。**
- **修复顺序**：`/W4`（先不加 `/WX`，把警告当待办清单）→ gtest 骨架（Math/ECS 优先，不依赖 GL）→ `/WX` → CI。

---

### P4 — 工程化与构建

#### P4-1 `ThirdParty/CMakeLists.txt` 的问题

- **位置**：`Source/ThirdParty/CMakeLists.txt`
- **问题**：
  1. `:10` `<...>/entt/include()` —— 路径里多了空括号（**P0-10**）。
  2. `add_subdirectory(./ThirdParty ${CMAKE_BINARY_DIR}/ThirdParty_build)`（`Source/CMakeLists.txt:16`）把第三方构建目录**嵌到 build 根下**，而 `CMAKE_RUNTIME_OUTPUT_DIRECTORY` 等又是全局设置 → 第三方 target 也被塞进 `bin/`；`build/` 里同时有 `.vs/`、`x64/`、`CMakeFiles/`、`bin/`、`lib/`、`ThirdParty_build/`，结构混乱。
  3. `imgui` target 用 `target_link_libraries(imgui PUBLIC glfw)`（`:71`）—— 只有 Editor 用 imgui，却让 imgui 公共传播 glfw 依赖。
  4. 文件名 `CmakeLists.txt`（小写 m）在大小写敏感文件系统上会有问题。
- **修复**：修正路径；用 target 级输出目录覆盖替代全局；明确第三方库类型与 `EXCLUDE_FROM_ALL`。

#### P4-2 `Source/CMakeLists.txt` 的问题

- **位置**：`Source/CMakeLists.txt`
- **问题**：
  1. `Bamboo` 在 `:19` 就 `add_subdirectory`，而生成的 `Config.h` 的 include 到 `:36` 才加（CMake 允许后追加所以能生效，但顺序表达是混乱的）；`Editor` 没加 generated 的 include（当前侥幸不需要）。
  2. **没有 `BAMBOO_BUILD_TESTS` / `BAMBOO_BUILD_EDITOR` / `BAMBOO_BUILD_SANDBOX` 选项** —— 三个 target 无条件构建。
  3. **没有 install/export 规则**，引擎无法被外部项目使用。
  4. 没有 `CMakePresets.json` —— `Source/生成工程命令.txt` 里手敲 `cmake .. -G "Visual Studio 17 2022" -A x64` 正是缺 presets 的症状。
  5. `Source/生成工程命令.txt` 与 `Source/Bamboo/CMakeLists.txt` **带 UTF-8 BOM**（看不见但存在）。CMake 通常容忍，某些工具链会报 unexpected character。建议统一存为 UTF-8 无 BOM。
- **修复**：加 `option()` 开关、`CMakePresets.json`、`install()` 规则、统一去 BOM。

#### P4-3 头文件包含风格混乱且脆弱

- **现象**：同一文件里混用多种包含风格，且都依赖"从 `Source/` 出发"的相对路径：
  - `"../Bamboo/Core/Log.h"`（从 `Source/Bamboo/` 出发，**绕回自己**）
  - `"../Core/Log.h"`（正确的相对路径）
  - `"./Core/Ref.h"`（从哪出发？）
  - `"Bamboo/Math/Color.h"`（`ECS/Component/Shape/RectangleComponent.h:2`，能编过是因为 `Source/Bamboo` 在 include 路径里）
  - `"entt.hpp"`（依赖第三方 include 路径）
  - `#include "../Bamboo/core/Ref.h"`（`Assets/AssetFactory.h:5`，**小写 `core`！** 在 Windows 大小写不敏感的文件系统上侥幸通过）
- **后果**：文件移动/重命名成本极高；非 Windows 平台立刻崩；`AssetFactory.h:5` 的小写 `core` 是**明确的定时炸弹**。
- **修复**：
  1. 建立**单一包含根**：`target_include_directories(Bamboo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)`（即 `Source/`），然后**全仓库统一 `#include "Bamboo/..."`**（或统一 `#include "Core/Log.h"`）。
  2. 用脚本批量替换（一次性投入，之后不再纠结）。**这是"重构升级"里收益最直接的一项。**

#### P4-4 源文件收集用 `GLOB_RECURSE`

- **位置**：`Bamboo/CMakeLists.txt:3-6`、`Sandbox/CMakeLists.txt:1-4`
- **问题**：新增文件**不会自动触发重新配置**，必须手动 re-run CMake。`AGENTS.md:20` 把这当成"注意事项"写进文档了 —— 又是"用文档解释设计缺陷"。
- **修复**：显式列出源文件（最稳），或保留 GLOB 但加 `CONFIGURE_DEPENDS`：
  ```cmake
  file(GLOB_RECURSE BAMBOO_SRC CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
  ```

#### P4-5 `Editor/CMakeLists.txt` 的问题

- **位置**：`Source/Editor/CMakeLists.txt`
- **问题**：
  1. `:64` `source_group(TREE ... FILES ${BAMBOO_SRC})` —— **`BAMBOO_SRC` 是父作用域变量**（来自 `Bamboo/CMakeLists.txt`），跨 target 复用同一源文件列表变量，IDE 分组会错乱。
  2. `:66-68` `add_compile_options("/utf-8")` —— 用**全局**命令（虽然 Editor 是最后一个 target）。应改 `target_compile_options(Editor PRIVATE ...)`。同样问题在 `Bamboo/CMakeLists.txt:24-26`。
  3. `:49-55` 显式重复链接 `imgui spdlog stb glfw` —— 已经通过 `Bamboo` 的 `PUBLIC` 传递，重复且脆弱。
  4. **没有 `VS_DEBUGGER_WORKING_DIRECTORY`**（只有 Sandbox 有），编辑器运行时 cwd 不确定。
  5. 大量注释掉的源文件（`:10-13, 18-21, 26-28`）。
- **修复**：删除 `BAMBOO_SRC` 复用、改 target 级编译选项、精简链接、加工作目录。

#### P4-6 缺少 `.gitignore`（**当前影响最大的一条**）

- **位置**：仓库根目录（`D:\project\StudyProject\Bamboo\`）
- **现状**：`Test-Path .gitignore` → **False**。`git status` 显示未跟踪：
  ```
  ?? .VSCodeCounter/
  ?? .vs/
  ?? .vscode/
  ?? Code-queries-main/
  ?? Code-queries-main.zip
  ?? Source/build/
  ?? all_bamboo_files.txt
  ?? all_source_files.txt
  ```
- **后果**：`Source/build/` 里有 40MB+ 的 `Bamboo.lib`/`Sandbox.exe`/`.pdb`；一次 `git add -A` 就会把它们全部提交进历史，**永久污染仓库**（Git 历史无法轻易瘦身）。
- **修复（立即执行，成本 5 分钟）**：
  ```gitignore
  # 构建产物
  Source/build/
  build/
  out/
  # IDE / 编辑器
  .vs/
  .vscode/
  .VSCodeCounter/
  *.sln
  *.vcxproj
  *.vcxproj.filters
  *.vcxproj.user
  # 生成物
  all_*.txt
  Code-queries-main/
  Code-queries-main.zip
  # 编译中间件
  *.obj
  *.pdb
  *.ilk
  *.idb
  *.exp
  *.lib
  *.exe
  *.dll
  ```
  （`*.sln`/`*.vcxproj` 是否忽略取决于你是否希望别人不开 CMake 就能打开工程——建议忽略，CMake 会重新生成。）
- **同时**：`docs/roadmap.md:250-256` 已把它列为待办，**现在就该做掉**。

#### P4-7 没有 CI、没有 CMakePresets、没有版本管理

- 无 `.github/workflows/`（`roadmap.md` 的 P1.3 未落地）。
- 无 `CMakePresets.json`。
- `project(BambooEngine)`（`Source/CMakeLists.txt:2`）**没有 VERSION**。
- **修复**：`project(BambooEngine VERSION 0.1.0 LANGUAGES C CXX)`；加 presets（`debug`/`release`/`ci`）；加最小 CI（Windows runner + configure + build + ctest）。

---

### P5 — 命名与文档

#### P5-1 命名与拼写问题（完整对照表见附录 A）

高优先级：

| 现状 | 建议 | 位置 |
|---|---|---|
| `ShaderDatatType` | `ShaderDataType` | `Graphics/RenderBuffer.h:9` |
| `FileUtlis` | `FileUtils`（同时改文件名） | `Core/FileSystem/FileUtlis.h/.cpp` |
| `Shoudown` | `Shutdown` | `Editor/Source/Core/Application.h:19`、`.cpp:81` |
| `GetNormalTexture` | `GetWhiteTexture`（它返回纯白 1×1 纹理，不是法线贴图） | `Renderer2D.h:31`、`.cpp:457-460` |
| `IsDirector` | `IsDirectory` | `FileUtlis.h:21` |
| `vertexPaht`/`fragmentPaht` | `vertexPath`/`fragmentPath` | `Graphics/Shader.h:37` |
| `Vector2Int`（不存在的类） | `IntVector2`（已存在） | `Math/Rect.h:68,77` |

低优先级（纯风格）：`Core/Base.h:8` 的 `BIT(x)` 宏（应改 `constexpr`，且**用完要 `#undef`**，否则污染所有包含者）；`Math/MathDefs.h` 的 `B_PI`/`B_TWO_PI`/`B_HALF_PI`/`B_EPSILON` 宏（应改 `constexpr float`，并**修掉 `B_HALF_PI` 的值错误**——当前 `1.41592653589793238`，正确应为 `1.5707963267948966`）。

#### P5-2 死代码与注释代码堆积

| 位置 | 内容 |
|---|---|
| `Sandbox/main.cpp:41-109` | 整段被 `/** ... **/` 包住的旧测试代码（约 70 行） |
| `Renderer2D.cpp:106,163,219,331-334,401-408,418,428-430` | 注释掉的 `TextureSlots` 数组方案残留 |
| `Renderer.cpp:21` | 注释掉的 `SetViewport` |
| `RendererAPI.cpp:10-22` | 注释掉的 `switch` |
| `Camera.h:30`、`Camera.cpp:32-35,75` | 注释掉的 `GetViewMatrix`/`LookAt`/旧正交计算 |
| `TransformSystem.cpp:13-18` | 注释掉的 `Dirty` 检查 |
| `PhysicsSystem.cpp:24-30` | 空的分支体 + `// todo` |
| `Editor/Source/Core/Application.cpp:45-56,63,69-76` | 注释掉的 while 循环与说明 |
| `Sandbox/main.cpp:6-13,35-37,97-106` | 注释掉的内存检测代码 |
| `Entity.h:31,38,51` | 注释掉的断言行 |
| `Renderer2D.cpp:73,88-89,93-94` | 注释掉的旧实现 |
| `OpenGLTexture.cpp:7-8` | 注释掉的 include |
| `Scene.h:20` | 注释掉的 `// class PhysicsSystem;` |
| `Math/Vector4.cpp` | **0 字节空文件** |
| `Log.cpp` | 客户端日志器从未初始化（见 P2-10） |

- **修复**：一次性大扫除。**判据**：如果是"上一版实现"，删掉（`git log` 里有）；如果是"未完成的 todo"，写成 `TODO(名字): 具体内容` 保留，或删掉并在 issue 里跟踪。**注释掉的大块代码是"恐惧型编程"，它让读者无法判断哪些是活的。**

#### P5-3 文档现状与建议

| 文档 | 现状 | 建议 |
|---|---|---|
| `README.md` | 12 字节（"learn cpp "） | 需要真正的 README：项目定位、构建步骤、目录说明、当前能力边界（哪些能用哪些不能用） |
| `AGENTS.md` | **准确**（构建方式、asset root 坑、约定都对） | 保留；把 `refactor_plan.md` 链接进去作为当前工作重点 |
| ~~`docs/project_structure.md`~~ | 22KB，**已过时**（仍描述已删除的 `SpriteRendererSystem`，把 `Vector4`/`Octree`/`SceneCamera` 说成已实现） | ✅ **已执行（2026-09-18）**：文件已删除；有价值的结构信息核对后并入 `docs/architecture.md`，作废清单见该文档 §9 |
| `docs/roadmap.md` | 12KB，**内容质量不错**，但定位是"学 C++"而非"修引擎"，部分行号已过期 | 保留为"学习路线"；本文件作为"技术重构路线"；两者开头互相链接并明确分工 |
| `docs/refactor_plan.md` | **本文件** | 执行时在每条后面打勾 |

---

## 3. 不合理的地方（设计层面总评）

这一节不重复具体缺陷，而是抽象出**七个结构性问题**。修 bug 是一时的，这七条决定了引擎能不能长大。

### 3.1 "黑盒约定"多于"类型约束"

引擎里大量正确性依赖**人的记忆**而不是**编译器的检查**：

| 约定 | 现在是靠什么保证的 | 应该靠什么 |
|---|---|---|
| "先 `SetOrthographic` 再 `SetViewportSize`，否则投影是单位阵" | 记忆（`Scene.cpp:28-29` 恰好这个顺序） | 投影矩阵惰性求解，参数变了自动重算（`m_Dirty` + `GetProjection()` 内重算） |
| "资源路径必须重新 configure" | `AGENTS.md` 文档 | 运行期路径解析（P3-4） |
| "加了源文件要手动 re-run cmake" | `AGENTS.md` 文档 | `CONFIGURE_DEPENDS`（P4-4） |
| "`Entity` 必须来自 `Scene::CreateEntity`，否则一用就崩" | 记忆 | `m_Scene` 判空 + `IsValid()`（P0-6） |
| "`Color(int a,int r,int g,int b)` 是 ARGB" | 记忆 | 删除这个重载（P1-4） |
| "`DrawSprite` 的 texture 参数不能传临时对象" | 记忆（非 const 引用） | `const&`（P1-9） |
| "纹理槽用完后 `NextBatch` 会重置，别跨批持有序号" | 记忆 | 类型上区分"批内索引"与"全局句柄" |

**改法**：把每条约定转成"要么编译期报错、要么 Debug 断言、要么 API 上根本表达不出来"。

### 3.2 三层坐标系没有边界，像素/世界/NDC 混着用

现状里同一帧内出现了至少四套坐标：

| 坐标 | 出现位置 | 依据 |
|---|---|---|
| 像素（1280×720） | `Renderer2D.cpp:351` 除以 1280/720；`BreakoutApp.cpp:39` 的 `-640` | **硬编码常量** |
| 世界（y∈[-720,720]） | `Camera::ReCalculateProjectionMatrix`（`Camera.cpp:38-42`） | `m_ViewportHeight * 0.5` |
| NDC（±0.5） | `QuadVertexPosition`/`SpriteVertexPositions`（`Renderer2D.cpp:158-161,213-216`） | 硬编码顶点 |
| 屏幕→世界转换 | `Camera::ScreenToWorldPosition`（`Camera.cpp:55-62`） | `m_CameraData`（未初始化，且**无任何调用者**） |

**这是引擎里最容易反复出 bug 的地方。** 必须建立明确边界：

- **NDC 只存在于渲染器内部的顶点常量里**，外部可见的只有"世界坐标"。
- **世界单位 = 1 像素**（2D 引擎最直观），投影矩阵由 `视口宽高` + `相机中心` + `缩放` 唯一确定。
- 所有"像素→世界"集中在一个 `Camera::ScreenToWorld`；所有"世界→像素"集中在一个 `Camera::WorldToScreen`。**其他地方不允许出现 1280/720/640/360 这类数字。**
- 提供一个 Debug 叠加层显示世界原点、相机框、鼠标世界坐标——**这是验证坐标系正确性的最快手段**。

### 3.3 "系统"的契约没有定义

`ISystem` 只有 `Init()` 和 `Update(registry&, dt)`（`ISystem.h:5-8`）：

- 没有执行阶段/优先级 → 顺序靠 `Scene` 构造函数里 `push_back` 的**书写顺序**（`Scene.cpp:15-18`），用户系统只能"最先跑"。
- 没有 `Init` 的调用时机 → `PhysicsSystem::Init` 成了死代码。
- 没有"只读/可写"区分 → 渲染系统堂而皇之写 ECS（P0-8）。
- 没有 `OnEntityCreated`/`OnEntityDestroyed` 钩子 → 组件生命周期逻辑只能写在用户代码里。
- 没有启用/禁用、没有调试名（无法在编辑器里列出系统）。

**建议契约**：

```cpp
enum class SystemPhase { PreUpdate, Update, PostUpdate, PreRender, Render };

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual std::string_view GetName() const = 0;
    virtual SystemPhase GetPhase() const { return SystemPhase::Update; }
    virtual int  GetPriority() const { return 0; }        // 同阶段内排序
    virtual void OnInit(Scene&) {}                        // 由 Scene 调用
    virtual void OnShutdown(Scene&) {}
    virtual void OnUpdate(Scene&, float dt) = 0;          // 可写
};
// 渲染系统单独走只读接口，由编译器强制
class IRenderSystem {
public:
    virtual ~IRenderSystem() = default;
    virtual void OnRender(const Scene&, Renderer2D&) = 0; // const Scene& → 编译期只读
};
```

### 3.4 组件是"数据 + 行为 + 依赖"的混合体

- `CameraComponent` 按值持有 `Camera`（`CameraComponent.h:8`），而 `Camera` 又是"参数 + 视图矩阵 + 主相机标记"的混合体（P2-5）。
- `RigidbodyComponent` 有 `Vector2 Friction`，`BoxCollider2DComponent` 也有 `float Friction`——**摩擦力有两个来源，谁生效？**
- `BallComponent` 有 `Velocity`（`BallComponent.h:9`），而 `RigidbodyComponent` **没有** `Velocity`（只有 `Force`/`Impulse`）——**"速度"这个最基本的状态在物理组件里缺失**，于是 `BallSystem` 只能自己维护速度、物理系统无法接管。
- `SpriteRendererComponent` 有 `Visible`/`Opacity`（`SpriteRendererComponent.h:17-19`）但渲染器**从不检查**它们（`RendererSystem.cpp:81-95`）——**数据存在但不生效，比不存在更糟**（用户以为设置了）。
- `TriangleComponent::Size`/`QuadComponent::Size` 从未被使用。

**建议**：给每个组件写清"字段的语义 + 谁读它 + 单位"，并把"声明了但没人读"的字段删掉或接上。

### 3.5 所有权模型三套并存

| 所有权表达 | 使用位置 | 问题 |
|---|---|---|
| `Scope<T>` = `unique_ptr` | `Window`、`SceneManager`、`AssetManager`、`ISystem`、`RendererAPI`、`PhysicsWorld`、`GraphicsContext` | 清晰 |
| `Ref<T>` = `shared_ptr` | `Texture`、`Shader`、`VertexArray`、`VertexBuffer`、`IndexBuffer`、`UniformBuffer`、`Asset`、`UIElement`、`UIComponent` | GPU 资源用 shared_ptr 意味着**析构时机不确定**；`TextureSlotMap` 用 `shared_ptr` 做 key 更是把"资源身份"和"引用计数"耦在一起 |
| 裸 `new[]`/`delete[]` | `Renderer2D` 的 `TriangleVertices`/`QuadVertices`/`SpriteVertices`/`CircleVertices`（`:133,157,197,212`）、`Shutdown`（`:452-454`） | **手动内存管理 + 显式 Shutdown**，任何提前 return 或异常都泄漏；且 `Shutdown` 里**漏了 `CircleVertices`**（`:452-454` 只 delete 三个）—— 又一个 P0 |

**建议**：
1. GPU 资源（VAO/VBO/IBO/Shader/Texture/UBO）用 `unique_ptr` + 明确的 `Device`/`ResourceManager` 持有，**渲染器只持裸引用/指针**。
2. 每帧临时数据用 `std::vector` 复用。
3. **全仓库禁止裸 `new[]`/`delete[]`**（第三方除外）。
4. 补 `Renderer2D::Shutdown` 遗漏的 `CircleVertices`。

### 3.6 抽象层与实际实现脱节（"为了抽象而抽象"）

引擎有一整套 `Graphics/`（接口）+ `GraphicsAPI/OpenGL/`（实现）的双层结构，但：

- `RendererAPI::Create()` 的 switch 被注释（P2-9）。
- `Shader::Create` 绕过 `RendererAPI::GetAPI()`（P2-9）。
- `OpenGLShader::SetInt` 是空实现——**接口承诺了但实现是空的**，这比不声明更糟。
- `UniformBuffer` 有完整抽象，但 `Renderer2D` 用它只传一个 `Matrix4`，而 shader 侧还得靠 `SetMat4`——**两条路都开着，但都不完整**。
- `RenderBuffer.h` 定义了 `Mat3`/`Mat4`/`Bool` 的类型大小，但 `OpenGLVertexArray::AddVertexBuffer` 的 switch **没有处理 `Mat3`/`Mat4`**——矩阵属性需要多次 `glVertexAttribPointer`，当前实现不支持。

**这是 C++ 引擎里最经典的坑**：抽象层的成本是"每个概念维护 2~3 份定义"，如果抽象不能带来实际收益（现在根本没有第二个后端），它就是纯粹的负债。

**两种出路（第 8 节 D1）**：

- **A. 诚实降级**：承认 OpenGL-only，把 `GraphicsAPI/` 合并进 `Graphics/`（或至少去掉"接口 + 工厂 + 唯一实现"的三层套娃），代码量减少约 30%，改动路径清晰。以后真要换后端时再抽——那时你已知道真实需求。
- **B. 兑现抽象**：所有 `Create` 走 `RendererAPI::GetAPI()`；补齐 `SetInt` 等空实现；把 `RendererState`（混合/深度/剔除）抽成命令对象；加一个 "Null/Stub 后端" 用于自动化测试（这个后端本身就是抽象层的第一份收益）。

### 3.7 每个类都在"顺手加功能"，没人管边界

- `Camera` 管投影、管视图、管"我是不是主相机"、还存 `m_Forward/m_Up`（P2-5）。
- `RendererSystem` 管清屏、管找相机、管排序、管补默认纹理（P2-4）。
- `Scene` 管实体、管系统、管"创建默认主相机"、还管按名字查找（P2-12）。
- `AssetManager` 管缓存、管异步、管类型转换、还管工厂（P2-8）。
- `Renderer2D` 一个类里塞了 4 种图元的批次状态，每种都有自己的 `*Vertices`/`*VerticesPtr`/`*IndexCount`/`*Buffer`/`*VertexArray`/`*Shader`（`Renderer2D.cpp:51-107`）——**6 个字段 × 4 种图元 = 24 个成员**，任何修改都要在 4 处同步。

**改法**：抽一个 `Batch<TVertex>` 结构体，把"缓冲 + 指针 + 计数 + 容量 + 布局 + VAO + Shader"打包成可复用单元，`Renderer2DData` 只持有 4 个这样的批次。**这是让渲染器可维护的关键一步。**

---

## 4. 要完善的地方（能力缺口）

按"对 2D 游戏引擎的必要程度"排序。标记：🔴 阻塞基本可用 / 🟡 显著影响体验 / 🟢 锦上添花。

### 4.1 🔴 渲染能力

| 缺口 | 现状 | 目标 |
|---|---|---|
| 相机参数可控 | `SetOrthographic` 无效（P1-6），无缩放/无平移跟随 | `Camera` 支持中心/缩放/旋转，`Renderer` 提供 UI 调试叠层 |
| 分辨率无关 | 硬编码 1280/720（P1-7） | 任意窗口/DPI 下正确 |
| 窗口缩放响应 | `Renderer::OnWindowResize` 空（P2-9） | 正确更新视口 + 相机宽高比；支持黑边/信箱模式 |
| 纹理翻转策略一致 | stb 全局翻转 + UV 叠加（P1-9） | 单一策略，单元测试覆盖 |
| 批处理容量 | 100 精灵上限（P0-4） | 可配置（1k~10k），按纹理/容量自动换批 |
| 图元完备 | 只有三角形/四边形（且忽略参数）/圆（未接通）/精灵 | 直线、圆环、多边形、文本、9-slice 精灵 |
| 渲染统计 | `Statistics` 只有 `DrawCalls` 且从不输出 | DrawCall/顶点数/批次数/CPU 帧耗时，HUD 或日志输出 |
| 深度/Z 排序 | 精灵按 `ZOrder` 排序，三角形/四边形不参与排序 | 统一排序键（`ZOrder` + 图层 + 稳定序） |
| 图层/遮罩 | 无 | `Layer` + `SortingLayer` 概念 |
| 帧缓冲/离屏渲染 | 无 `FrameBuffer` 抽象 | 编辑器视口、后处理、截图的必要前置 |
| 文本渲染 | 无 | 位图字体或 `stb_truetype` + 图集 |

### 4.2 🔴 物理能力（**当前几乎为零**）

| 缺口 | 现状 |
|---|---|
| 刚体积分 | `PhysicsSystem::Update` 是**空函数**（`PhysicsSystem.cpp:15-32`，只有一个空 `if/else`） |
| `PhysicsWorld::Step` | 被注释掉了（`PhysicsWorld.h:15`），`PhysicsWorld.cpp` 内容为空 |
| `PhysicsWorld` 从未创建 | `PhysicsSystem::Init` 从未被调用，`m_PhysicsWorld` 恒 `nullptr` |
| 碰撞检测 | 完全没有。`BoxCollider2DComponent`/`CircleColliderComponent` 定义了但**无任何代码读取** |
| 碰撞响应 | 无 |
| 空间分区 | `Scene/Octree.h` 只有 `class Octree {};`（3 行空类） |
| 速度状态 | `RigidbodyComponent` **没有 `Velocity` 字段**——最基础的状态缺失 |
| 摩擦力归属 | 刚体和碰撞体各有一个 `Friction`，语义未定 |
| 单位/量纲 | 世界单位与像素的关系未定（§3.2），重力 `-9.8f`（`PhysicsWorld.h:19`）在"1 单位 = 1 像素"下会瞬间穿屏 |
| 触发器/碰撞层 | `IsTrigger` 存在但无实现；无碰撞层/掩码 |
| 物理调试可视化 | 无 |

**物理建议路线**：
1. **先定单位**（推荐"1 世界单位 = 1 像素"，重力按 `pixels/s²` 配置，如 `-980`）。
2. **纯数学层先行**：`Rigidbody { position, velocity, acceleration, mass, invMass, restitution }` + 半隐式欧拉积分 + `dt` 钳制（P2-11）。**这一层完全可单元测试，不碰 GL、不碰 ECS。**
3. **碰撞检测**：AABB（先修 `Merge`）→ 圆-圆 → 圆-AABB → 旋转矩形 SAT。
4. **响应**：先位置修正（分离）+ 速度反弹（恢复系数），再加冲量法（含摩擦）。
5. **空间分区**：均匀网格（2D 比八叉树合适得多），删除或改名 `Octree`。
6. **接入 ECS**：`RigidbodyComponent` + `ColliderComponent` + `PhysicsSystem`。
7. **调试绘制**：碰撞体线框叠加（依赖 4.1 的图元能力）。

### 4.3 🔴 场景与数据驱动

| 缺口 | 现状 |
|---|---|
| 序列化 | `SceneSerializer.cpp` **5 行，全空** |
| 场景文件格式 | 无 |
| 场景管理 | `SceneManager::LoadScene(name)` **忽略 `name`，永远 `CreateRef<Scene>()`**（`SceneManager.cpp:6-10`）；无注册表、无切换、无过渡 |
| 预制体/模板 | 无 |
| 实体层级 | 无 `Parent`/`Children`；`TransformSystem` 的 "WorldMatrix" 是假的（P1-12） |
| 实体启用/禁用 | 无 `Active` 概念（`Visible` 存在但渲染器不读） |
| 组件注册/反射 | 无。序列化需要"类型名 → 读写函数"的注册机制 |
| 资源引用序列化 | 组件里直接存 `Ref<Texture2D>`（`SpriteRendererComponent.h:11`）——**GPU 句柄不可序列化**，必须改为"资源路径/ID + 运行时解析" |

**建议**：
1. **组件序列化注册**：`ComponentRegistry::Register<T>(name, ser, deser)`。
2. **JSON**：见第 8 节 D2。
3. **资源引用**：组件存 `AssetHandle`（UUID 或路径），由 `AssetManager` 解析。
4. **往返测试**：`序列化 → 反序列化 → 序列化`，两次输出必须字节一致（序列化器唯一可信的验收方式）。

### 4.4 🟡 资源管理

- 引用句柄（`AssetHandle`）与生命周期；现在 `Ref<Asset>` 直接暴露给组件。
- 依赖追踪（纹理依赖图像、材质依赖着色器）、卸载时的引用计数。
- 资源热重载（文件监听 + 重新加载 + 通知使用者）。
- 路径解析统一（P3-4）。
- 异步加载的正确实现（P2-8）。
- 资源元数据（`.meta`）+ 稳定 UUID（现在 key 是路径字符串，重命名即断链）。

### 4.5 🟡 编辑器（当前是"看起来像编辑器"的空壳）

`Editor/Source/Core/Application.cpp`：
- **完全绕过引擎**：自己 `glfwInit` + `glfwCreateWindow`（`:18,35`），不用 `Bamboo::Application`、不用 `Window` 抽象、不用 `Scene`、不用 `Renderer2D`。**两个 `Application` 类同名不同命名空间**（`Bamboo::Application` vs `BambooEditor::Application`）。
- `ImGui_ImplOpenGL3_Init` **从未调用**（只 `ImGui_ImplGlfw_InitForOpenGL`，`:59`）→ ImGui 无法渲染（缺 GL3 后端）。
- `Initialize()` 在 `Run()` 里**被调用两次**（构造函数 `:12` 一次，`Run` 里 `:39` 一次），且 `glfwCreateWindow` 在 `Initialize` **之前**（`:35` vs `:39`）→ 顺序混乱。
- 无 `IMGUI_CHECKVERSION()`；无 `ImGui_ImplGlfw_Shutdown`/`ImGui::DestroyContext`（`:81` 的 `Shoudown` 是空函数）。
- `main.cpp:8-9` 用 `std::cin >> i` 暂停控制台——**调试残留**。
- 没有视口/层级面板/属性检查器/内容浏览器（`CMakeLists.txt` 里全是注释）。

### 4.6 🟡 输入系统

- `KeyCodes.h` **只有 A–Z**（`KeyCodes.h:12-38`）；没有数字、方向键、功能键、鼠标键、修饰键。
- `Input` 只有 `IsKeyPressed` 和 `GetMousePosition`（`Input.h:14-16`）：没有"本帧刚按下/刚释放"、没有鼠标按键/滚轮、没有手柄/触屏、没有文本输入。
- `Input` 通过 `Application::GetInstance()` 拿窗口（`WindowsInput.cpp:8,16`）——**空指针链**，且 `glfwGetKey` 前不检查 `window` 是否为空。
- `KeyEvent` 有 `m_Repeat` 但 `WindowsWindow` 的 `GLFW_REPEAT` 分支是空的（`WindowsWindow.cpp:81-84`），所以 `isRepeat` 永远是 `false`。
- `glfwSetCharCallback`（`:88-91`）和 `glfwSetMouseButtonCallback`（`:93-97`）是空 lambda。
- 没有输入映射/动作抽象（`InputAction`/`InputAxis`），游戏逻辑直接绑物理按键。

### 4.7 🟡 音频、字体、文本

完全没有。2D 游戏引擎的"游戏感"一半来自音效；`UI/Text` 需要字体图集/SDF。建议排在中后期。

### 4.8 🟢 调试与工具

- 无 Debug 叠层（FPS、DrawCall、实体数、鼠标世界坐标、相机框）。
- 无日志分级配置（`Log::Init` 硬编码 `trace` 级别，`Log.cpp:16`——**trace 全开会严重拖慢 Release**）。
- 无控制台/命令系统。
- 无崩溃处理器/dump。
- 无 GL 调试回调（`glDebugMessageCallback`）——`roadmap.md` 的性能清单提到了但未接。

### 4.9 🟢 打包与分发

- 无资源随包分发（P3-4）。
- 无图标/版本信息/窗口图标（`glfwCreateWindow` 没设 icon）。
- 无 installer/zip 产物。

---

## 5. 重构路线图

设计原则（沿用 `roadmap.md` 的精神，但把"学习导向"换成"可交付导向"）：

1. **每个阶段必须有一条可运行的验收命令**（不是"感觉好了"）。
2. **每个阶段结束必须能编译 + 通过已有测试 + 跑通 Breakout**。
3. **不在一个阶段里同时改渲染和物理**——出问题时无法定位。
4. **每阶段开独立分支**，按 Conventional Commits 提交（`roadmap.md` 已有规范，直接沿用）。
5. **改前先补测试**（纯逻辑模块）；渲染这类难测的部分用"截图对比 + 缓冲内容数值断言"兜底。

---

### S0 — 止血：让工作区回到"干净可编译"（0.5~1 天）

**目标**：`cmake --build build --config Debug` 零错误，警告数量有基线记录。

| # | 任务 | 文件 | 验收 |
|---|---|---|---|
| S0.1 | 加仓库根 `.gitignore`（P4-6 的内容） | `.gitignore` | `git status` 只剩有意义的改动 |
| S0.2 | 确认 `Source/build/` 不会被提交；检查 `git log` 是否已有大文件（`git count-objects -vH`） | — | 记录仓库体积 |
| S0.3 | 修 `ThirdParty/CMakeLists.txt:10` 的 `include()` | 1 行 | **删掉 `Source/build/` 后重新 configure 能成功**（关键验收：证明不是靠脏缓存） |
| S0.4 | 修 `new QuadVertex[4]` → `new CircleVertex[4]`（或直接按 S1 重写） | `Renderer2D.cpp:197` | 编译通过 |
| S0.5 | 补 `#include <algorithm>`（`RendererSystem.cpp`），排查同类隐式包含 | 多处 | 编译通过 |
| S0.6 | 加 `BAMBOO_WARNINGS` 选项：MSVC `/W4`（先不加 `/WX`） | 3 个 CMakeLists | 记录警告条数作为基线（预期 >50） |
| S0.7 | 记录当前警告清单，按文件分组存进 issue/文档 | — | 有可对比的基线 |
| S0.8 | 提交（不要带 `build/`） | — | `git status` 干净 |

**出口条件**：从**全新 clone** 到一个新目录，能一条命令 configure + build 成功。这一步会暴露所有"靠脏缓存活着"的问题。

---

### S1 — 渲染主链路重写（3~5 天，本计划最高优先级）

**目标**：`Renderer2D` 的缓冲/批次契约正确，坐标系单一来源，`/W4` 下渲染相关无警告。

| # | 任务 | 关键点 |
|---|---|---|
| S1.1 | **重写 `Renderer2D` 内部数据布局**：把 4 种图元的 `{Buffer, VertexArray, Shader, *Vertices, *VerticesPtr, Capacity, IndexCount}` 抽成可复用结构 | 消灭 24 个散落成员（§3.7） |
| S1.2 | **修正缓冲尺寸**：按 `MaxCount` 一次性分配；`VertexBuffer` 记录 `m_Size`；`SetData` 断言 `size <= m_Size` 并用 `glBufferSubData`（P0-3、P3-1#2） | `OpenGLBuffer.{h,cpp}` |
| S1.3 | **修正索引缓冲**：Quad 索引按 `MaxQuads` 预生成完整表并上传正确 count（P0-2） | 照抄 Sprite 分支的正确写法 |
| S1.4 | **加边界检查与自动换批**：顶点数达上限、纹理槽达上限统一走 `NextBatch()`；`MaxSpriteCount` 可配置（P0-4） | |
| S1.5 | **`DrawQuad`/`DrawTriangle` 尊重参数**：统一通过模型矩阵变换 ±0.5 顶点；删除重复重载（P1-7、P1-8） | 世界单位 = 1 像素 |
| S1.6 | **消除硬编码分辨率**：删掉 `DrawTriangle` 里的 `/1280`、`/720`；UV 与 Y 翻转策略二选一（P1-9） | 全仓库 grep `1280\|720\|640\|360` 应只剩窗口默认尺寸与相机初始化 |
| S1.7 | **`Flush` 只读注册表**：`RendererSystem` 不再写 `SpriteTexture`；缺纹理在 `DrawSprite` 内回落（P0-8） | 渲染期 `const registry&`（若 S3 已做）或至少注释契约 |
| S1.8 | **纹理槽管理**：`TextureSlotMap` 用小数组或 `unordered_map<uint32_t,uint32_t>`；`Flush` 先快照再绑定（P0-8、P3-1#3） | 槽位索引改整型 |
| S1.9 | **`Renderer2D::Shutdown` 补全**：加 `CircleVertices`，或改为 `std::vector` 后整体删除 Shutdown | §3.5 |
| S1.10 | **修 `BAMBOO_ASSERT`**：失败即断（Debug `__debugbreak` / Release `abort`），格式串正确（P0-5） | `Core/Assert.h`，然后**把断言接回所有边界检查** |
| S1.11 | **修 `Camera`**：投影参数唯一来源、不再覆盖 size、除零修正、删死字段（P1-6） | `Graphics/Camera.{h,cpp}` |
| S1.12 | **修 `Matrix3` 行主序一致性 + `Matrix4::RotateXYZ` 角度转弧度**（P1-1、P1-2） | 补单元测试或写临时验证程序 |
| S1.13 | **修 `AABB::Merge` + 成员初始化**（P1-3） | |
| S1.14 | **修 `Color` 三套语义**（P1-4） | 按 D2/§8 决策统一 |
| S1.15 | **修 `Vector2::Cross` 语义 + 除零一致性**（P1-5） | 检查所有 `Cross` 调用点 |
| S1.16 | **修 `Renderer::OnWindowResize`**：真正更新视口 + 相机宽高比（P2-9） | 窗口缩放测试 |
| S1.17 | **加 GL 调试回调**（Debug 构建）：`glDebugMessageCallback` + `GL_DEBUG_OUTPUT_SYNCHRONOUS` | 让后续所有 GL 错误立刻可见（S1 的"保险"） |
| S1.18 | **渲染统计接入**：`Statistics` 扩展为 `{DrawCalls, QuadCount, SpriteCount, VertexCount, BatchCount}` + `GetStats()`；在窗口标题或 Debug 叠层显示 | 为"前后对比"提供数据 |

**出口条件**：
- Breakout 在新分辨率（1920×1080、800×600）下画面正确（砖块不偏、不变形）。
- Debug 构建下 GL 调试回调**无任何错误输出**。
- 1000 个精灵实体不崩溃、DrawCall 数合理。
- 渲染相关文件在 `/W4` 下无警告。

---

### S2 — 应用骨架与生命周期（2~3 天）

**目标**：初始化顺序显式化，帧循环顺序正确，`dt` 受控，窗口/输入可靠。

| # | 任务 | 关键点 |
|---|---|---|
| S2.1 | `Application` 析构改 `virtual`；`s_Instance` 析构时置 `nullptr`（P2-11） | |
| S2.2 | 引入显式初始化阶段表：`Log → Window → Context → RendererAPI::Init → Renderer2D::Init → AssetManager → SceneManager`（P2-10） | `RendererCommand::s_RendererAPI` 延迟构造 |
| S2.3 | 帧循环顺序：`事件轮询 → Time::Update → 物理/逻辑 → 渲染 → SwapBuffers`（P2-11） | 明确"输入在同一帧内可见" |
| S2.4 | `dt` 钳制（上限如 `1/30`）+ 可选固定步长累加器 | 物理稳定性的前置条件 |
| S2.5 | `Time` 命名统一（`deltaTime` → `s_DeltaTime`）、暴露 `GetUnscaledDeltaTime`/时间缩放 | |
| S2.6 | `Window` 初始化：设定 GL 版本/Profile/`glfwSwapInterval`、窗口图标、每步失败**让上层知道**（P3-2） | `WindowsWindow.cpp` 失败路径补日志+返回值 |
| S2.7 | `glfwTerminate` 只在 `glfwInit` 成功后调用；多窗口引用计数（P2-11） | |
| S2.8 | `Log`：初始化 client logger（或删掉这套设计）；级别可配置（Release 默认 `info`）；可选文件 sink（P2-10、§4.8） | |
| S2.9 | `Input`：补全 `KeyCodes`；增加 `IsKeyDown`/`IsKeyPressedOnce`/`IsKeyReleased`；鼠标按键与滚轮；`nullptr` 防护（§4.6） | 与 `GLFW_REPEAT` 联动 |
| S2.10 | `WindowsWindow` 回调补全：`GLFW_REPEAT` → `KeyPressedEvent(key, true)`；char 回调 → `KeyTypedEvent`；鼠标按键 → `MouseButtonEvent`（§4.6） | |
| S2.11 | `Event`：`Dispatch` 检查 `Handled`；加 `IsInCategory` 辅助（P2-11 关联） | |
| S2.12 | `ApplicationResizeEvent` 成员初始化顺序与声明一致（`-Wreorder`）（P2-11） | |

**出口条件**：窗口缩放画面正确；键盘/鼠标事件全类型可达；连续断点调试 10 次不出现"逻辑瞬移"。

---

### S3 — ECS 与场景契约（3~4 天）

**目标**：系统有阶段与所有权，实体句柄安全，场景不自己造内容。

| # | 任务 | 关键点 |
|---|---|---|
| S3.1 | `Entity` 改用 `entt::handle`（或 `m_Scene` 判空 + `IsValid()`），组件操作前置检查（P0-6） | `Entity.h` |
| S3.2 | 消除 `Entity.h` ↔ `Scene.h` 循环包含：`m_Registry` 私有化，`Entity` 只依赖 `entt::registry*`（P2-1） | |
| S3.3 | `ISystem` 契约升级：`GetName/GetPhase/GetPriority/OnInit/OnShutdown/OnUpdate`；`Scene` 负责 `OnInit` 与阶段排序（§3.3、P2-2、P0-9 的 `PhysicsSystem::Init`） | |
| S3.4 | 渲染系统走只读接口 `OnRender(const Scene&, Renderer2D&)`（§3.3、P0-8） | 用 `const` 强制分层 |
| S3.5 | `Scene` 不再在构造函数里创建主相机；提供 `CreateDefaultScene()` 工厂或让序列化器负责（P2-10#3） | |
| S3.6 | `Scene::GetMainCamera` 改为按 `Primary` 的唯一判据 + `std::optional<Entity>`；与 `CameraSystem`/`RendererSystem` 统一（P0-7、P1-13） | |
| S3.7 | `DestroyEntity` 先取 UUID 再 destroy；注册 `registry.on_destroy` 清理 `m_EntityMap`；`m_EntityMap` 改存 `entt::entity`；UUID 冲突检测（P2-12） | |
| S3.8 | `TransformSystem`：恢复 `Dirty` 语义；明确"当前无父子层级"并重命名 `WorldMatrix`→`LocalToWorld`，或真正实现层级（P1-12） | **建议本轮先做前者**，层级放 S7 |
| S3.9 | 全部组件成员加**类内初始化器**；`RigidbodyComponent` 补 `Velocity`；统一摩擦力的归属（P0-9、§3.4） | |
| S3.10 | 清理无效字段：`SpriteRendererComponent::Visible/Opacity` 要么让渲染器读、要么删；`TriangleComponent::Size`/`QuadComponent::Size` 同理（§3.4） | |
| S3.11 | `Component.h` 补全所有组件头（含 collider/rigidbody/shape），并明确"这是组件总入口" | `Component.h:6-12` 缺 5 个 |
| S3.12 | 用脚本统一头文件包含风格（P4-3） | 建议 `#include "Bamboo/..."`，`target_include_directories` 加 `Source/` |

**出口条件**：`Entity` 的所有非法用法在 Debug 下断言、Release 下不崩；系统执行顺序可由数据配置；Breakout 用新的系统注册方式跑通。

---

### S4 — 物理与碰撞（4~6 天，技术含量最高）

**目标**：基于"1 世界单位 = 1 像素"的 2D 物理，Breakout 可用物理驱动（球会弹、砖块会被打掉）。

| # | 任务 | 关键点 |
|---|---|---|
| S4.1 | **定义单位与配置**：像素单位、重力 `-980 px/s²`、固定步长（如 1/120 累加器） | 写进 `PhysicsDefine.h` |
| S4.2 | **纯数学物理层**（不依赖 ECS/GL，可单测）：`Rigidbody { pos, vel, acc, mass, invMass, restitution, friction, type }`；半隐式欧拉积分 | 新目录 `Physics/Core/` |
| S4.3 | **碰撞形状**：`AABB` 与 `Circle` 的相交/接触点/穿透深度 | 复用并修好 `Math/AABB`、`Math/Vector2` |
| S4.4 | **宽相**：均匀网格（`SpatialHash`）；**删除 `Octree.h`** 或改名实现 | 1000 物体下要有性能数字 |
| S4.5 | **窄相**：AABB-AABB → Circle-Circle → Circle-AABB → OBB(SAT)（按需） | |
| S4.6 | **响应**：位置修正 + 速度反射（恢复系数）+ 摩擦 | 先"能弹对"，再"弹得准" |
| S4.7 | **接入 ECS**：`RigidbodyComponent` 补 `Velocity`；碰撞体组件与 `PhysicsSystem::OnInit/OnUpdate` 真正实现 | 注意 `PhysicsWorld` 的所有权 |
| S4.8 | **触发器与碰撞层**：`IsTrigger`、`LayerMask`、`onCollisionEnter/Stay/Exit` 回调（事件队列） | |
| S4.9 | **物理调试绘制**：碰撞体线框 + 速度矢量叠加 | 验证物理正确性的关键工具 |
| S4.10 | **性能验收**：1000 动态物体 + 静止墙，测步进耗时；对比暴力 O(n²) 与网格 | 有数字才叫完成 |

**出口条件**：Breakout 用物理驱动（球撞砖块反弹、砖块消失、球拍反弹），50~1000 物体下 60FPS。

---

### S5 — 数据驱动：序列化与资源（3~5 天）

**目标**：场景可存可读，资源引用不依赖绝对路径。

| # | 任务 | 关键点 |
|---|---|---|
| S5.1 | **组件注册表**：`ComponentRegistry::Register<T>(name, ser, deser)`；用宏减少样板 | 序列化的前置 |
| S5.2 | **场景格式**：JSON（决策见 D2）；含实体 ID/名称/组件/资源引用 | |
| S5.3 | `SceneSerializer::Serialize/Deserialize` 实现；**往返测试** | |
| S5.4 | **资源引用改为句柄**：`SpriteRendererComponent` 存 `AssetHandle`，不再存 `Ref<Texture2D>`（§4.3） | 需要 `AssetManager` 解析 |
| S5.5 | `SceneManager`：场景注册、`LoadScene(name)` 真正加载、多场景切换、`UnloadScene` | |
| S5.6 | **资源路径运行期解析**：环境变量 → 可执行文件相对路径 → 编译期默认值（P3-4） | 可选 `POST_BUILD` 拷贝 `BambooAssets/` |
| S5.7 | `AssetManager`：修 `AsyncLoad` 的 key 与缓存逻辑、去 `detach`、修 `Load` 失败仍缓存、锁一致（P2-8） | |
| S5.8 | `AssetFactory`：支持 Shader/Font/Audio，或明确只支持 Image 并让 `Create` 返回 `std::optional` | |
| S5.9 | `ImageAsset`：修成员初始化与失败状态；支持 1/2 通道；`Unload` 重置 `m_IsLoaded`（P0-9、P3-2） | |
| S5.10 | `FileUtlis` → **`FileUtils`**（改名 + 统一异常策略 + `WriteFile` 签名改 const ref）（P5-1、P3-2） | 标注 `SetCurrentDirectory` 仅启动期可用 |
| S5.11 | `SceneCamera`：删除（空壳且无人用）或实现（P0-12） | 建议删除 |
| S5.12 | `Octree.h`：删除（被 `SpatialHash` 取代） | |

**出口条件**：能保存场景 → 退出 → 重开 → 场景完全一致（同一截图）。

---

### S6 — 工程化：测试 / 警告 / CI（2~3 天，可与 S3~S5 并行）

**目标**：把"能编译"从口头约定变成机器强制。

| # | 任务 | 关键点 |
|---|---|---|
| S6.1 | `Source/Tests/` + GoogleTest（vendored 或 `FetchContent`）；`BAMBOO_BUILD_TESTS` 开关；引擎库**不**依赖 gtest | `roadmap.md` 附录 A 已有示例 |
| S6.2 | 测试覆盖（不依赖 GL 的部分优先）：`Math`、`Entity`、`Scene`、`TransformSystem`、**序列化往返**、**物理积分与碰撞** | |
| S6.3 | **为已发现的 bug 补回归测试**：`Matrix3*Vector3`（P1-1）、`RotateXYZ` 角度（P1-2）、`AABB::Merge`（P1-3）、`Color::FromHex`（P1-4）、`Vector2::Cross`（P1-5） | 每个 P1 bug 一条测试，永久防回归 |
| S6.4 | `/W4` 清零 → 加 `/WX`（或至少 CI 上 `/WX`）；可选 clang-tidy | |
| S6.5 | `CMakePresets.json`：`debug`/`release`/`ci` | 替代手敲命令 |
| S6.6 | `project(... VERSION 0.1.0)` + `install()` 规则 | |
| S6.7 | GitHub Actions：Windows runner → configure → build → `ctest` | |
| S6.8 | `Renderer2D` 的"数值测试"：headless 后端或直接断言 CPU 侧顶点缓冲内容 | 让批次逻辑可测 |
| S6.9 | 性能基线脚本：固定场景下记录 `DrawCalls`/帧耗时到 CSV | 供后续对比 |

**出口条件**：`ctest` 全绿；CI 红叉能拦住回归；`/WX` 下构建通过。

---

### S7 — 编辑器与工具（5~8 天，依赖 S5）

**目标**：Editor 基于引擎的 `Application`/`Scene` 运行，能加载/编辑/保存场景。

| # | 任务 | 关键点 |
|---|---|---|
| S7.1 | **重构 `Editor::Application` 继承 `Bamboo::Application`**，删掉自己的 `glfwInit`/`glfwCreateWindow`（§4.5） | 消除"两个 Application" |
| S7.2 | 补 `ImGui_ImplOpenGL3_Init("#version 330")` + `NewFrame/RenderData` + shutdown 链（§4.5） | ImGui 能渲染的前置 |
| S7.3 | 引入 **Layer 概念**（`Layer`/`LayerStack`）：游戏层 + ImGui 层，事件按层传播 | 编辑器架构基础 |
| S7.4 | 视口（渲染到 FBO → ImGui 图像）；处理 DPI/宽高比/点击坐标映射 | 需要 `FrameBuffer` 抽象（当前**没有**） |
| S7.5 | `SceneHierarchyPanel`（实体树 + 增删 + 重命名） | |
| S7.6 | `InspectorPanel`（基于 S5.1 的组件注册表自动生成属性编辑器） | 组件反射的第二份收益 |
| S7.7 | `ContentBrowserPanel`（浏览 `BambooAssets/`、拖拽到场景） | |
| S7.8 | Gizmo（平移/旋转/缩放拖拽） | |
| S7.9 | Undo/Redo（命令模式） | 编辑器的"专业度"分水岭 |
| S7.10 | Editor 的 `VS_DEBUGGER_WORKING_DIRECTORY` 与资源解析 | |

**出口条件**：Editor 中打开场景 → 拖动实体 → 保存 → 重新打开，结果一致。

---

### S8 — 可选/长期（按兴趣与需要）

| 方向 | 内容 | 前置 |
|---|---|---|
| 脚本 | Lua（sol2）先验证设计 → 再考虑 C#（`roadmap.md` 的建议是对的）；绑定层与热重载 | S5 |
| 音频 | `miniaudio` 单头；音效/音乐/混音 | S5 |
| 文本渲染 | 字体图集（stb_truetype）+ SDF；接 `UI/Text` | S1、S5 |
| UI 系统 | 布局（锚点/约束）+ 命中检测 + 与 ECS 打通（或整体重写） | S1 |
| 层级变换 | `Parent`/`Children` + 世界矩阵递归求解 | S3 |
| 多线程 | 任务系统（Job System）；资源加载/物理/AI 分帧 | S5、S6 |
| 其他后端 | 若 D1 选择"诚实降级"，此时重新抽象才有真实需求支撑 | — |
| 打包 | 资源随包、图标、installer | S5 |

---

### 5.1 阶段依赖图

```
S0 止血
 └─> S1 渲染主链路 ──┬─> S3 ECS/场景 ──┬─> S4 物理
                     │                  ├─> S5 数据驱动 ──> S7 编辑器
                     └─> S2 应用骨架 ───┘                        │
 S6 工程化（可与 S3~S5 并行，但 S6.1/S6.3 建议紧跟 S1）           └─> S8 脚本/音频/文本
```

### 5.2 建议的迭代节奏（每个迭代 1~3 天，可交付）

| 迭代 | 内容 | 交付物 |
|---|---|---|
| I1 | S0 全部 | **全新 clone 能一条命令构建成功** |
| I2 | S1.1~S1.6 | 渲染正确、分辨率无关；GL 调试回调零错误 |
| I3 | S1.7~S1.18 + S6.1~S6.3 | 批次正确 + 断言生效 + 第一批单元测试（含 P1 bug 回归） |
| I4 | S2 全部 | 初始化/帧循环/输入/事件可靠 |
| I5 | S3.1~S3.7 | ECS 安全 + 系统阶段化 |
| I6 | S3.8~S3.12 + S6.4~S6.5 | 组件契约清理 + `/W4` 清零 + presets |
| I7~I8 | S4 | Breakout 物理驱动 + 性能数字 |
| I9~I10 | S5 | 场景存读 + 资源路径 |
| I11~I13 | S7 | 可用编辑器 |

---

## 6. 目标架构

### 6.1 分层（自下而上）

```
┌──────────────────────────────────────────────────────────────┐
│  Game / Editor / Sandbox          应用层                     │
│    Application · LayerStack · EditorPanels · GameSystems     │
├──────────────────────────────────────────────────────────────┤
│  Scene                             场景与数据层              │
│    Scene · Entity · ComponentRegistry · SceneSerializer      │
│    SceneManager · Prefab                                     │
├──────────────────────────────────────────────────────────────┤
│  ECS                               逻辑层                    │
│    ISystem(阶段/优先级) · IRenderSystem(只读) · 内置系统       │
├──────────────────────────────────────────────────────────────┤
│  Systems                           领域层                    │
│    Physics(积分/宽窄相/响应) · Transform(层级) · Camera        │
│    Sprite/Text/UI Renderer                                   │
├──────────────────────────────────────────────────────────────┤
│  Asset                             资源层                    │
│    AssetManager · AssetHandle · AssetLoader · 路径解析         │
├──────────────────────────────────────────────────────────────┤
│  Render                            渲染层                    │
│    Renderer2D(批次/统计) · RendererCommand · Camera(数学)     │
│    Shader/Texture/VertexArray/Buffer/UniformBuffer 接口      │
├──────────────────────────────────────────────────────────────┤
│  RHI (Render Hardware Interface)   后端层                    │
│    OpenGL 实现（唯一的当前实现）                              │
├──────────────────────────────────────────────────────────────┤
│  Platform                          平台层                    │
│    Window · Input · FileSystem · Time                        │
├──────────────────────────────────────────────────────────────┤
│  Core                              基础层                    │
│    Math · Log · Assert · Ref · UUID · Event · Random          │
└──────────────────────────────────────────────────────────────┘
```

**依赖规则（可直接写进 `AGENTS.md`，并用脚本检查）**：

1. **只能向下依赖，不能反向**。特别地：`Render`/`Systems` 层**不得** `#include "Game/..."`（修掉 P2-6）。
2. **`Core` 不依赖任何其他层**（不依赖 entt/GLFW/GL）。
3. **`Render` 不依赖 `Asset`**（它只接受已解析的资源），`Asset` 可以依赖 `Render`（加载 GPU 资源）。
4. **组件（数据）不包含行为**；组件不 `#include` 系统。
5. 每个目录一个 `CMakeLists.txt` 或一个 `target`，用 `target_link_libraries` 表达依赖 → **编译期强制分层**。

> 落地建议：短期不必拆成多个 CMake target（成本高），但**必须**用一条 CI 脚本 grep 禁止的 include 模式（如"`Render/` 下出现 `Game/`"）。这是最低成本的架构守护。

### 6.2 目录结构（建议）

```
Source/
├── CMakeLists.txt              # project()/options/presets 入口
├── CMakePresets.json
├── Bamboo/                     # 引擎静态库（保持单 target，内部按目录分层）
│   ├── Core/                   # 无依赖基础
│   │   ├── Math/               # Vector/Matrix/Color/AABB/Rect/Random/Quaternion
│   │   ├── Log.h / Log.cpp
│   │   ├── Assert.h
│   │   ├── Ref.h / Base.h / UUID.h
│   │   ├── Event/
│   │   └── FileSystem/FileUtils.{h,cpp}
│   ├── Platform/
│   │   ├── Platform.h
│   │   └── Windows/{WindowsWindow,WindowsInput}
│   ├── Render/
│   │   ├── Renderer2D.{h,cpp}          # 批次与统计
│   │   ├── RendererCommand.h
│   │   ├── Camera.{h,cpp}              # 纯数学
│   │   ├── Shader/Texture/VertexArray/Buffer/UniformBuffer 接口
│   │   └── OpenGL/                     # 后端实现
│   ├── Asset/
│   │   ├── Asset.h / AssetHandle.h
│   │   ├── AssetManager.{h,cpp}
│   │   ├── AssetFactory.{h,cpp}
│   │   └── Loaders/{ImageAsset,ShaderAsset,...}
│   ├── ECS/
│   │   ├── Entity.h                    # entt::handle 包装
│   │   ├── Component/                  # 纯数据
│   │   └── System/{ISystem.h,IRenderSystem.h}
│   ├── Scene/
│   │   ├── Scene.{h,cpp}
│   │   ├── SceneManager.{h,cpp}
│   │   ├── SceneSerializer.{h,cpp}
│   │   └── ComponentRegistry.{h,cpp}
│   ├── Physics/
│   │   ├── PhysicsDefine.h
│   │   ├── Core/{Rigidbody,Collision,Integrator,SpatialHash}
│   │   ├── PhysicsWorld.{h,cpp}
│   │   └── PhysicsSystem.{h,cpp}
│   ├── Systems/                        # 引擎内置系统
│   │   ├── TransformSystem.{h,cpp}
│   │   ├── CameraSystem.{h,cpp}
│   │   └── RendererSystem.{h,cpp}
│   └── Game/
│       ├── Application.{h,cpp}
│       ├── Layer.h / LayerStack.h
│       └── Window.h
├── Editor/                     # 编辑器（依赖 Bamboo）
├── Sandbox/                    # 示例（依赖 Bamboo）
├── Tests/                      # gtest（只依赖 Bamboo，优先不依赖 GL）
├── BambooAssets/               # 资源（可选：构建后拷贝到 bin/）
└── ThirdParty/
```

**注意**：目录重命名（`Graphics`→`Render`、`GraphicsAPI`→`Render/OpenGL`）会产生大量 include 改动，建议**与 S3.12 的包含风格统一合并成一次机械替换**，在独立提交里完成（脚本 + 编译验证）。

### 6.3 关键接口草案

```cpp
// ---------- Core/Assert.h ----------
#define BAMBOO_ASSERT(cond, ...)                                               \
    do {                                                                       \
        if (!(cond)) {                                                         \
            BAMBOO_CORE_ERROR("Assertion failed: {} | {}:{}",                  \
                              BAMBOO_STRINGIFY_MACRO(cond), __FILE__, __LINE__);\
            BAMBOO_CORE_ERROR(__VA_ARGS__);                                    \
            BAMBOO_DEBUG_BREAK();  /* Debug: __debugbreak(); Release: abort() */\
        }                                                                      \
    } while (0)

// ---------- Render/Camera.h ----------
class Camera {
public:
    void SetViewportSize(uint32_t w, uint32_t h);   // 不除零，参数变了惰性重算
    void SetWorldHeight(float height);              // 唯一的"缩放"来源
    void SetPosition(const Vector3& p);
    void SetRotationZ(float radians);
    const Matrix4& GetProjection() const;           // 惰性重算
    const Matrix4& GetView() const;
    Matrix4 GetViewProjection() const;
    Vector3 ScreenToWorld(const Vector2& pixel) const;
    Vector2 WorldToScreen(const Vector3& world) const;
private:
    bool m_ProjectionDirty = true, m_ViewDirty = true;
    float m_WorldHeight = 720.0f;                   // 世界视野高度（像素）
    float m_AspectRatio = 16.0f / 9.0f;
    Vector3 m_Position{};
    float m_RotationZ = 0.0f;
    float m_Near = -1.0f, m_Far = 1.0f;
    mutable Matrix4 m_Projection, m_View;
};

// ---------- Render/Renderer2D.h ----------
struct Renderer2DStats {
    uint32_t DrawCalls = 0, Batches = 0;
    uint32_t Quads = 0, Triangles = 0, Circles = 0, Sprites = 0;
    uint32_t Vertices = 0, Indices = 0;
    uint32_t TextureBindings = 0;
};
class Renderer2D {
public:
    static void Init(const Renderer2DSpec& spec);   // 容量可配置
    static void Shutdown();
    static Renderer2DStats GetStats();
    // 只接受"世界变换矩阵"，位置/缩放/旋转由调用方（TransformSystem）给出
    static void DrawQuad  (const Matrix4& transform, const Color& color);
    static void DrawCircle(const Matrix4& transform, const Color& color, float thickness = 0.0f);
    static void DrawSprite(const Matrix4& transform, const Color& color, const Ref<Texture2D>& tex);
    // 用法作用域 RAII，避免 BeginScene/EndScene 手工配对错误
    class Scope { public: explicit Scope(const Camera&); ~Scope(); };
};

// ---------- ECS/System/ISystem.h ----------
enum class SystemPhase { PreUpdate, Update, PostUpdate, PreRender, Render };
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual std::string_view GetName() const = 0;
    virtual SystemPhase GetPhase() const { return SystemPhase::Update; }
    virtual int  GetPriority() const { return 0; }
    virtual void OnInit(Scene&) {}
    virtual void OnShutdown(Scene&) {}
    virtual void OnUpdate(Scene&, float dt) = 0;
};
class IRenderSystem {
public:
    virtual ~IRenderSystem() = default;
    virtual void OnRender(const Scene&, Renderer2D&, const Camera&) = 0;  // const Scene& → 只读
};

// ---------- Scene/ComponentRegistry.h ----------
class ComponentRegistry {
public:
    using SerializeFn   = std::function<void(JsonWriter&, const entt::registry&, entt::entity)>;
    using DeserializeFn = std::function<void(const JsonValue&, entt::registry&, entt::entity)>;

    template <typename T>
    static void Register(std::string_view name, SerializeFn ser, DeserializeFn deser);

    static const ComponentMeta* Find(std::string_view name);
    static void ForEach(const std::function<void(const ComponentMeta&)>& fn);  // 供 Inspector 用
};

// ---------- Asset/AssetHandle.h ----------
class AssetHandle {
public:
    AssetHandle() = default;
    explicit AssetHandle(UUID id) : m_Id(id) {}
    UUID GetId() const { return m_Id; }
    bool IsValid() const { return m_Id != UUID(0); }
private:
    UUID m_Id{};
};
// 组件里存 AssetHandle（可序列化），AssetManager 负责解析为 Ref<Texture2D>
struct SpriteRendererComponent {
    Color       Color    = Color::White;
    AssetHandle Texture{};          // 不再是 Ref<Texture2D>
    Vector2     Size     = {100.0f, 100.0f};
    int         ZOrder   = 0;
    float       Opacity  = 1.0f;
    bool        Visible  = true;    // 渲染器必须真的读它
};
```

### 6.4 架构守护（低成本落地）

在 CI 里加一段脚本，检查禁止的依赖：

```powershell
# tools/check_layering.ps1（示意）
$rules = @(
    @{ Dir = 'Bamboo/Render'; Forbid = '#include\s+"[^"]*Game/'     ; Msg = 'Render 层不得依赖 Game 层' },
    @{ Dir = 'Bamboo/ECS';    Forbid = '#include\s+"[^"]*Platform/' ; Msg = 'ECS 层不得依赖 Platform 层' },
    @{ Dir = 'Bamboo/Core';   Forbid = '(glad|GLFW|entt)'           ; Msg = 'Core 层不得依赖第三方图形/ECS 库' }
)
# ... 扫描并失败退出
```

再加上"必须包含的头"检查（如"任何 `.h` 必须有 `#pragma once`"——当前 `Core/Singleton.h` 和 `Scene/SceneCamera.h` 就没有）。

---

## 7. 每个子系统的验收标准

| 子系统 | 验收方式 | 通过标准 |
|---|---|---|
| **构建** | 全新 clone → 一条命令 configure + build | 零错误；`/W4` 警告数 = 0（S6 之后） |
| **Math** | gtest | 覆盖率 >80%；每个 P1 bug 有回归测试（`Matrix3*Vector3`、`RotateXYZ` 角度、`AABB::Merge`、`Color::FromHex`、`Vector2::Cross`） |
| **渲染批次** | CPU 侧断言顶点/索引缓冲内容（headless 可测）+ GL 调试回调 | 无 GL 错误；1000 精灵下 `DrawCalls` ≤ 10；`MaxCount` 边界处自动换批且画面正确 |
| **坐标系** | 在 800×600 / 1280×720 / 1920×1080 三种分辨率下运行 | 同一场景的相对布局完全一致；`ScreenToWorld(mouse)` 与期望世界坐标误差 < 0.5px |
| **应用/窗口** | 手动 + 自动化 | 缩放窗口画面正确；最小化-恢复不崩；拖窗口 2 秒后逻辑不瞬移 |
| **ECS** | gtest + Debug 断言 | 空 `Entity` 的所有操作不崩；销毁实体后 `m_EntityMap` 无残留；系统执行顺序与配置一致 |
| **物理** | 独立物理测试场景 + 单测 | 自由落体 `v = g*t` 误差 < 1%；球撞墙反弹角正确；1000 物体步进 < 4ms；有暴力法对比数字 |
| **序列化** | 往返测试 | `序列化→反序列化→序列化` 两次输出字节一致；重开后截图一致 |
| **资源** | 移动仓库目录后运行 | 不重新 configure 也能加载资源；加载失败有显式错误提示 |
| **输入** | 手动 + 事件日志 | 所有按键/鼠标事件类型可达；`IsKeyPressedOnce` 每帧最多 true 一次 |
| **编辑器** | 手动端到端 | 打开场景 → 改属性 → 保存 → 重开一致；ImGui 正常渲染；视口缩放正确 |
| **性能** | 固定场景 + CSV 记录 | 记录每次迭代的帧耗时/DrawCall/物理步进，**前后对比有数字** |
| **架构守护** | CI 脚本 | 分层规则零违反；所有 `.h` 有 `#pragma once` |

---

## 8. 需要你决策的事项

这些问题没有唯一正确答案，取决于你的目标。定方向后就能按路线图执行。

| # | 决策点 | 选项 A | 选项 B | 建议 |
|---|---|---|---|---|
| D1 | **图形抽象层怎么处理**（§3.6） | **诚实降级**：合并 `GraphicsAPI/OpenGL` 进 `Graphics`，删掉空转的工厂/switch，代码量 −30%，改动路径清晰 | **兑现抽象**：所有 `Create` 走 `GetAPI()`，补齐 `SetInt` 等，加 Null 后端用于测试 | **A**。当前没有第二个后端的真实需求，抽象成本 > 收益。将来要换后端时再抽，那时需求清晰 |
| D2 | **序列化格式**（S5.2） | **自写极简 JSON**（约 600 行，完全掌控，符合"学 C++、少依赖"基调） | **vendored `nlohmann/json`** 单头（成熟、省时间、社区标准） | **B**。序列化不是学习重点，且自写 JSON 的边界情况（转义/精度/UTF-8）会吃掉大量时间 |
| D3 | **物理自己写还是接库**（S4） | **自己写**（`roadmap.md` 的定位就是学透物理；2D 刚体 + AABB/圆足够） | 接 **Box2D**（工业级、省时间，但把最有学习价值的部分外包） | **A**（与 roadmap 一致）。若目标是"尽快做出游戏"，则 B |
| D4 | **空间分区用哪个**（S4.4） | **均匀网格 / SpatialHash**（2D 简单高效，几十行） | **四叉树/八叉树**（`Octree.h` 已有占位，但 2D 收益不如网格） | **A**。并删除 `Octree.h` |
| D5 | **UI 系统怎么处理**（P2-7） | **并入 ECS**：`UIElementComponent` + `UISystem` 走同一渲染路径，删除自建组件系统 | **保留独立体系**，补齐按钮/文本/布局/命中检测 | 短期**先移出编译**（`BAMBOO_BUILD_UI` 开关），中期按 **A** 重做 |
| D6 | **`Editor` 是否继续投入** | **先做游戏侧**（物理/序列化/渲染），编辑器等引擎稳定后再做 | **编辑器优先**（数据驱动能让后续迭代快很多） | 近期目标是"做出能玩的 2D 游戏" → **先游戏侧**；目标是"理解引擎架构" → **编辑器优先**（它会强迫你把序列化/反射/分层做对） |
| D7 | **是否升到 C++20/23** | 保持 **C++17** | 升 **C++20**（`concepts`/`span`/`<format>`/指定初始化器；MSVC 支持良好） | **C++20**。你已在用 `std::is_base_of_v`、`std::optional`、结构化绑定；`concepts`（约束模板组件）、`std::span`（缓冲视图）、指定初始化器（组件初始化）收益明显，成本只是改一行 |
| D8 | **是否引入第三方大件** | 纯自研 | 引入 glm（数学）/Box2D（物理）/nlohmann（JSON） | **按需**：`glm` 可考虑（你的 `Math` 有 P1 级 bug，修完再决定是否替换）；其余按 D2/D3 |
| D9 | **多线程资源加载** | 保留并修好 `AsyncLoad` | 暂时移除异步接口，先做对同步加载 | **B**（短期）。异步的复杂度现在换不来收益，且当前实现是错的 |
| D10 | **`Sandbox` 的定位** | 保留 Breakout 作为**回归验证用例**（每阶段必须跑通） | 另建"引擎特性演示场"（物理/渲染/UI 分场景） | **两者都要**：Breakout 作为 S1/S4 验收，另加 `Sandbox/Scenes/*.json` 作为后续演示 |

---

## 9. 立即可做的第一步

**今天就能做完、且立刻降低风险的三件事**（按顺序）：

1. **加 `.gitignore`（P4-6）** —— 5 分钟。在 `git add -A` 误提交 40MB 构建产物之前。
2. **修 `ThirdParty/CMakeLists.txt:10` 的 `include()`，然后删掉 `Source/build/` 从零 configure 一次** —— 20 分钟。这一步会**一次性证明**你的工程是否真的能重建，而不是靠脏缓存活着。**如果这一步就失败，说明后面的所有计划都要先让位于"构建可靠性"。**
3. **修 `Renderer2D.cpp:197` 的 `QuadVertex` → `CircleVertex`，让工作区重新编译通过并提交** —— 15 分钟。**别在编译不过的工作区上继续开发。**

做完这三件事，再按 S1 开始渲染主链路重写。

**关于 `docs/` 里的文档**：建议在 `roadmap.md` 和本文件开头互相加一行链接，明确分工：

- `roadmap.md` = **学习路线**（每个 bug 对应什么 C++ 知识点、GitHub 工作流）
- `refactor_plan.md` = **技术重构路线**（改什么、为什么、怎么验收）
- `architecture.md` = **现状架构**（对照代码核对过的分层/执行顺序/不变量）
- ~~`project_structure.md`~~ = **已于 2026-09-18 删除**（它描述的是不存在的代码；有价值的结构信息已并入 `architecture.md`，作废清单见该文档 §9）

---

## 附录 A：命名与拼写统一对照表

| 现状 | 建议 | 位置 | 优先级 |
|---|---|---|---|
| `ShaderDatatType` | `ShaderDataType` | `Graphics/RenderBuffer.h:9` | 高（公开 API） |
| `FileUtlis` | `FileUtils` | `Core/FileSystem/FileUtlis.{h,cpp}` | 高（文件名 + 类名） |
| `IsDirector` | `IsDirectory` | `FileUtlis.h:21` | 高 |
| `Shoudown` | `Shutdown` | `Editor/Source/Core/Application.{h,cpp}` | 高 |
| `GetNormalTexture` | `GetWhiteTexture` | `Renderer2D.{h,cpp}` | 高（语义错，它不是法线贴图） |
| `vertexPaht` / `fragmentPaht` | `vertexPath` / `fragmentPath` | `Graphics/Shader.h:37` | 高 |
| `Vector2Int`（不存在的类） | `IntVector2`（已存在） | `Math/Rect.h:68,77` | 高（当前是"引用了不存在的类型"） |
| `B_HALF_PI = 1.41592653589793238` | `constexpr float kHalfPi = 1.5707963267948966f` | `Math/MathDefs.h:5` | 高（**数值错误**） |
| `B_PI`/`B_TWO_PI`/`B_EPSILON` 宏 | `constexpr float kPi/kTwoPi/kEpsilon` | `Math/MathDefs.h` | 中（宏污染） |
| `BIT(x)` 宏 | `constexpr` 函数/模板 + `#undef` | `Core/Base.h:8` | 中 |
| `m_data`(Matrix4/Matrix3) vs `m_Min`(AABB) | 统一 `m_` + PascalCase（或全 camelCase） | 多处 | 低 |
| `m_orthographic`/`m_Orthographic` | 统一 `m_` 前缀 | `Camera.h:59` | 低 |
| `deltaTime`（无前缀） | `s_DeltaTime` | `Core/Time.h:17` | 低 |
| `Core/` vs `core/` | 统一 `Core/` | `Assets/AssetFactory.h:5`（**小写，Windows 侥幸通过**） | 高（跨平台必炸） |
| `CmakeLists.txt` | `CMakeLists.txt` | `ThirdParty/` | 低（Windows 可容忍） |
| `Graphics/` + `GraphicsAPI/` | `Render/` + `Render/OpenGL/` | 目录名 | 低（大改动，放 S3.12） |
| `ECS/Component/Shape/` 与 `ECS/Component/` 混放 | 组件统一一层，或明确 `Shape/` 的含义 | `ECS/Component/` | 低 |
| `Sandbox/BreakoutDemo/` 与 `Sandbox/` 混放 | 示例统一进 `Sandbox/Demos/` | `Sandbox/` | 低 |

---

## 附录 B：本次评审涉及的 C++ 知识点索引

（延续 `roadmap.md` 的"每个 bug 对应一个知识点"精神。标 ★ 的是**本次评审中新发现的、`roadmap.md` 未覆盖**的。）

| 知识点 | 对应问题 |
|---|---|
| 未定义行为（UB）与类型双关 | P0-1（`QuadVertex*`→`CircleVertex*`）、P0-2（未初始化索引） |
| 缓冲区溢出与防御式编程 | P0-3、P0-4 |
| 断言的语义（只在 Debug 生效、失败必须停止） | P0-5 ★（`roadmap.md` 只提到"条件写反"，未发现"不中断"这个更严重的语义问题） |
| 空指针解引用与不变量（invariant） | P0-6、P0-7 |
| 悬垂指针 / 迭代器失效 | P0-7、P0-8 ★ |
| 类内成员初始化器（NSDMI）与"默认构造即有效状态" | P0-9 ★ |
| 静态初始化顺序问题（SIOF） | P2-10 ★（`roadmap.md` 未覆盖） |
| 虚析构函数与多态基类 | P2-11 ★ |
| 编译期契约优于运行期约定 | §3.1 ★ |
| `const` 正确性（`const&` 参数、`const registry&`） | P1-9、§3.3 ★ |
| 行主序 vs 列主序（矩阵存储约定） | P1-1 ★（最容易出错的数学 bug） |
| 角度 vs 弧度 | P1-2 ★ |
| 除零 / inf / NaN 传播 | P1-5、P1-6 ★ |
| 重载决议与隐式转换（`Color` 的 int/float 重载） | P1-4 ★ |
| ODR 与单一定义规则 | `roadmap.md` §0.2 提到 `STB_IMAGE_IMPLEMENTATION`——**本次核查发现该问题在当前代码中已修复**（只有 `OpenGLTexture.cpp` 定义），但 `#pragma once` 出现在 `.cpp` 里仍是坏味道 |
| 头文件包含与循环依赖、前向声明的适用边界 | P2-1 ★ |
| 隐藏依赖（传递包含） | P0-11 ★ |
| 所有权与 RAII（`unique_ptr`/`shared_ptr`/裸 `new[]` 的取舍） | §3.5 ★ |
| 线程安全（`detach` 线程 + 悬垂 `this`） | P2-8 ★ |
| 成员初始化顺序与 `-Wreorder` | P2-11 ★ |
| `concepts`、`std::span`、指定初始化器（C++20） | §6.3、D7 ★ |
| 分层架构与依赖倒置 | §3.2~§3.7、§6.1 ★ |
| 数据驱动设计与反射思想 | S5（`roadmap.md` 有，此处细化为 `ComponentRegistry`） |

---

## 附录 C：本次评审未覆盖 / 需要进一步确认的部分

诚实标注哪些结论是有条件的，避免误导。

1. **运行期行为未实测**。本次是**纯静态代码评审**（通读源码 + 构建元数据交叉验证），我没有运行 `Sandbox.exe`，也没有用 RenderDoc / GL 调试器观察实际帧。因此：
   - "画面偶尔看起来正常"这类判断是从代码推断的，需要实测确认。
   - **GL 4.5 的 DSA 函数（`glCreateTextures`/`glTextureStorage2D`/`glNamedBufferData`）在你的驱动上是否真的可用未验证。** `WindowsWindow` 创建窗口前**没有设置 GL 版本/Profile hint**（无 `glfwWindowHint`），依赖 GLFW 给平台默认。**如果驱动给的上下文低于 4.5 且无 `ARB_direct_state_access`，这些函数指针会是 `nullptr` → 调用即崩。** 请在 S0 顺手加 `glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR/MINOR)` + `GLFW_OPENGL_PROFILE`，并在 `OpenGLContext::Initialize` 里打印 `glGetString(GL_VERSION)`。**这一项必须实测。**
2. **`roadmap.md` 与 `project_structure.md` 的行号未逐一核对**。本文件所有行号基于**当前工作区快照**（`HEAD = 761574d` + 未提交的 `Renderer2D.cpp` 改动）。`roadmap.md` 里更早的行号（如 `Renderer2D.cpp:150`）已对不上。
3. **性能结论基于代码模式而非实测数据**。P3-1 的排序是"按经验收益排序"，实际收益需 S1.18 的统计接入后才能量化。
4. **`Source/build/` 里的旧缓存内容未逐项检查**。我只确认了生成器、`Config.h` 内容与产物时间戳。`build/` 里存在 `.vs/` 和 `x64/`，暗示配置被反复带旧状态重跑；**建议 S0.3 直接删掉整个 `build/` 重建**，而不是尝试修缓存。
5. **`ThirdParty/` 各库版本未确认**。`entt`/`spdlog`/`GLFW`/`glad` 的具体版本号未记录（`glfw3.h` 有 `GLFW_VERSION_*` 宏可查）。若后续要升级或改用 `FetchContent`，需要先建版本清单。
6. **`Editor` 的 ImGui 后端结论是确定的**：`ThirdParty/CMakeLists.txt` 编译了 `imgui_impl_opengl3.cpp`，但 `Editor/Source/Core/Application.cpp` **从未调用 `ImGui_ImplOpenGL3_Init`**（只调了 `ImGui_ImplGlfw_InitForOpenGL`），所以 ImGui 渲染不出来。这是"缺失调用"，结论确定。
7. **未检查 `.vs/`、`.vscode/` 里的配置**。`.vscode/c_cpp_properties.json` 存在，若其 include 路径与实际 CMake 不一致，会造成"IDE 不报错但编译失败"（或反之）的困扰。
8. **未检查 `Code-queries-main/`**（Python 代码统计工具），它与引擎无关，建议直接 gitignore 或移出仓库。

---

### 文档维护约定

- 本文件是**活文档**：每完成一项任务就在对应条目后追加 `✅ S1.3 (2026-XX-XX, commit abc1234)`。
- 新增缺陷请追加到第 2 节对应优先级下，**不要插入已有条目中间**（保持编号稳定，便于引用）。
- 每个阶段（S0~S8）完成后，在第 5 节对应小节末尾追加"实际耗时 / 与计划的偏差 / 意外发现"。
- 与本文件冲突时，**以代码为准**；发现冲突请更新本文件而不是默默忽略。
