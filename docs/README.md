# Bamboo 引擎 · 项目文档

> **这是唯一入口文档。** 项目现状、架构问题、要改成什么样、按什么顺序改、用什么验收——全部在这里。
> 唯一的补充材料是 [`research_csharp_embedding.md`](research_csharp_embedding.md)（C# 嵌入技术调研，91 条引用、可复制代码、未验证清单），它太长且是纯技术参考，所以单独存放。
>
> 核对基准：**2026-09-22**｜引擎核心约 4,600 行｜单人开发｜Windows + MSVC + OpenGL

---

## 目录

- [1. 项目是什么 / 现在能做什么](#1-项目是什么--现在能做什么)
- [2. 快速开始](#2-快速开始)
- [3. 三个必须知道的坑](#3-三个必须知道的坑)
- [4. 架构：五大根因](#4-架构五大根因)
- [5. 缺陷清单](#5-缺陷清单)
- [6. 目标架构与迁移路径](#6-目标架构与迁移路径)
- [7. 游戏计划：用游戏验收引擎版本](#7-游戏计划用游戏验收引擎版本)
- [8. 编辑器设计](#8-编辑器设计)
- [9. C# 脚本：关键技术约束](#9-c-脚本关键技术约束)
- [10. 工程化：测试 / 构建 / CI](#10-工程化测试--构建--ci)
- [11. 全局验收标准](#11-全局验收标准)
- [12. 路线图与里程碑](#12-路线图与里程碑)
- [13. 风险登记册](#13-风险登记册)
- [14. 学习地图：每个问题对应什么 C++ / 引擎知识](#14-学习地图每个问题对应什么-c--引擎知识)
- [15. 开发约定](#15-开发约定)
- [16. 待决策事项](#16-待决策事项)

---

## 1. 项目是什么 / 现在能做什么

Bamboo 是一个用 C++ 从零写的 **2D 游戏引擎**（学习与实用双目标），Windows / MSVC / OpenGL。

| | |
|---|---|
| 平台 | Windows only（MSVC / Visual Studio 2022） |
| 语言 | **C++20**（`stdcpp20`，见 `Source/CMakeLists.txt:7`） |
| 图形 | OpenGL（glad 加载） |
| 构建 | CMake 3.20+ |
| 第三方 | 全部 vendored：GLFW / glad / entt / spdlog 1.15.3 / stb / imgui 1.92.4 |
| 代码量 | 引擎核心 ≈ 4,600 行 |

### 1.1 当前能力（诚实版）

| 模块 | 状态 | 说明 |
|---|---|---|
| 窗口 / 上下文 | ✅ 可用 | GLFW + OpenGL；关闭/缩放事件可用 |
| ECS | ✅ 可用 | entt + `Entity` + `SystemRegistry`（四阶段分桶） |
| 渲染（精灵） | ⚠️ 有坑 | 2D 批次，按 ZOrder 排序；缺陷见 §5 |
| 渲染（三角/四边形/圆） | ⚠️ 有缺陷 | 三角/四边形**忽略传入的位置与尺寸**；圆未实现 |
| 相机 | ⚠️ 部分 | 像素相机；`SetOrthographic` 的 size 被覆盖，无法缩放 |
| 输入 | ⚠️ 部分 | 仅 A~Z + 鼠标位置；无按键/滚轮/方向键 |
| 资源加载 | ⚠️ 部分 | 能从 `BambooAssets/` 加载；失败只打日志 |
| 物理 | ❌ 未实现 | `PhysicsSystem::Update` 空函数；碰撞体组件无人读取 |
| 场景序列化 | ❌ 未实现 | `SceneSerializer` 空实现 |
| 场景管理 | ❌ 未实现 | `LoadScene(name)` 忽略参数 |
| 编辑器 | ❌ 空壳 | 能开窗口；**绕过引擎**且 ImGui 未接 GL3 后端，渲染不出内容 |
| UI 系统 | ❌ 空壳 | `Source/Bamboo/UI/` 大部分未实现 |
| 音频 | ❌ 无 | |
| C# 脚本 | ❌ 无 | 计划中，见 §9 |

**结论**：现在只能用 C++ 直接写渲染/ECS 代码跑出一个画面；**做不了**"在编辑器里搭关卡"或"用脚本写逻辑"。

---

## 2. 快速开始

### 2.1 前置要求

- Windows 10/11
- **Visual Studio 2022**（勾选「使用 C++ 的桌面开发」）
- **CMake 3.20+**（VS 自带或单独安装）

### 2.2 命令行构建

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

运行需要在输出目录作为工作目录（原因见 §3.1）：

```bat
cd Source\build\bin\Debug
Sandbox.exe
```

### 2.3 Visual Studio

打开 `Source/`（或 `Source/build/BambooEngine.sln`），选 `Sandbox`，F5。工作目录已由 CMake 自动设置。

### 2.4 VS Code

仓库已含 `.vscode/` 配置，**打开仓库根目录**即可：

- 需要扩展：`ms-vscode.cpptools`、`ms-vscode.cmake-tools`（打开时自动提示）
- `Ctrl+Shift+P` → `CMake: Configure` → `CMake: Build`
- F5 → `Sandbox (Debug)`

> **IntelliSense 报「找不到头文件」** → 见 §3.3。

---

## 3. 三个必须知道的坑

### 3.1 资源路径是编译期烤进去的绝对路径

`Source/Bamboo/Config.in.h` 经 CMake `configure_file` 生成 `Source/build/generated/Config.h`，把 `BAMBOO_ASSET_ROOT` **写死成绝对路径**指向 `Source/BambooAssets/`。

**移动/克隆仓库后必须重新 configure**，否则资源加载失败（且只打一条日志，表现是黑屏或白块）。

### 3.2 新增源文件后必须重新 configure

`Source/Bamboo/CMakeLists.txt` 与 `Sandbox/CMakeLists.txt` 用 `file(GLOB_RECURSE ...)` 收集源文件，**不会**自动感知新文件。加了 `.cpp`/`.h` 后要重跑 `CMake: Configure`。

### 3.3 VS Code IntelliSense 排查（5 步）

`.vscode/c_cpp_properties.json` 已配好 MSVC + 全部第三方 include 根。仍报错时依次检查：

1. `Source/build/generated/Config.h` 是否存在（只在配置成功后生成）
2. `Source/ThirdParty/*/include` 目录是否完整
3. 是否装了 CMake Tools 扩展（配置里已开启 `CMAKE_EXPORT_COMPILE_COMMANDS`）
4. `compilerPath` 里的 MSVC 版本号（当前 `14.44.35207`）是否与你的 VS 一致
5. 打开的是**仓库根目录**（不是 `Source/`）

改完执行 `C/C++: Reset IntelliSense Database`。

---

## 4. 架构：五大根因

> **这一节是全文最重要的部分。** §5 的 37 项缺陷是**症状**，这里才是**根因**。
> 只按 §5 逐条修 bug，架构不动，新代码会继续长出同样的形状——bug 数量随模块数**平方增长**。

一个有 3 个以上模块的系统，必须为下面五件事指定**唯一权威（authority）**。当前引擎五件都缺。

### 4.1 缺「执行顺序」权威

**现状**：`SystemRegistry` 看起来是阶段化的，但阶段集合是**硬编码的封闭枚举**：

```cpp
// ISystem.h:6-12
enum class SystemPhase { Logic, Render, Physics, Transform };

// SystemRegistry.h:13-22 —— 每阶段一个容器 + 一个 Update 函数
void UpdateLogic/UpdatePhysics/UpdateRender/UpdateTransform(...);
std::vector<Scope<ISystem>> m_Logic/m_Physics/m_Render/m_Transform;

// Scene.cpp:39-42 —— 阶段的**先后顺序只存在于这 4 行**
m_SystemRegistry.UpdateLogic(...);
m_SystemRegistry.UpdateTransform(...);
m_SystemRegistry.UpdatePhysics(...);
m_SystemRegistry.UpdateRender(...);
```

**加一个新阶段要改 4 个地方**（枚举 / 容器 / Update 声明 / Update 实现）。引擎骨架每加一个阶段就要被剪开一次。

**更根本的问题**：`CameraSystem` 落在 `Logic` 桶，却**读** `TransformComponent`（算视图矩阵）。它现在正确**纯属巧合**——因为 `Transform` 桶恰好排在 `Logic` 之前。这条依赖关系**没有在任何地方声明**：全仓库搜 `DependsOn`/`After`/`Priority`/拓扑排序，**零结果**。

于是有三重隐式：
1. 阶段间顺序 → `Scene::Update` 的书写顺序
2. 同阶段内顺序 → `Register<T>()` 的调用顺序
3. 跨阶段读写依赖 → **无人记录**，只能靠人记住

**根因**：顺序知识散落在调用点上，而不是集中在注册表里。

### 4.2 缺「对象身份与生命周期」权威

```cpp
// Entity.h:16,74 —— Entity 持有 Scene 裸指针
Entity(entt::entity handle, Scene* scene);
Scene* m_Scene = nullptr;

// Entity.h:23,39,45,52 —— 所有组件操作无条件解引用它
T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, ...);

// Scene.h:48 —— 为了让 Entity 能访问，registry 被迫 public
entt::registry m_Registry;

// Entity.h:3 + Scene.h —— 于是头文件互相包含
#include "../Scene/Scene.h"
```

三个纠缠的问题：
1. **`Entity` 是视图还是所有者？** 未定义。它存 `Scene*` 只是为了访问 `registry`。
2. **"无效实体"没有表示。** 默认构造是 `{entt::null, nullptr}`，但所有方法都假设它有效 → `FindEntityByName` 失败返回的空实体一用就崩。
3. **所有权与销毁责任不明。** `DestroyEntity` 先 destroy 再取 UUID；组件里的 `Ref<Texture2D>` 由 `shared_ptr` 顺手管理，没人知道纹理何时真正释放。

**这就是为什么封装必须是 public**：`Entity` 需要 `registry`，`registry` 属于 `Scene`，两者互不知边界。

### 4.3 缺「资源身份」权威

```cpp
// AssetManager.h:20,89 —— 资源的身份 = 路径字符串
Ref<T> Load(const std::string &path);
std::unordered_map<std::string, Ref<Asset>> m_Assets;

// SpriteRendererComponent.h:11 —— 组件直接持有 GPU 对象
Ref<Texture2D> SpriteTexture;
```

1. **身份 = 路径** → 改文件名即断链，所有引用处都要跟着改。
2. **组件持有 GPU 对象** → **含纹理的场景无法序列化**（这是 `SceneSerializer` 空着的根本原因之一，不只是"没写"）；同时让组件依赖渲染层，分层被打破。
3. **生命周期无人负责** → `shared_ptr` 让"最后一个引用消失"决定释放时机，可能在**渲染进行中**释放；且 `AssetManager::Unload` 与 `shared_ptr` 是**两套互不知情的释放机制**。

### 4.4 缺「数据模式」权威

**问题不在"序列化器没写"，而在于没有任何东西知道"一个组件由哪些字段组成"。**

后果是加法式的痛苦：加一个组件要改 **4 个地方**——序列化器、编辑器检查器、复制粘贴、脚本绑定。每加一个字段，4 处都要改，漏一个就是 bug。

**这才是"编辑器做不动"的真正原因**——不是编辑器难写，是成本随组件数线性增长，而组件数必然增长。

### 4.5 缺「渲染契约」

```cpp
// RendererSystem.cpp:26,45,55,66 —— 渲染系统每帧自己遍历 registry 决定画什么
registry.view<CameraComponent, TransformComponent>();
registry.view<SpriteRendererComponent, TransformComponent>();

// Renderer2D.cpp:51-117 —— 4 种图元 × 6 个字段 = 24 个散落成员
```

1. **"什么该被画"没有单一答案** —— 渲染器自己决定（遍历、排序、补默认纹理）。加图层要改渲染器；编辑器想"只画选中物体"无法表达。
2. **绘制 API 形状不稳定** —— `DrawQuad` 两个重载函数体相同；`DrawTriangle` 忽略参数；`DrawSprite` 用非 const 引用导致无法接临时对象。**4 个函数就已经无法演进了。**
3. **渲染器状态是"全局单例 + 每帧隐式重置"**，没有"这次绘制属于哪个视图/相机"的概念 → 直接挡住多视口（编辑器必需）。

### 4.6 根因 → 症状对照

| 根因 | 典型症状 |
|---|---|
| §4.1 执行顺序 | `CameraSystem` 靠巧合正确；`PhysicsSystem::Init` 从未被调用；"内置/逻辑"两套流水线 |
| §4.2 生命周期 | 空实体一用就崩；相机指针悬垂；`m_Registry` 必须 public；循环包含 |
| §4.3 资源身份 | 改名断链；组件持有 GPU 对象；两套释放机制 |
| §4.4 数据模式 | 序列化器空实现；加组件要改 4 处；字段"声明了没人读" |
| §4.5 渲染契约 | 渲染期回写 ECS；`DrawQuad` 忽略参数；24 个散落成员 |

---

## 5. 缺陷清单

优先级：**P0 阻断**（编译失败/必然 UB/崩溃）｜**P1 正确性**（能跑但结果错）｜**P2 设计**（架构耦合）｜**P3 可维护**｜**P4 工程化**｜**P5 命名**

### P0 阻断级

| # | 位置 | 问题 | 影响 |
|---|---|---|---|
| P0-1 | `Renderer2D.cpp:165-183` | Quad 索引缓冲：生成循环因 `QuadIndexCount == 0` 从未执行，却 `IndexBuffer::Create(quadIndices, 2)` 上传**未初始化数据** | 越界读取顶点缓冲，靠"没人用 Quad"掩盖 |
| P0-2 | `Renderer2D.cpp:126,153,204` | 顶点缓冲按**1 个顶点**大小创建，靠 `SetData` 里 `glBufferData` 重新分配掩盖 | 构造参数是骗人的；每帧重分配显存 |
| P0-3 | `Renderer2D.cpp:435-447` | `DrawSprite` 无 `MaxSpriteCount(100)` 边界检查 | 超过 100 精灵静默写穿堆 |
| P0-4 | `Core/Assert.h:7-14` | `BAMBOO_ASSERT` 失败**只打日志不中断**；格式串把 `bool` 当参数打印 | 等于没有断言 |
| P0-5 | `ECS/Entity.h:19-53` | 默认构造的 `Entity`（`m_Scene == nullptr`）调用任何组件方法都崩 | `FindEntityByName` 失败的返回值直接崩 |
| P0-6 | `Scene/Scene.cpp:87-96` | `GetMainCamera` 返回 registry 内部组件地址，按**名字**查找 | 实体销毁后悬垂；与 `RendererSystem`/`CameraSystem` 三套判据不一致 |
| P0-7 | `RendererSystem.cpp:83-86` | 渲染期**回写** ECS（`sprite->SpriteTexture = ...`） | 破坏渲染只读契约；抹掉"用户没设纹理"这一信息 |
| P0-8 | 多处 | 成员未初始化：`CircleIndexCount`、`RigidbodyComponent` 全部字段、`Camera::m_ProjectionType`、`UIElement` 全部字段、`GLFWwindow* m_Window` | 读未初始化值 |
| P0-9 | `ThirdParty/CMakeLists.txt:10` | `${...}/entt/include()` —— 路径尾部多了**空括号** | 头文件找不到（历史构建靠脏缓存侥幸） |
| P0-10 | `RendererSystem.cpp:78` | 用了 `std::sort` 但未 `#include <algorithm>` | 隐式依赖，换 STL 即失败 |
| P0-11 | `UI/`、`Scene/SceneCamera.h`、`Math/Quaternion.h`、`Math/Rect.h`、`Vector4.cpp` | 声明了但从未定义（`UIElement` 析构、`Canvas`/`Button` 全部、`Quaternion::Identity`、`RectInt` 构造）；`Vector4.cpp` 是 **0 字节**；`Rect.h` 引用了不存在的类 `Vector2Int` | 潜在链接错误 |
| P0-12 | `Renderer2D.cpp:91` | `CircleIndexCount` 无初值，且 `StartBatch` 不重置它 | 圆实现后会读到垃圾 |
| P0-13 | `AI/`（无） | — | — |

### P1 正确性级

| # | 位置 | 问题 |
|---|---|---|
| P1-1 | `Math/Matrix3.h:19` vs `.cpp:22-30,106-192` | **内部布局自相矛盾**：`operator()` 用列主序索引，`operator*` 用行主序；`RotateX/Y/Z`/`Translate` 用列主序。矩阵→向量乘法**等于乘了转置** |
| P1-2 | `Math/Matrix4.cpp:255-283` | `RotateXYZ` 把**角度当弧度**用（项目其他处用 `Math::AngleToRadian`） |
| P1-3 | `Math/AABB.cpp:41-45` | `Merge` 用**刚被改写的** `m_Min` 去算 `m_Max`；`AABB` 成员无初值 |
| P1-4 | `Math/Color.h` | 三套通道语义矛盾：`Color(int a,int r,int g,int b)` 是 **ARGB**、`Color(unsigned)` **不归一化**、`SetColorToHex` 只接受 9 字符（`#RRGGBB` 静默失败） |
| P1-5 | `Math/Vector2.h:40` vs `:47` | 成员 `operator/` **无除零检查**，友元版本**有**；`Cross` 返回 `Vector2` 但 2D 叉乘应是**标量**，且两个分量都算错 |
| P1-6 | `Graphics/Camera.cpp:28-53` | `ReCalculateProjectionMatrix` **覆盖** `SetOrthographic` 传入的 size → **无法缩放**；`SetViewportSize` 的除零守卫"判断了却什么都不做"且应为 `\|\|`；`Perspective` 分支为空 |
| P1-7 | `Renderer2D.cpp:348-364` | `DrawTriangle` 硬编码 `/1280`、`/720`；算出的 `inPos` **从未使用**（position 实际被忽略）；缩放硬编码 0.5 |
| P1-8 | `Renderer2D.cpp:366-386` | 两个 `DrawQuad` 重载函数体**完全相同**，都忽略 `position` 与 `size` |
| P1-9 | `Renderer2D.h:27` | `DrawSprite` 参数是 `Ref<Texture2D>&` **非 const 引用** → 无法传临时对象；且 stb 全局 Y 翻转与 UV 表叠加，图可能上下颠倒 |
| P1-10 | `Renderer2D.cpp:363,374,446` | `*IndexCount` 在三角形分支当**顶点数**、在 Quad/Sprite 分支当**索引数** |
| P1-11 | `ECS/System/TransformSystem.cpp` | `WorldMatrix = LocalMatrix`（注释还写着"世界矩阵=本地矩阵的**逆矩阵**"）；旋转被注释掉；`Dirty` 无条件清零 → 每帧全量重算 |
| P1-12 | `ECS/System/CameraSystem.cpp` | 用"取负角度"求视图矩阵的逆，只对纯平移+Z 旋转侥幸成立；`Matrix4::Inverse()` 已实现却不用 |
| P1-13 | `Vector2/Vector3/IntVector2` | 除零、`Cross` 语义、`Length()` 返回 int 截断等一组数值问题 |

### P2 设计级

| # | 位置 | 问题 |
|---|---|---|
| P2-1 | `ECS/Entity.h` ↔ `Scene/Scene.h` | 循环包含；`m_Registry` 被迫 public；`Scene.h` 对 `Entity` 只能前向声明，任何按值使用即炸 |
| P2-2 | `Scene.h:52-53` + `Scene.cpp:37-48` | 系统被切成"内置/逻辑"两条流水线，逻辑系统跑在 Transform/Physics **之前** |
| P2-3 | `ECS/System/ISystem.h` | 契约不完整：无可见性控制、无阶段声明（现有枚举未用于路由）、无生命周期钩子定义 |
| P2-4 | `RendererSystem.cpp` | 清屏色硬编码；找相机逻辑与另外两处重复；每帧堆分配 + 排序；精灵缩放在渲染期现算 |
| P2-5 | `Graphics/Camera.h:48-52` | `Camera` 既管投影/视图（数学），又存"我是不是主相机"（场景语义） |
| P2-6 | `Graphics/Texture.cpp:14` | `Texture2D::Create(path)` → `Application::GetInstance()->GetAssetManager()->...` 三层解引用 + **分层倒置** |
| P2-7 | `UI/`（7 文件） | 与 ECS 平行的独立组件树，大部分空实现；`UIElement` 用 `enable_shared_from_this` 导致栈对象 `SetParent` 抛 `bad_weak_ptr` |
| P2-8 | `Assets/AssetManager.h:47-85` | `AsyncLoad` 缓存键写成 `find(asset)`（类型不符）；`std::thread` + `detach` 持有 `this`；`Load` 失败仍写缓存 → 永不重试；`Load` 无锁 |
| P2-9 | `Graphics/RendererAPI.cpp:9-23` | `Create()` 的 switch 被注释，无论 `s_API` 都返回 OpenGL；`Shader::Create` 绕过 `GetAPI()`；`OpenGLShader::SetInt` 是**空实现**；`AddVertexBuffer` 不处理 `Mat3/Mat4` |
| P2-10 | `Game/Application.cpp:12-30` | 初始化顺序错：`RendererCommand::s_RendererAPI` 静态初始化在 `main` 前；`Log::Init` 只初始化 CoreLogger；`Scene` 构造函数里就建主相机实体；**`PhysicsSystem::Init` 从未被调用**；`Renderer2D::Init` 早于 `Renderer::Init` |
| P2-11 | `Game/Application.h` | **`~Application()` 不是 virtual**（`BreakoutApp` 继承它 → UB 隐患）；`s_Instance` 析构不清空；`glfwTerminate` 无条件调用；`dt` 无上限钳制 |
| P2-12 | `Scene/Scene.cpp:56-85` | `DestroyEntity` 先 destroy 再取 UUID；无 `on_destroy` 钩子清理 `m_EntityMap`；`FindEntityByName` O(n) 且允许重名；UUID 冲突静默覆盖 |

### P3 可维护性

| 项 | 位置 | 问题 |
|---|---|---|
| P3-1 | `OpenGLShader.cpp:48-63` | **每帧每次 Set 都调 `glGetUniformLocation`** |
| P3-2 | `OpenGLBuffer.cpp:43` | `SetData` 用 `glBufferData` 每次重分配显存 |
| P3-3 | `Renderer2D.cpp:336` | 遍历 `TextureSlotMap` **按值拷贝** `shared_ptr` key |
| P3-4 | `RendererSystem.cpp:69` | 每帧构造 `vector<tuple<...>>` 堆分配 |
| P3-5 | `WindowsWindow.cpp` | 未开启垂直同步；**GLFW 初始化失败时日志被注释**（已修） |
| P3-6 | 多处 | 错误处理缺失：着色器编译失败留下未链接程序；纹理加载失败不记录状态；`dynamic_pointer_cast` 不检查结果；`Camera`/`Vector2` 除零 |
| P3-7 | `OpenGLShader.cpp:118-148` | 只取文件名再拼 `Shaders/`（传入前缀被忽略）；`m_Name` 被每个 `ReadFile` 覆写 → `GetName()` 返回片元文件名 |
| P3-8 | 全项目 | 资源路径依赖**编译期烤入的绝对路径** |
| P3-9 | 无 | 无测试、无 `/W4`、无静态检查、无性能基线 |

### P4 工程化 / P5 命名

| # | 位置 | 问题 |
|---|---|---|
| P4-1 | `ThirdParty/CMakeLists.txt:10` | entt 路径多空括号（同 P0-9） |
| P4-2 | `Source/CMakeLists.txt` | 无 `option()` 开关、无 install 规则、无 `CMakePresets.json`；两个 CMakeLists 带 **UTF-8 BOM** |
| P4-3 | 全项目 | 头文件包含风格混乱（`"../Bamboo/Core/Log.h"` / `"../Core/Log.h"` / `"Bamboo/..."` / `"./Core/Ref.h"`）；`AssetFactory.h:5` 用了小写 `core`（Windows 侥幸，跨平台必炸） |
| P4-4 | `Bamboo/CMakeLists.txt:3` | `GLOB_RECURSE` 不感知新增文件 |
| P4-5 | `Editor/CMakeLists.txt:64` | 复用了父作用域的 `BAMBOO_SRC` 变量；`add_compile_options` 应为 target 级 |
| P4-6 | 仓库根 | ~~缺少 `.gitignore`~~ → **已修复（2026-09-18）**，注意旧规则 `/build/` 从未生效（实际目录是 `Source/build/`） |
| P4-7 | — | 无 CI、无 CMakePresets、`project()` 无 VERSION |
| P5-1 | 多处 | `ShaderDatatType`、`FileUtlis`、`Shoudown`、`IsDirector`、`GetNormalTexture`（返回白纹理不是法线贴图）、`vertexPaht`；`MathDefs.h` 的 `B_HALF_PI = 1.4159...`（**数值错误**，应为 1.5708） |
| P5-2 | 多处 | 死代码（**已清理 122 行 → 0**）；模板占位符 `@author <NAME>`（**已清**） |

---

## 6. 目标架构与迁移路径

### 6.1 权威对照表（本文档核心结论）

| 要决定的事 | 现在的权威 | 目标权威 |
|---|---|---|
| 系统按什么顺序跑 | 无（`Scene::Update` 的书写顺序） | `SystemRegistry` + `RunsAfter()` **拓扑排序**，检测环并启动时报错 |
| 系统能不能写数据 | 无（都能写） | `ISystem`（可写）/ `IRenderSystem`（`const Scene&`，**编译期只读**） |
| 实体是否有效 | 无（约定"必须来自 Scene"） | `Entity::IsValid()`（改用 **`entt::handle`**） |
| 谁拥有组件里的资源 | `shared_ptr` 的所有持有者 | `AssetManager` 引用计数 + **帧末回收** |
| 资源的身份 | 路径字符串 | **GUID**（`.meta`） |
| 组件的字段有哪些 | 每个消费方各自手写 | **`ComponentRegistry`** 元数据 |
| 什么该被画 | 渲染器自己遍历 ECS | 收集系统产出 **`RenderView`** |
| 渲染器能读什么 | 整个 registry（可写） | 只有 `RenderItem`（**不依赖 ECS**） |

### 6.2 分层与依赖规则

```
Game / Editor / Sandbox     应用层
Scene                       编排权威（Scene · SystemRegistry · Serializer）
ECS                         数据权威（Entity · ComponentRegistry · Component）
Systems                     行为（逻辑/物理 · 收集 · 相机）
Render                      渲染契约权威（RenderView · RenderItem · Renderer2D · Camera）
Asset                       资源身份权威（AssetDatabase · AssetManager）
Platform / RHI              平台与后端（Window · Input · OpenGL）
Core                        无依赖基础（Math · Log · Assert · Ref · UUID）
```

**四条硬性规则**（违反即退化）：

| 规则 | 现状 |
|---|---|
| `Core` 不依赖任何其它层（含 entt/GLFW/GL） | ✅ |
| `Render` 不依赖 `Game`/`Scene`/`ECS` | ❌ `Texture.cpp` 依赖 `Application` |
| `ECS` 不依赖 `Render`/`Asset` | ❌ 组件持有 `Ref<Texture2D>` |
| `Asset` 不依赖 `Scene`/`Game` | ❌ `Texture2D::Create(path)` 依赖 `Application` |

### 6.3 关键接口草案

```cpp
// ---------- 执行顺序权威 ----------
enum class BuiltinPhase { PreUpdate, Update, PostUpdate, PreRender, Render };

class ISystem {
public:
    virtual std::string_view GetName() const = 0;        // 必须命名（依赖与调试都要用）
    virtual PhaseRef         GetPhase() const;           // { 阶段名, 阶段内顺序 }
    virtual std::vector<std::string_view> RunsAfter() const { return {}; }
    // 不提供 RunsBefore()：双向声明无法拓扑排序
    virtual void OnInit(Scene&) {}
    virtual void OnUpdate(Scene&, float dt) = 0;
};

class IRenderSystem {                                     // 只读视图系统
    virtual void OnRender(const Scene&, Renderer2D&) = 0; // const Scene& → 编译期强制只读
};

class SystemRegistry {
public:
    void RegisterPhase(std::string_view name, int order);  // 开放阶段列表
    template <typename T, typename... Args> T& Register(Args&&...);
    void Update(entt::registry&, float dt);                // 单一入口，不再有 4 个 Update
};

// ---------- 对象身份权威 ----------
class Entity {
public:
    explicit Entity(entt::handle handle) : m_Handle(handle) {}
    bool IsValid() const { return m_Handle && m_Handle.registry()->valid(m_Handle.entity()); }
    operator bool() const { return IsValid(); }
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        BAMBOO_ASSERT(IsValid(), "AddComponent on invalid entity");
        return m_Handle.emplace_or_replace<T>(std::forward<Args>(args)...);
    }
private:
    entt::handle m_Handle;   // 不再需要 Scene*，不再需要 public registry，不再循环包含
};

// ---------- 资源身份权威 ----------
class AssetHandle { UUID m_Id{}; public: bool IsValid() const; UUID GetId() const; };

struct SpriteRendererComponent {          // 组件 = 纯数据，不持有 GPU 对象
    AssetHandle Texture{};
    Color       Color = Color::White;
    Vector2     Size  = {100.0f, 100.0f};
    int         ZOrder = 0;
    float       Opacity = 1.0f;
    bool        Visible = true;
};

// ---------- 数据模式权威 ----------
BAMBOO_REGISTER_COMPONENT(TransformComponent, "Transform",
    BAMBOO_FIELD(Position), BAMBOO_FIELD(Rotation), BAMBOO_FIELD(Scale));
// 一处声明 → 序列化 / 检查器 / 复制粘贴 / 脚本绑定 四处受益
// 需要"跳过快照字段"标记：Dirty/WorldMatrix 等缓存字段不该存盘

// ---------- 渲染契约权威 ----------
struct RenderItem {
    Matrix4     Transform;
    AssetHandle Texture;      // 可空 → 白纹理
    Color       Tint;
    int         ZOrder = 0;
    uint32_t    Layer  = 0;
};
struct RenderView {
    Camera                  View;
    FrameBuffer*            Target = nullptr;   // null = 默认帧缓冲
    std::vector<RenderItem> Items;              // 已排序
};
class Renderer2D {
public:
    static void BeginFrame(const RenderView&);  // 取代 BeginScene/EndScene 的手工配对
    static void Submit(const RenderItem&);
    static void EndFrame();
};
template <typename TVertex>                     // 消灭 4 套重复状态
struct Batch { Ref<VertexBuffer> Buffer; Ref<VertexArray> Array; Ref<Shader> Shader;
               std::vector<TVertex> Vertices; uint32_t Capacity = 0, IndexCount = 0; };
```

### 6.4 迁移路径（不是重写）

**每一步都保持可编译、可运行。**

| 步骤 | 内容 | 范围 | 验证方式 |
|---|---|---|---|
| **A1** | `Entity` → `entt::handle`；`m_Registry` 私有化 | `Entity.h`、`Scene.h`（~120 行） | 现有代码编译通过；`FindEntityByName` 失败不再崩 |
| **A2** | 开放阶段列表 + 单一 `Update` + 拓扑排序 | `ISystem.h`、`SystemRegistry.{h,cpp}`、`Scene.cpp`（~200 行） | 故意写循环依赖 → **启动时报错** |
| **A3** | 拆出 `IRenderSystem`（`const Scene&`），渲染改只读 | `ISystem.h` + `RendererSystem`（~80 行） | 渲染系统**编译期无法**回写 ECS |
| **B1** | `AssetHandle` + `AssetDatabase` + `.meta`；组件字段改句柄 | 新建 ~4 文件 + 改 2 | 改贴图名后场景不断链 |
| **B2** | `ComponentRegistry` 元数据 + 首批 8 组件注册 | 新建 ~3 文件 | 能打印所有组件与字段 |
| **B3** | `SceneSerializer`（基于 B1+B2） | 新建 ~2 文件 | **往返测试**：Save→Load→Save 字节一致 |
| **C1** | `RenderItem`/`RenderView` + 收集/提交分离 + `Batch<T>` | 重写 `Renderer2D` + 新建收集系统 | 截图与重构前一致；可 headless 单测 |
| **C2** | `FrameBuffer` + 相机惰性重算 | 新建 ~2 文件 | 渲染到 512×512 FBO 截图正确 |

**顺序理由**：
- **A 组先做** —— 决定后面所有代码的形状，且现在只有 4 个系统要改（等写完物理/UI/脚本就是 8~10 个）。
- **B1 必须早于 B3** —— 资源身份先于序列化，否则序列化器要重写。
- **C 组可晚** —— 是"渲染质量"投资，玩法不受阻。但 **C2 的 FrameBuffer 必须早于编辑器**。

**总量约 1,400~1,800 行** —— 不是重写引擎（引擎才 4,600 行），是**补上五个权威**。

### 6.5 架构守护：让退化无法通过

文档最大的问题是没人执行。把规则变成自动化检查：

```powershell
# tools/check_architecture.ps1
$rules = @(
  @{ Dir='Bamboo/Graphics';      Forbid='ECS/|Scene/|Game/';            Msg='Render 层不得依赖 ECS/Scene/Game' }
  @{ Dir='Bamboo/ECS';           Forbid='Graphics/|GraphicsAPI/|Platform/'; Msg='ECS 层不得依赖渲染/平台' }
  @{ Dir='Bamboo/ECS/Component'; Forbid='Graphics/|GraphicsAPI/';        Msg='组件必须是纯数据' }
  @{ Dir='Bamboo/Core';          Forbid='glad|GLFW|entt|Scene/|Graphics/'; Msg='Core 层必须无依赖' }
)
# 另加三条：所有 .h 必须有 #pragma once；.cpp 不得出现裸 new[]/delete[]；
#          每个 ISystem 子类必须 override GetName()（否则拓扑排序无法报错）
```

### 6.6 什么不做（防止过度设计）

引擎只有 4,600 行、单人开发。**判据：这个抽象能消灭一类未来的 bug 吗？** 能就做，只是"更规范"就不做。

| 不做 | 理由 |
|---|---|
| ❌ 拆成多个 CMake target | 会显著增加构建成本。用 CI 脚本 grep 代替，效果 80%、成本 5% |
| ❌ 事件总线 / 消息系统 | 目前无需求。等 UI 交互与碰撞回调需要时再加 |
| ❌ 反射代码生成 | 手工注册 8 个组件的成本 < 写生成器。超 20 个组件再考虑 |
| ❌ 换 ECS 库 / 自己写 ECS | entt 没问题，问题在**怎么用它** |
| ❌ 立即模式 → 保留模式大改 | `RenderItem` 列表够了。10k+ 实体实测有瓶颈时再说 |
| ❌ 多后端渲染抽象 | 建议"诚实降级"：承认 OpenGL-only，删掉空转的抽象层（见 D1） |

---

## 7. 游戏计划：用游戏验收引擎版本

### 7.1 方法论

1. **每个游戏只能用引擎已有的能力做完。** 想用而引擎没有 → 那就是下一个引擎任务。
   **不要为了跑起来而在游戏代码里绕过引擎**（那会让引擎永远得不到该能力）。
2. **引擎版本号由游戏定义**：`v0.x.0` 的发布条件 = **某个游戏从"打开编辑器摆关卡"到"打包运行"全流程走通**。
3. **上一次的游戏必须永远能跑。** 每次引擎改动后 G1~Gn 全部回归——游戏即永久测试用例。

**反模式**：❌ 为了引擎而做游戏（不想玩的别做）｜❌ 先造完整引擎｜❌ 游戏代码里写引擎补丁｜❌ 跳过 G1/G2 直接做想做的那个

### 7.2 游戏阶梯

| # | 游戏 | 验收版本 | 一句话 | 引擎能力重点 | 估时 |
|---|---|---|---|---|---|
| **G1** | **Breakout** | v0.1.0 | 冒烟测试：从"能出画面"到"真能玩" | 渲染批次、相机、输入、AABB 碰撞、**最小 C# 脚本** | 3~5 天 |
| **G2** | **横版平台跳跃沙盒** | v0.2.0 | **编辑器的诞生**：只用鼠标摆关卡并试玩 | 序列化、编辑器、**预制体**、撤销、触发器/碰撞层、最小音频 | 3~4 周 |
| **G3** | **幸存者类** | v0.3.0 | 1,500 实体 60FPS，**全 C# 驱动** | 空间分区、对象池、批量查询、性能工具 | 3~4 周 |
| **G4** | **开屏（开箱/抽卡）** | v0.4.0 | **UI 系统重构的验收场** | RectTransform、多层弹窗、滚动列表、网格布局、数据表 | 3~4 周 |
| **G5** | **SLG 战棋** | v1.0.0 | 完整项目，能发布 | Tilemap、寻路、回合制、范围高亮、技能系统、存档 | 8~12 周 |
| G6 | 物理沙盒（可选） | v0.5.0 | 不做游戏，验证物理稳定性与确定性 | 完整物理、可复现重放、调试绘制 | 1~2 周 |

**顺序理由**：G1 已有素材、3~5 天就能拿到"能玩的东西"；G2 是**杠杆最大**的一步（编辑器让后续所有游戏的关卡搭建从"改代码"变成"拖鼠标"）；G3 是**最省的性能压力测试**（玩法极简、数量极大）；G4 是**唯一能逼出 UI 系统**的类型；G5 必须在工具齐备后做。

### 7.3 G5：为什么选 SLG 而不是 RPG

| | SLG 战棋 | RPG（剧情向） |
|---|:--:|:--:|
| 需要的新子系统 | **约 6 个** | **约 11 个** |
| 内容量 | 中（几张地图 + 10~20 单位） | **极高**（10~40 小时剧情 = 海量文本/美术/音乐） |

RPG 要多做：**对话系统、事件/触发器、背包/装备/商店、任务日志、场景切换、大地图**——每个都是独立模块，没法用已有能力替代。

选 SLG 的六个理由：① 新增子系统少一半 ② 内容量可控（关卡 = 一张地图 + 几个单位 + 一个胜利条件，8~10 关即可发布，且**能在编辑器里摆**）③ 引擎需求与已有能力高度重叠（是"渲染+物理形状+UI"的自然延伸）④ **可测试性极好**（回合制 + 确定性 → 可写自动化回放测试）⑤ AI 是可控难点（做差了游戏也能玩）⑥ 素材门槛低（`Into the Breach` 就是极简美术）。

**唯一让 RPG 更好的情形**：你对某个 RPG 故事有强烈表达欲且接受 1~2 年的项目。那热情能带你走完，工程理由都要让位。但作为**第一台引擎上的第一个完整游戏**，SLG 是明显更优的工程决策。

### 7.4 G5 范围锁定（防膨胀 —— 最重要的一节）

| 项 | **做** | **明确不做** |
|---|---|---|
| 地图 | 单张 **12×12 ~ 16×16**，固定关卡 | ❌ 大地图/世界地图 ❌ 程序生成 ❌ 多层地形 |
| 单位 | **4~6 个**，**3 兵种**（近战/远程/治疗） | ❌ 转职 ❌ 职业树 ❌ 装备槽 ❌ 等级成长 |
| 回合 | 玩家回合 → 敌方回合；胜负 = 歼灭/占领/存活 N 回合 | ❌ 行动条/速度值 ❌ 同时回合 |
| 战斗 | 攻击 → 伤害计算 → 反击（可选） | ❌ 命中/闪避/暴击复杂公式 ❌ 战斗动画（只做位移 + 数字弹出） |
| 技能 | **6~8 个**，数据表驱动 | ❌ 技能树 ❌ 被动叠加 ❌ 复杂 Buff 结算 |
| 地形 | **3 种**（平地/障碍/增益格 +20% 防御） | ❌ 地形组合 ❌ 地形改造 |
| AI | **3 条规则**：能打打最近 / 血少后退治疗 / 否则向最近敌人移动 | ❌ 威胁评估/博弈/难度分级 ❌ 行为树编辑器 |
| 关卡 | **8~10 关**，难度递增，**全部在编辑器里手工摆放** | ❌ 支线 ❌ 多结局 ❌ 程序生成 |
| 剧情 | 每关一段文字过场（一张图 + 几行字） | ❌ 对话系统 ❌ 好感度 ❌ 任务系统 |
| 存档 | **单档位** | ❌ 多槽位 ❌ 云存档 |
| 美术 | 极简（纯色块/几何图形即可） | ❌ 立绘 ❌ 行走动画（用位移 tween） ❌ 场景美术 |
| 音频 | **5~8 个音效** + 1 首 BGM | ❌ 每关不同 BGM ❌ 音乐淡入淡出 |
| 界面 | **4 个**：主菜单、关卡选择、战斗、结算 | ❌ 设置页 ❌ 图鉴 ❌ 成就 |

**开工第一步**：把上面的"不做清单"抄进 `Sandbox/Games/G5_Tactics/README.md`——以后想加东西时会先看到自己写的话。

**自检问题**：每次想加东西时问 **"这一条会让发布推迟几天？它让游戏更有趣吗？"** 答案是"3 天 / 不是特别"就不做。

### 7.5 G4：为什么"开箱"比卡牌更适合做 UI 验收场

它把 UI 系统最难的几件事**一次全压出来**，而逻辑极简（不需要战斗规则、回合制、平衡）：

三层弹窗+遮罩（主界面→开箱→结果→详情）、滚动列表+网格、**同一 UI 预制体实例化 100 次各显示不同数据**、稀有度视觉分级、进度条+数字滚动、点击/悬停/长按/拖拽、界面栈与返回。

**验收 #12 是真正的目的**：

> **`Source/Bamboo/UI/` 已删除，UI 全部走 ECS 组件，且 Hierarchy/Inspector/序列化/预制体对 UI 元素无需特殊代码即可工作。**
> 如果为了让 UI 能在检查器里编辑、能存盘、能做预制体，你不得不在编辑器里为 UI 加"专用分支"——说明整合没做对，**回去重做整合，而不是加分支**。

---

## 8. 编辑器设计

### 8.1 定位（决定成败的选择）

**编辑器是「引擎内的一个 Layer」，不是「另一个程序」。**

现状 `Editor/Source/Core/Application.cpp` 自己 `glfwInit` + `glfwCreateWindow`，完全不用引擎的 `Application`/`Window`/`Scene`/`Renderer2D` → 它根本不是引擎的编辑器。

```
Bamboo::Application
 ├─ LayerStack
 │   ├─ GameLayer        ← 运行场景（Game/Editor 共用同一份）
 │   └─ EditorLayer      ← 只在编辑器模式挂载
 │        ├─ ViewportPanel  (渲染到 FBO 再贴到 ImGui)
 │        ├─ HierarchyPanel / InspectorPanel / ContentBrowserPanel / ConsolePanel
 │        └─ GizmoLayer
 └─ Scene / AssetManager / Renderer2D
```

**为什么关键**：只有编辑器和游戏跑在**同一份 Scene/Renderer** 上，编辑器里看到的才**保证**和游戏里一致。否则你会永远在修"编辑器里对、游戏里不对"。这也是 Unity/Godot/Unreal 的共同选择。

**三种模式**：Editor（Game+EditorLayer，可暂停/单步）、Game（仅 GameLayer）、Headless/Test（无窗口无 GL，供 CI）。

### 8.2 默认布局（对齐 Unity）

```
┌──────────────────────────────────────────────────────────────────────┐
│ 菜单栏  File Edit GameObject Component Assets Scene Window Help      │
├──────────────────────────────────────────────────────────────────────┤
│ 工具栏  [▶ ⏸ ⏭ ⏹] [移动 旋转 缩放] [世界/本地] [吸附:16] [2D] [统计]  │
├───────────────┬──────────────────────────────┬───────────────────────┤
│  Hierarchy    │      Scene （编辑视图）       │    Inspector          │
│  ▸ ▼ Level01  │   网格 / 坐标轴 / 碰撞线框    │  Player          [✓]  │
│    ▸ ▼ Player │        ┌────┐                │  ▼ Transform          │
│       • Sprite│        │ ▣  │ ← Gizmo       │    Position  x y z    │
│    ▸ Platform │        └────┘                │  ▼ Sprite Renderer    │
├───────────────┼──────────────────────────────┼───────────────────────┤
│  Project      │      Game （运行预览）        │    Console            │
│  （内容浏览器）│      按分辨率预览             │    [!] [x] > 命令_     │
└───────────────┴──────────────────────────────┴───────────────────────┘
```

| 优先级 | 面板 | 关键功能 |
|---|---|---|
| **P0** | Scene | 视口、网格、选择、Gizmo、碰撞线框、缩放平移 |
| **P0** | Game | 运行预览、分辨率下拉、Stats 叠加 |
| **P0** | Hierarchy | 树、搜索、增删改、拖拽改父级、可见/锁定、右键菜单 |
| **P0** | Inspector | **从组件注册表自动生成**；增删组件；字段级 Undo；多选编辑 |
| **P0** | Project | 目录树、缩略图、拖拽到 Scene、新建/重命名/删除 |
| **P0** | Console | 分级过滤、搜索、折叠重复、命令输入 |
| **P1** | Stats / Scene Settings / UI Builder / Prefab 编辑模式 | |
| **P2** | Asset Preview / Build Settings / Profiler | |

**明确不做（编辑器镀金清单）**：❌ 多窗口/多显示器 ❌ 主题市场 ❌ 可视化脚本（C# 就是）❌ 内置商店/版本管理 ❌ 动画曲线编辑器 ❌ 材质节点图

### 8.3 技术底座（必须先有）

| # | 组件 | 说明 |
|---|---|---|
| 1 | `Layer` / `LayerStack` + 事件按层传播 | 编辑器是引擎内的一个 Layer |
| 2 | `Editor::Application : Bamboo::Application` | 不再自建窗口 |
| 3 | ImGui GL3 后端完整接入 | 补 `ImGui_ImplOpenGL3_Init` / `NewFrame` / `RenderDrawData` / `Shutdown` |
| 4 | **ImGui Docking 分支** | ⚠️ 内置的是 **1.92.4 master，无 `IMGUI_HAS_DOCK`** → Unity 式可停靠布局的前提是**换 docking 分支**（约 1 小时）。备选：手动固定布局 |
| 5 | 编辑器字体 | 含中文的 ttf + 字形范围，否则面板全乱码 |
| 6 | **`FrameBuffer`** | 场景视图渲染到 FBO 再贴进 ImGui（**硬前置**） |
| 7 | `EditorState` | 选中集合、面板开关、编辑器相机、吸附设置（**不放 Scene**） |

### 8.4 Scene View 交互

| 操作 | 行为 |
|---|---|
| 中键拖拽 | 平移视图（最高频） |
| 滚轮 | **以鼠标位置为锚点**缩放（0.1x~10x） |
| F / Shift+F | 聚焦选中 / 聚焦并锁定 |
| 左键点击 | 选中光标下最上层（按渲染层级/Gizmo 优先） |
| 左键拖空白 | 框选；Ctrl+点击加选/减选；Alt+点击循环重叠 |
| W / E / R / T | 移动 / 旋转 / 缩放 / Rect（UI） |

**Gizmo 设计要点**：屏幕空间恒定大小｜拖拽平面求交｜Ctrl 吸附（位置 16px、旋转 15°）｜拖拽时显示实时数值｜**一次拖拽 = 一条 Undo**｜多选时在重心｜选中物体加轮廓高亮。

**鼠标状态与输入隔离**（最容易出 bug 的地方）：

| 状态 | 判据 | 效果 |
|---|---|---|
| 鼠标在 Scene 视口 | `IsWindowHovered()` | Scene 接受导航；游戏世界输入**屏蔽** |
| 鼠标在其它 ImGui 面板 | `WantCaptureMouse` | Scene 不响应；游戏输入屏蔽 |
| 正在拖 Gizmo | 内部标志 | **即使移出视口也继续拖拽**（否则手感极差） |
| Play 模式 + 鼠标在 Game 视口 | `IsWindowHovered()` | **游戏输入开启**（唯一入口） |

### 8.5 摆物体工作流（G2 验收脚本）

| 需求 | 实现 |
|---|---|
| 创建 | 菜单 `GameObject ▸ Create Empty/Child`；`Ctrl+Shift+N`；**拖贴图到 Scene 自动建带 SpriteRenderer 的实体**；拖 `.prefab` 实例化 |
| 变换 | Inspector 精确输入（拖数值标签可滑动）；Gizmo 拖拽；**格点吸附（默认 16px）**；对齐/等距分布；Transform 齿轮 Reset |
| 层级 | Hierarchy 拖拽改父级，**必须显示插入指示线**；**按住 Shift 拖拽 = 保持世界位置**（Unity 细节）；眼睛/锁图标 |
| 预制体 | 拖到 Project 生成 `.prefab`；实例顶部显示 **Overrides 列表 + Apply/Revert**；双击进入预制体编辑模式。**嵌套预制体先不做** |
| 批量 | 框选；Inspector 显示共有字段并批量应用；`Ctrl+D` 复制偏移；`Ctrl+G` 编组 |
| Undo | 字段改动（连续拖同一滑块合并为一条）／Gizmo 拖拽／增删复制／改父级／批量／组件增删／Prefab Apply。**选择变更与相机移动不进 Undo 栈** |

**这 10 步必须全部可用**（v0.2.0 验收）：新建场景 → Create Empty → Add Sprite Renderer → Add Rigidbody2D + BoxCollider2D → 右键建脚本 → Add Script 组件并填字段 → 拖入层级 → Ctrl+S → Play 试玩 → Stop 还原 → Build 打包。

### 8.6 拼 UI 工作流（G4 验收脚本）

**技术选型：重写为基于 ECS 的 UI，删除现有 `Source/Bamboo/UI/`。**

| 理由 | 说明 |
|---|---|
| 检查器自动可用 | UI 元素如果是实体，Inspector/Hierarchy/序列化/预制体**全部免费复用** |
| 一套层级 | 不必维护"场景树 + UI 树"两套父子关系 |
| 渲染统一 | 走同一个 `Renderer2D` 批次，只需"UI 层在最上面"的排序规则 |
| 代价可接受 | 现有 `UIElement` 里真正有用的只有 `Position/Scale/Rotation/Size/Anchor` 和父子关系 |

**组件清单**：`CanvasComponent`、`RectTransformComponent`（**取代 Transform**）、`UIImageComponent`、`UITextComponent`、`UIButtonComponent`、`UILayoutGroupComponent`、`UIPanelComponent`

**RectTransform 锚点语义 —— 照抄 Unity，不要自己发明**：

```
AnchorMin/AnchorMax ∈ [0,1]²   锚点矩形（相对父矩形）
Pivot ∈ [0,1]²                  自身轴心
AnchoredPosition                轴心相对"锚点矩形对应点"的像素偏移
SizeDelta
  ├─ AnchorMin == AnchorMax（点锚点）：SizeDelta = 实际尺寸
  └─ AnchorMin != AnchorMax（拉伸）：SizeDelta = 相对锚点矩形的尺寸增量
```

> 这是 UI 布局里最反直觉但最标准的约定。自己发明一套会导致"换分辨率就乱"且没人能帮你调试。**写单元测试**：给定父矩形 + 锚点参数 → 期望的最终矩形（纯数学，完全可测）。

**UI 编辑模式**：设计态显示元素边界（**即使没贴图也能看见**）+ 锚点虚线/角标；运行态显示真实效果。**Rect Tool（T）**：边框手柄改 `SizeDelta`、锚点手柄改 `AnchorMin/Max`（Alt 拖拽 = 同时移动锚点与元素）、轴心点改 `Pivot`。**Scene 视图顶部一个分辨率下拉**——切换后立即看到不同分辨率的表现，这是验证锚点最快的手段。

**8 步验收**（v0.4.0）：建 Canvas → 建 Image 设全拉伸 → 建 Text 设左上锚点 → 建 Button 设三态颜色 → 挂 C# 脚本并填 fields → 切 3 种分辨率检查不跑版 → Ctrl+S → Play 点击生效 → Stop 还原。

**渲染顺序规则**：UI 在游戏世界之上（分两批）｜UI 内部顺序 = 层级遍历顺序（父在前子在后，同级按 Hierarchy 顺序）｜弹窗遮罩 = 全屏半透明 Image + `blocksRaycast`｜**Mask 先不做**（用设计上避免溢出绕开）

### 8.7 编辑器与引擎的版本协同

| # | 规则 | 落地 |
|---|---|---|
| 1 | 编辑器只用引擎的**公开 API** | 出现 include 引擎内部头即违规，CI 检查 |
| 2 | 游戏需要的每个能力 → **先做进引擎 + 加测试，再在游戏里用** | 游戏代码里出现绕过引擎的 hack → 立刻转成引擎任务 |
| 3 | 每次引擎改动 → **跑全部已有游戏作为回归** | 这是"游戏即测试"的兑现 |

**版本号规则**：`MAJOR`（存档格式/API 不兼容）｜`MINOR`（新增能力/某个游戏走通，**与游戏里程碑绑定**）｜`PATCH`（修 bug）。
**兼容性承诺（编辑器的生命线）**：场景/预制体/UI 文件**向前兼容**（靠版本迁移链）｜**未知组件保留**（旧版本打开新场景存盘不丢数据）｜脚本 API 变更加 `abi_version` 并在启动时报错｜GUID 稳定。

**迭代循环**：选一个游戏 → 做 1~2 个玩法片段 → 遇到引擎缺的能力记进 backlog → 按优先级做引擎能力（附测试）→ 回到游戏 → 跑全部旧游戏回归 → 打 tag / 写 CHANGELOG / 录 GIF → 下一个。
**"backlog 驱动"而不是"清单驱动"** —— 这是游戏优先的核心。

---

## 9. C# 脚本：关键技术约束

> 完整技术调研（.NET 版本、NativeAOT 导出规则、hostfxr 序列、可回收 ALC 的完整护栏、5 段可复制代码、11 项未验证清单）见 **[`research_csharp_embedding.md`](research_csharp_embedding.md)**。这里只放**决策与硬约束**。

### 9.1 已定方案

**选 B：CoreCLR + 可卸载 `AssemblyLoadContext`**（脚本语言 = C#）。发布阶段再评估 D（同一份脚本源码加一条 NativeAOT 构建路径）。

### 9.2 决定性事实：NativeAOT 与热重载互斥

微软官方文档原文：*"Unloading Native AOT libraries (via `dlclose` or `FreeLibrary`) is not supported."*（[文档](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/libraries)、[官方样例](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/README.md)、[dotnet/runtime#54617](https://github.com/dotnet/runtime/issues/54617)）

**所以"NativeAOT 编译的脚本"= 每次改一行都要重启引擎。这不是难度问题，是设计上不提供。**

### 9.3 最关键的工程约束：跨边界函数指针只能指向「不可回收的桥接程序集」

这条不遵守，CoreCLR 路线的热重载**必然失败**（且静默失败）：

| 拿函数指针的方式 | 对 ALC 卸载的影响 | 后果 |
|---|---|---|
| `Marshal.GetFunctionPointerForDelegate(delegate)` | 运行时生成 loader-allocator stub 并持有**强句柄** | **ALC 永远卸不掉**（内存单向增长） |
| 裸 `delegate* unmanaged<...>`（`&Method`） | 不产生 GC 引用，不阻止卸载 | ALC 一死，指针**悬垂** → 下次调用**必然崩溃** |

**两条路都通向灾难。正确架构**：

```
引擎进程
 ├─ C++ 宿主
 ├─ Bootstrap 程序集（不可回收，Default ALC）  ← 跨边界函数指针只能指向这里
 │    LoadScripts(path, HostApi*) / ReloadScripts() / Tick(dt) / Shutdown()
 │    内部：创建 collectible ALC → LoadFromAssemblyPath → 反射取 MethodInfo
 └─ 脚本程序集（collectible ALC，可整块丢弃）   ← 只放逻辑，绝不被 C++ 直接持有指针
      只能通过 HostApi 表回调宿主（表指向 C++ 函数，永远不在 ALC 内）
```

**其他硬约束**：
- **不要用 `hdt_load_assembly_and_get_function_pointer`** —— 它的隔离 ALC 你拿不到引用、无法卸载，返回指针是进程生命周期且官方明说没有释放方式。`hdt_load_assembly` 加载进 Default ALC（不可回收）同样不行。
- **卸载是协作式且静默失败**。已知钉死来源：线程栈上有 ALC 帧、任意一侧的强 `GCHandle`、静态字段、事件订阅、`RegisteredWaitHandle`、**以及你自己的 ALC 子类的字段**。
- **第一道防线**：每次重载后 `GC.Collect()` + `WaitForPendingFinalizers()` + **`WeakReference.IsAlive` 断言旧 ALC 已回收**，不回收就告警。必须写进代码。
- **CI 跑"连续重载 200 次"压测** —— .NET 10 上出现过"反复 load/unload 3~4 次后泛型虚派发挂死一个核"的回归（[#132562](https://github.com/dotnet/runtime/issues/132562)），说明反复重载是**测试覆盖不足**的路径。

### 9.4 宿主 API 表（纯 C ABI）

```c
typedef struct BambooHostAPI {
    uint32_t abi_version;                              // 脚本层先检查，不匹配即明确报错
    uint64_t (*entity_create)(const char* name);
    void*    (*component_get)(uint64_t e, const char* typeName);
    bool     (*input_key_down)(int keyCode);
    void     (*log_info)(const char* msg);
    // ...
} BambooHostAPI;
```

**设计理由**：纯 C ABI（C#/Lua/未来 Rust 都能用）｜`abi_version` 校验（不匹配明确报错而非随机崩溃）｜**`component_get` 用类型名 → 脚本能访问的组件 = 注册过的组件，自动同步，不需要为每个组件写绑定**。

**实体寻址必须用"代际句柄"，不要裸指针**：

```c
typedef uint64_t EntityHandle;   // 低 32 位 slot 索引，高 32 位 generation
```

好处：能检测悬垂句柄｜**句柄是纯整数，热重载后依然有效**（裸指针重载后全部失效）。

### 9.5 AOT-clean 纪律（从第一天就守，否则将来切不过去）

| 禁止 | 原因 |
|---|---|
| `System.Reflection`（除宿主类发现）／`Type.GetType(string)` | NativeAOT 下受限/不可靠 |
| `dynamic`／`Reflection.Emit`／`Expression.Compile` | AOT 完全不支持 |
| 泛型虚方法派发 | .NET 10 反复 load/unload 后有挂死回归 |
| 脚本 `Update` 里的堆分配（LINQ、闭包捕获、装箱） | GC 尖峰 + 帧时间抖动（G3 靠它达标） |
| 在脚本里长期缓存组件指针 | ALC 重载后悬垂 |
| 跨边界传 `string`/`bool`/数组作为导出参数 | 非 blittable（`bool` → `byte`；字符串 → `byte*` + len） |

**必做三件事**：① **结构体布局自检**（启动时 C# 上报 `Marshal.SizeOf<T>()`，C++ 上报 `sizeof(T)` + 字段偏移，不一致明确报错；C++ 侧再加 `static_assert`）② **脚本类显式注册**（不用反射扫描，便于 AOT）③ 分配检测（Debug 下对比 `GC.GetAllocatedBytesForCurrentThread()`）

### 9.6 构建与生命周期

| 阶段 | 方式 |
|---|---|
| 早期（G1~G2） | `dotnet build` → `BambooScripts.dll` → 编辑器启动时加载；编辑器里按钮触发"重编译 + 重载" |
| 中期（G3~G4） | 同上 + 文件监听自动重编译 |
| 发布（G5） | 同一份源码 `-p:PublishAot=true -p:NativeLib=Shared` 出原生库（**发布态无热重载**，玩家也不需要） |

**生命周期**：`Awake() → Start() → Update(dt) → LateUpdate(dt) → OnDestroy()`。执行顺序显式定义：`所有 Awake → 所有 Start → 逐个 Update（按实体 ID 稳定序）→ 物理 → 逐个 LateUpdate`。**`Update` 的实体顺序必须稳定**，否则帧间行为不确定，调试噩梦。

**目标框架**：本机目前只有 SDK 9.0.305（**.NET 9 是 STS，2026-11-10 EOL**）。当前 LTS 是 **.NET 10**（支持到 2028-11）。**建议装 .NET 10 SDK，TFM 用 `net10.0`，并开启 `CheckSdkVulnerabilities`**。

---

## 10. 工程化：测试 / 构建 / CI

### 10.1 测试分层

```
┌─ 性能基准 benchmark ── 数字回归：帧耗时/DrawCall/物理步进，超标即红
├─ 渲染快照 snapshot ── 参照场景截图比对（容差 1%），拦视觉回归
├─ 集成测试 integration ── Scene+系统+序列化+资源（headless，无 GL）
├─ 单元测试 unit ──────── Math / ECS / 序列化 / 物理 / 工具（占 80% 数量）
└─ 静态检查 static ────── /W4+/WX、clang-tidy、分层守护、.meta 完整性
```

**关键约束：单元/集成测试不依赖 GL 和窗口。** 需要"headless 模式"（`Scene` 脱离 `Window` 运行）+ 渲染测试改用"CPU 侧批次内容断言"。**这是让引擎可测的核心设计，不是测试的附属品。**

### 10.2 必测清单

| 模块 | 内容 |
|---|---|
| Math | 全运算符、**`Matrix3 * Vector3` 与手算比对**、`RotateXYZ` 角度制、`AABB::Merge`、`Color::FromHex` 全格式、`Vector2::Cross` 标量语义、除零行为、`Inverse`/`Determinant` 一致性 |
| ECS | 组件增删查、**空 `Entity` 所有操作不崩**、销毁后 map 无残留、系统按阶段稳定排序 |
| 序列化 | **往返字节一致**、100 实体全字段往返、旧版本迁移、**fuzz 不崩** |
| 物理 | 自由落体 vs 解析解、形状两两碰撞、响应能量/动量、**宽相与暴力法结果完全一致** |
| 资源 | GUID 稳定性（改名不断链）、引用计数、卸载无泄漏、`.meta` 缺失检测 |
| 脚本 | 边界调用、异常隔离、**重载 50 次无泄漏**、ABI 版本校验 |
| 渲染 | 批次容量边界（画 N 个恰好换批）、顶点数据正确性、截图快照 |

**回归防空转**：把 `Matrix3::operator*(Vector3)` **故意改回错误版本 → 测试必须红**（验证测试真的有效）。

### 10.3 构建与 CI

- 加 `BAMBOO_BUILD_TESTS` / `BAMBOO_BUILD_EDITOR` / `BAMBOO_BUILD_SANDBOX` 开关
- 加 `CMakePresets.json`（`debug`/`release`/`ci`），替代手敲命令
- `project(BambooEngine VERSION 0.1.0)`
- `/W4` 清零 → 加 `/WX`（至少 CI 上）
- GitHub Actions：configure → build → `ctest` → 性能对比
- 覆盖率门槛：总行 ≥ 70%；Math/ECS/序列化/物理 ≥ 85%
- 源文件收集改 `CONFIGURE_DEPENDS` 或显式列文件（解 §3.2）
- 统一头文件包含根（解 P4-3）
- 去掉 `Source/CMakeLists.txt` 与 `Source/Bamboo/CMakeLists.txt` 的 UTF-8 BOM

### 10.4 调试与可观测性

| 工具 | 优先级 |
|---|---|
| **Debug HUD**（FPS / 帧耗时 / DrawCall / 批次数 / 实体数 / 物理耗时 / 内存 / 鼠标世界坐标） | P0 |
| **控制台 + 命令系统**（`scene.load`、`physics.debug on`、`timescale 0.5`；命令可注册，脚本也能注册） | P0 |
| **时间控制**（暂停 / 单步 / 时间缩放） | P0 |
| **性能计时器**（`BAMBOO_PROFILE_SCOPE` 宏 + 层级计时 + 输出 CSV） | P0 |
| 日志升级（分级可配、channel、文件 sink + 轮转、ImGui Console sink） | P0 |
| 场景调试绘制（碰撞体线框、速度矢量、网格、相机框） | P1 |
| 崩溃报告（SEH + minidump + 记录最近日志与场景路径） | P1 |
| 自动化截图/录像（`--screenshot <scene> <out.png>`） | P1 |

---

## 11. 全局验收标准

### 11.1 功能

| # | 验收项 | 判定 |
|---|---|---|
| F1 | 启动不崩 | 连续启停 50 次零崩溃、零 GL 错误 |
| F2 | 分辨率无关 | 800×600 / 1280×720 / 1920×1080 / 2560×1440 下截图对比误差 < 1% |
| F3 | 窗口缩放正确 | 不变形、不拉伸、黑边策略一致 |
| F4 | 渲染正确 | 精灵/四边形/圆/线/文字正确；Z 排序稳定；透明混合正确 |
| F5 | 物理正确 | 自由落体 `v=g·t` 误差 < 1%；球撞墙反射角误差 < 1° |
| F6 | 场景存读 | 编辑→保存→重启→打开，截图一致 |
| F7 | 序列化鲁棒 | 损坏/截断/字段缺失/未知组件/旧版本**不崩溃**，有明确错误 |
| F8 | 资源热重载 | 改贴图/着色器，**2 秒内**生效 |
| F9 | 编辑器可用 | 编辑器内完成"新增实体→加组件→改属性→拖位置→存盘" |
| F10 | 脚本可用 | C# 写移动/碰撞逻辑，改脚本重载 < 1 秒（CoreCLR） |
| F11 | 发布可用 | 一条命令产出可分发目录，**未装 SDK 的干净机器**能跑 |

### 11.2 性能（必须有前后对比数字）

| # | 场景 | 目标 |
|---|---|---|
| P1 | 10,000 静态精灵 | DrawCall ≤ 20，CPU 帧耗时 < 4ms |
| P2 | 2,000 动态精灵 | 稳定 60 FPS（< 16.6ms） |
| P3 | 1,000 动态刚体 + 100 静态 | 物理步进 < 4ms（对比暴力 O(n²) 的加速比） |
| P4 | 1,000 实体场景加载 | < 500ms |
| P5 | 编辑器 5,000 实体空闲 | UI 帧耗时 < 8ms |
| P6 | 反复进出场景 20 次 | 显存与内存**不增长** |
| P7 | 1,000 脚本实体 Update | 总耗时 < 2ms |

### 11.3 工程

| # | 判定 |
|---|---|
| E1 | 全新 clone → 一条命令 configure + build；`/W4` 零警告 |
| E2 | `ctest` 全绿；覆盖率达标 |
| E3 | push 后自动构建 + 测试 + 性能基线对比；回归会红 |
| E4 | 架构守护脚本零违反 |
| E5 | README 能让人 30 分钟内跑起来；`docs/` 无过时文档 |
| E6 | Conventional Commits；每 feature 独立分支；无大文件入库 |

### 11.4 每个任务的 Definition of Done

1. **能编译** —— `/W4` 零警告零错误（Debug 与 Release 都过）
2. **有测试** —— 纯逻辑有单测，集成部分有集成测试，渲染有快照或 CPU 侧断言。**没有测试的功能视为未完成**（唯一例外：纯 UI 布局用手动清单）
3. **有验收证据** —— 能填上"通过 + 具体数字/截图路径"
4. **有文档** —— 公开 API 有注释；改变了使用方式就更新本文档
5. **有提交** —— 独立分支、Conventional Commit、`git status` 干净
6. **没留死代码** —— 注释掉的旧实现删除；`TODO` 必须带名字和具体内容
7. **有性能数字** —— 声明了性能目标就必须附前后对比，不能只写"更快了"
8. **不破坏已有验收** —— F1~F11 / P1~P7 / E1~E6 受影响项仍通过

---

## 12. 路线图与里程碑

### 12.1 阶段

```
第 0 步 卫生（半天）
  加 .gitignore ✅ ｜ 修 CMake entt 路径 ✅ ｜ 删 build 从零重建 ✅（已验证）
  ｜ 修 IntVector2 的 C++20 错误 ✅ ｜ 装 .NET 10 SDK ⬜

A 组：架构骨架（~400 行）—— 决定后面所有代码的形状
  A1 Entity → entt::handle ｜ A2 开放阶段 + 拓扑排序 ｜ A3 IRenderSystem 只读
        ↓
B 组：数据与资源（~600 行）
  B1 AssetHandle + AssetDatabase + .meta ｜ B2 ComponentRegistry ｜ B3 SceneSerializer
        ↓
C 组：渲染契约（~600 行）
  C1 RenderItem/RenderView + Batch<T> ｜ C2 FrameBuffer + 相机惰性重算
        ↓
G1 Breakout 能玩（含 C# 驱动） → G2 编辑器锁关卡 → G3 性能 → G4 UI → G5 SLG 发布
```

### 12.2 版本里程碑

| 版本 | 发布条件（= 某个游戏全流程走通） | 关键新能力 | 估时 |
|---|---|---|---|
| **v0.1.0** | **G1 Breakout 能玩**（逻辑用 C# 写） | 批次、相机、输入、AABB、**最小脚本 + 热重载** | 1.5~2 周 |
| **v0.2.0** | **G2 只用鼠标摆关卡并试玩** | 序列化、编辑器、预制体、撤销、触发器、最小音频 | 3.5~4.5 周 |
| **v0.3.0** | **G3 1,500 实体 60FPS 全 C# 驱动** | 空间分区、对象池、批量查询、性能工具 | 3~4 周 |
| **v0.4.0** | **G4 全部界面在 UI 编辑器里搭出来**（UI 重构验收） | UI 系统重写、RectTransform、UI 事件、布局、数据表 | 3~4 周 |
| v0.5.0 | G6 物理沙盒全部用例通过（可选） | 物理稳定性、确定性、CCD、调试绘制 | 1~2 周 |
| **v0.9.0** | **G5 的 8~10 关全部能在编辑器里摆出来并打完** | Tilemap+刷子、格子系统、A*、范围高亮、回合制、技能 | 4~6 周 |
| **v1.0.0** | **G5 SLG 打包发布，别人能玩** | 打磨、美术、关卡平衡、打包、文档 | 4~6 周 |

**总计约 5~7 个月**（单人、每周 15~20 小时）。**v0.9.0 单独拆出来**——"内容完成"和"打磨发布"是两件事，前者才是"引擎够用了"的证明。

### 12.3 能演示的里程碑

| 里程碑 | 时间点 | 你能演示什么 |
|---|---|---|
| M1 | A 组完成 | 循环依赖会在启动时报错；渲染系统编译期无法回写 ECS |
| M2 | B 组完成 | 手工写 JSON 场景 → 加载出画面；存盘再读一字不差 |
| M3 | C 组完成 | 1080p 下 10,000 精灵 60FPS；HUD 显示 DrawCall |
| M4 | v0.2.0 | **在编辑器里摆出一个关卡并试玩**（第一次真正"可用"） |
| M5 | v0.3.0 | 1,500 实体 60FPS + 全 C# 逻辑 |
| M6 | v0.4.0 | 整个游戏界面在 UI 编辑器里搭出来 |
| M7 | v1.0.0 | 一个能分发的小战棋游戏 + GIF + 性能报告 |

### 12.4 R3a 必须提前

脚本系统要在 v0.1.0 就用上，所以拆成两步：

| 步骤 | 内容 | 服务于 |
|---|---|---|
| **R3a 最小脚本**（3~5 天） | `IScriptEngine` + 宿主 API 表 + CoreCLR 加载 + `Update` 回调 + **热重载（含 ALC 卸载断言）** | G1 |
| **R3b 完整脚本**（1~2 周） | 组件访问、生命周期全套、异常隔离、`fields` 编辑、性能优化 | G2~G4 |

**R3a 紧跟渲染之后，不要等编辑器做完** —— 它是**风险最高的模块**，越早证伪越好。

---

## 13. 风险登记册

| # | 风险 | 影响 | 概率 | 缓解 |
|---|---|---|---|---|
| K1 | **C# 热重载需求落空** | 高 | **已确认为硬约束** | 已核实 NativeAOT 不支持卸载（§9.2）→ 必须走 CoreCLR + 可卸载 ALC。**R5 前先做 3 天 spike**：跑通"加载→卸载→重载 50 次无泄漏"再投正式开发。若失败 → 改 Lua 或"脚本宿主子进程"方案 |
| K1b | CoreCLR 路线**只能框架依赖部署**，不能自带运行时 | 中 | 已确认 | 发布期改走 NativeAOT（编辑器 CoreCLR + 发布 AOT 双路径）；R7 前必须做干净机验证 |
| K2 | **编辑器工作量失控**（面板无穷无尽） | 高 | 高 | 严格按 §8.2 的 P0/P1/P2 分级；P0 之外一律不做；M4 之后才允许碰 P1 |
| K3 | **序列化格式反复改** | 中 | 中 | B 组就把"版本号 + 迁移链 + 未知组件保留"做对；先用 8 个组件验证覆盖度 |
| K4 | **物理越做越复杂** | 中 | 中高 | 明确**不做**：软体/关节/连续碰撞/旋转摩擦精确解。只用"半隐式欧拉 + 冲量法 + SAT" |
| K5 | **性能优化过早** | 中 | 中 | 先出数字（测试基准），**对着数字优化**；C 组之后再谈批处理/多线程 |
| K6 | 脚本 C++/C# **结构体布局不一致** → 随机内存错误 | 高（极难调试） | 中 | `static_assert` 锁死 + 启动时尺寸自检（§9.5） |
| K7 | **只有你一个人**，生病/忙碌就停摆 | 中 | 高 | 每阶段收敛到"可提交、可运行"；不留半成品分支；用本文档接续上下文 |
| K8 | 引擎没有"用户"，**失去方向** | 中 | 中 | 用 §7 的游戏阶梯做北极星；每个里程碑产出可演示 GIF |
| K9 | 渲染抽象层重构与 C 组冲突 | 低 | 中 | 先按 §6.6 决定"诚实降级"，一次性做掉，C 组只做能力不做搬迁 |
| K10 | 打包/运行时分发踩坑（.NET 运行时、VC++ 运行库） | 中 | 中 | R7 前至少做一次完整干净机验证，别留到最后 |

---

## 14. 学习地图：每个问题对应什么 C++ / 引擎知识

> 沿用原 roadmap 的宗旨：**每修一个 bug 都要想清它对应的知识点**。
> ⚠️ 原文档的行号引用已全部过期（例如它说的 `Renderer2D.cpp:150` 现在是一个 `{`），所以下面**按内容索引**，不按行号。

| 知识点 | 对应问题 | 状态 |
|---|---|---|
| **UB 与类型双关** | `CircleVertices` 曾用 `new QuadVertex[]`（类型布局不同） | ✅ 已消除（circle 块已清空） |
| **缓冲区溢出与防御式编程** | Quad 索引未初始化、顶点缓冲尺寸错误、精灵超限无检查 | ❌ 仍在（P0-1~P0-3） |
| **断言的语义**（失败必须停止） | `BAMBOO_ASSERT` 只打日志不中断 —— 比"条件写反"更严重 | ❌ 仍在（P0-4） |
| **空指针与不变量** | 空 `Entity` 一用就崩、`Camera::SetViewportSize` 除零 | ❌ 仍在（P0-5、P1-6） |
| **悬垂指针 / 迭代器失效** | `GetMainCamera` 返回内部地址、`TextureSlotMap` 遍历中被清空 | ❌ 仍在（P0-6、P0-7） |
| **类内成员初始化器（NSDMI）** | 大量成员无初值（`RigidbodyComponent` 全部字段等） | ❌ 仍在（P0-8） |
| **静态初始化顺序（SIOF）** | `RendererCommand::s_RendererAPI` 在 `main` 前构造 | ❌ 仍在（P2-10） |
| **虚析构与多态基类** | `~Application()` 不是 virtual，`BreakoutApp` 继承它 | ❌ 仍在（P2-11） |
| **`friend` 与 `static` 不能同用** | `IntVector2.h` 的四个友元运算符 —— **C++20 才报错** | ✅ 已修（2026-09-22） |
| **`const` 正确性** | `DrawSprite(Ref<Texture2D>&)` 非 const 引用 | ❌ 仍在（P1-9） |
| **行主序 vs 列主序** | `Matrix3` 内部约定自相矛盾（`operator()` 与 `operator*` 不一致） | ❌ 仍在（P1-1）—— 最容易出错的数学 bug |
| **角度 vs 弧度** | `Matrix4::RotateXYZ` 直接对角度求 cos | ❌ 仍在（P1-2） |
| **除零 / inf / NaN 传播** | `Vector2::operator/` 无检查；`Camera` 除零守卫失效 | ❌ 仍在（P1-5、P1-6） |
| **重载决议与隐式转换** | `Color(int a,int r,int g,int b)` 的 ARGB 顺序 + int/float 重载 | ❌ 仍在（P1-4） |
| **ODR / 单一定义** | `STB_IMAGE_IMPLEMENTATION` 曾重复定义 | ✅ 已修 |
| **循环包含与前向声明边界** | `Entity.h` ↔ `Scene.h` | ❌ 仍在（P2-1） |
| **隐藏依赖（传递包含）** | `std::sort` 未 include `<algorithm>` | ❌ 仍在（P0-10） |
| **所有权与 RAII** | 三套所有权并存；`Renderer2D` 用裸 `new[]` + 显式 Shutdown | ❌ 仍在（§4.3） |
| **线程安全** | `AssetManager::AsyncLoad` 的 `detach` + 悬垂 `this` | ❌ 仍在（P2-8） |
| **`#pragma once`** | `Singleton.h`、`SceneCamera.h` 曾缺失 | ✅ 已修 |
| **编码与 `/utf-8`** | `PlatformWindows.h` 曾是 GBK，在 `/utf-8` 下报 C4819 | ✅ 已修 |
| **`C4312` 指针宽度转换** | 顶点属性偏移 `(void*)uint32_t` | ✅ 已修 |
| **架构：唯一权威** | 执行顺序/生命周期/资源身份/数据模式/渲染契约（§4） | ❌ 未开始 |
| **数据驱动与反射思想** | `ComponentRegistry`（一处声明、四处受益） | ❌ 未开始 |
| **依赖倒置与分层** | `Graphics` 依赖 `Game`；`ECS` 依赖 `Graphics` | ❌ 仍在 |

**沉淀**：为每个 ❌ 项补一条回归测试（§10.2），把知识点固化在测试里而不是记忆里。

---

## 15. 开发约定

- **注释与提交信息用中文**（与既有代码一致）。
- **注释约定**（判据：**注释要少但可信。错误注释比没有注释更糟——它让人相信谎言**）：
  - **该写**：非直觉的约定（坐标系、矩阵存储序、角度制）、不变量与前置条件、为什么这么写、**带编号的已知缺陷**（`见 P1-6`）、具体可归属的 `TODO(模块): ...`
  - **不该写**：复述代码（`GetWidth()` → `// 获取宽度`）、**注释掉的代码**（旧实现删掉，`git log` 里有）、未填的占位符、与实现矛盾的 `@param`
- **构建输出不入库**：`Source/build/` 已在 `.gitignore` 中。
- **资源必须入库**：`Source/BambooAssets/` 下的着色器与贴图是运行必需的。
- **不要修改 `Source/ThirdParty/`**；加新依赖先在 `ThirdParty/CMakeLists.txt` 注册。
- **提交信息**用 `类型(模块): 描述`，例如 `fix(renderer): 修复 Quad 索引缓冲未初始化`。
- **分层约束**：新代码不要让渲染层依赖应用层（见 §6.2）；CI 用 `tools/check_architecture.ps1` 检查。

---

## 16. 待决策事项

| # | 决策点 | 选项 | 建议 |
|---|---|---|---|
| D1 | **图形抽象层怎么处理** | A 诚实降级（合并 `GraphicsAPI/OpenGL` 进 `Graphics`，删空转抽象，代码 −30%）／B 兑现抽象（所有 `Create` 走 `GetAPI()`，补 `Null` 后端用于测试） | **A**。当前没有第二个后端的真实需求，抽象成本 > 收益 |
| D2 | **序列化格式** | A 自写极简 JSON（~600 行，完全掌控）／B vendored `nlohmann/json` 单头 | **B**。序列化不是学习重点，自写 JSON 的边界情况（转义/精度/UTF-8）会吃掉大量时间 |
| D3 | **物理自己写还是接库** | A 自己写（半隐式欧拉 + 冲量法 + SAT）／B 接 Box2D | **A**（与学习目标一致）。若目标是"尽快做出游戏"则 B |
| D4 | **空间分区** | A 均匀网格 / SpatialHash／B 四叉树/八叉树 | **A**。2D 下网格简单高效，并删除空的 `Octree.h` |
| D5 | **UI 系统** | A 并入 ECS（重写，删现有 `UI/`）／B 保留独立体系补齐 | **A**。短期可先加 `BAMBOO_BUILD_UI` 开关移出编译，中期重做 |
| D6 | **ImGui Docking** | A 换 docking 分支（~1 小时）／B 保持 master 手动固定布局 | **A**。编辑器有 6+ 面板，没有停靠几乎无法使用 |
| D7 | **`Editor` 投入时机** | A 先做游戏侧／B 编辑器优先 | 目标是"做出能玩的游戏" → **A**；目标是"理解引擎架构" → **B** |
| D8 | **第三方大件** | 是否引入 glm（数学）等 | 按需。`Math` 的 P1 级 bug 修完再决定是否替换 |
| D9 | **多线程资源加载** | A 保留并修好 `AsyncLoad`／B 暂时移除异步接口 | **B**（短期）。异步复杂度现在换不来收益，且当前实现是错的 |
| D10 | **.NET 目标框架** | A 现在装 .NET 10 用 `net10.0`／B 先用本机 `net9.0` 开工 | **A**（LTS，支持到 2028-11）。`net9.0` 也能跑通 spike，但命名 **2026-11-10 EOL**，发布前必须升 |
| D11 | **C# 还是先 Lua 验证边界** | A 直接上 C#（已确认）／B 先用 Lua 花 1~2 天验证边界设计 | **可选做 B**。边界设计占脚本系统难度的 1/3，Lua 原型成本极低；但不阻塞直接上 C# |
