# Bamboo 引擎架构（现状）

> **本文档描述"代码现在是什么样"**，不描述"应该是什么样"。
> - 想改什么 / 缺陷清单 → `refactor_plan.md`
> - 想按什么顺序建 → `upgrade_plan.md`
> - 做哪些游戏、编辑器怎么做 → `games_and_editor.md`
> - 学习路线 → `roadmap.md`
>
> 最后核对：**2026-09-18**（对照 `Source/Bamboo` 实际代码逐项确认）。
> **维护约定**：目录结构、执行顺序、依赖方向发生变化时必须更新本文档；否则请删掉对应小节，**不要留下与代码不符的描述**。

---

## 1. 项目构成

| Target | 类型 | 位置 | 说明 |
|---|---|---|---|
| `Bamboo` | 静态库 | `Source/Bamboo/` | 引擎核心 |
| `Sandbox` | 可执行 | `Source/Sandbox/` | 示例游戏（Breakout） |
| `Editor` | 可执行 | `Source/Editor/` | 编辑器（**当前是空壳，绕过引擎自建窗口，ImGui 未接 GL3 后端**） |
| ThirdParty | 混合 | `Source/ThirdParty/` | GLFW / glad / entt / spdlog / stb / imgui（全部 vendored） |

- 语言标准：**C++17**，Windows + MSVC（VS 2022）only。
- 构建：CMake 3.20，源文件用 `file(GLOB_RECURSE)` 收集（**新增文件后需要重新 configure**）。
- 资源根：`Source/BambooAssets/`，由 CMake `configure_file` 生成 `build/generated/Config.h`，把**绝对路径**烤进 `BAMBOO_ASSET_ROOT`（**换目录后必须重新 configure**）。
- 详细构建命令见 `AGENTS.md`。

---

## 2. 分层与依赖方向

```
┌──────────────────────────────────────────────┐
│ Game / Editor / Sandbox    应用层            │
│   Application · Window                       │
├──────────────────────────────────────────────┤
│ Scene                      场景与系统编排层   │
│   Scene · SceneManager · SystemRegistry      │
├──────────────────────────────────────────────┤
│ ECS                        数据与逻辑层       │
│   Entity · Component/* · System/*            │
├──────────────────────────────────────────────┤
│ Graphics / GraphicsAPI     渲染层            │
│   Renderer2D · Camera · Shader/Texture/...   │
│   GraphicsAPI/OpenGL/*     后端实现           │
├──────────────────────────────────────────────┤
│ Physics · Assets · UI      领域层             │
├──────────────────────────────────────────────┤
│ Platform/Windows           平台层             │
│   WindowsWindow · WindowsInput               │
├──────────────────────────────────────────────┤
│ Core                       基础层（无引擎内依赖）│
│   Ref · Log · Assert · Time · UUID · Input   │
│   KeyCodes · Base · FileSystem/FileUtlis     │
└──────────────────────────────────────────────┘
```

### ⚠️ 当前存在的反向依赖（**属于已知缺陷，不要模仿**）

| 违规 | 位置 | 为什么是问题 |
|---|---|---|
| 渲染层依赖应用层 | `Graphics/Texture.cpp` 里调 `Application::GetInstance()->GetAssetManager()` | 分层倒置；`Graphics` 不应知道 `Game` 的存在。而且三层解引用，任一层为空即崩 |
| 组件依赖渲染资源 | `SpriteRendererComponent` 直接持有 `Ref<Texture2D>` | GPU 句柄不可序列化；应改为 `AssetHandle`（见 `refactor_plan.md` §4.3） |
| 头文件包含风格混乱 | 同文件里混用 `"../Bamboo/Core/Log.h"` / `"../Core/Log.h"` / `"Bamboo/Math/Color.h"` / `"./Core/Ref.h"` | 文件移动成本高；`Assets/AssetFactory.h` 里还有小写 `core`（Windows 侥幸通过，跨平台必炸） |

---

## 3. 启动与帧循环

### 3.1 启动顺序（`Game/Application.cpp`）

```
Application::Application(name)
  ├─ Log::Init()                       // 只初始化 CoreLogger；ClientLogger 仍为 nullptr
  ├─ Window::Create({name})            // 创建 GLFW 窗口 + OpenGL 上下文
  ├─ Window::SetEventCallback(...)
  ├─ SceneManager()                    // 构造内部 LoadScene() → CreateRef<Scene>()
  ├─ AssetManager()
  ├─ Renderer2D::Init()                // 创建 VAO/VBO/Shader/白纹理（需要 GL 上下文）
  └─ Renderer::Init()                  // → RendererCommand::Init() → glEnable(GL_BLEND) 等
```

**注意两个已知问题**：
1. `RendererCommand::s_RendererAPI` 是**静态成员**、靠 `RendererAPI::Create()` 在 `main` 之前构造（静态初始化顺序隐患）。
2. `Time::Initialize()` 是在 `Run()` 里才调的，晚于上面所有步骤。

### 3.2 帧循环（`Application::Run`）

```
while (IsRunning())
  ├─ Time::Update()                       // 计算 deltaTime（⚠️ 无上限钳制）
  ├─ SceneManager::GetActiveScene()->Update(dt)
  └─ Window::Update()                     // glfwPollEvents() + SwapBuffers()
```

**注意**：轮询事件在场景更新**之后**（"事件比逻辑晚一帧"）。输入能工作是因为 `Input` 用 `glfwGetKey` 查询即时状态，而不是靠事件。

### 3.3 系统执行顺序（`Scene::Update` → `SystemRegistry`）

`Scene` 不再直接持有系统列表，而是委托给 `SystemRegistry`，**按阶段分桶执行**：

```
Scene::Update(dt)
  ├─ m_SystemRegistry.UpdateLogic(registry, dt)      // SystemPhase::Logic
  ├─ m_SystemRegistry.UpdateTransform(registry, dt)  // SystemPhase::Transform
  ├─ m_SystemRegistry.UpdatePhysics(registry, dt)    // SystemPhase::Physics
  └─ m_SystemRegistry.UpdateRender(registry, dt)     // SystemPhase::Render
```

| 系统 | `GetPhase()` 返回 | 落入的桶 |
|---|---|---|
| `TransformSystem` | `Transform` | Transform |
| `CameraSystem` | **未 override → 默认 `Logic`** | Logic |
| `Physics::PhysicsSystem` | `Physics` | Physics |
| `RendererSystem` | `Render` | Render |

- `SystemRegistry::Register<T>()` 会在注册时调用 `system->Init()`。
- **同一阶段内按注册顺序执行**（暂无优先级机制）。
- `Scene::AddSystem<T>()` 与内部注册走**同一条 phase 路由**（用户可以自己声明阶段）。

**已知问题**：
- 顺序仍然靠"注册顺序"这种隐式约定（`refactor_plan.md` §3.1）。
- `CameraSystem` 落在 `Logic` 桶，靠"Logic 早于 Transform"才是错的——实际上 Transform 先跑，所以它读到的是本帧的 Transform（**当前正确**，但依赖这个顺序关系，属于脆弱耦合，建议显式声明阶段）。
- `PhysicsSystem::Update` 目前是**空实现**（物理尚未接入）。

---

## 4. ECS 使用方式

### 4.1 实体

```cpp
Entity e = scene.CreateEntity("Player");     // 自动附加 IDComponent + TransformComponent + TagComponent
e.AddComponent<SpriteRendererComponent>();
auto& t = e.GetComponent<TransformComponent>();
```

`CreateEntityWithUUID` 会同时写入 `Scene::m_EntityMap`。

**已知问题**：`Entity` 默认构造后 `m_Scene == nullptr`，但所有组件操作都无条件解引用它 → 调用 `FindEntityByName` 失败返回的空实体就会崩（`refactor_plan.md` P0-6）。

### 4.2 组件清单（实际存在）

| 组件 | 头文件 | 字段要点 |
|---|---|---|
| `TransformComponent` | `ECS/Component/TransformComponent.h` | `Position/Rotation/Scale`、`Dirty`、`LocalMatrix`、`WorldMatrix` |
| `SpriteRendererComponent` | 同上目录 | `SpriteColor`、`Ref<Texture2D>`、`Size`、`ZOrder`、`Opacity`、`Visible` |
| `TriangleComponent` / `QuadComponent` | 同上 | 颜色（+ `Size`，**但渲染器忽略它**） |
| `CameraComponent` | 同上 | 按值持有 `Camera`、`Primary` 标记 |
| `TagComponent` / `IDComponent` | 同上 | 名字 / UUID |
| `RigidbodyComponent` | 同上 | `Type/Mass/Force/Torque/Impulse/Friction`（**全无初值**） |
| `BoxCollider2DComponent` / `CircleColliderComponent` | 同上 | 尺寸/偏移/摩擦/触发器（**默认构造不初始化**） |
| `Shape/CircleComponent` / `Shape/RectangleComponent` | `ECS/Component/Shape/` | 形状参数（**尚未被任何系统使用**） |

**注意**：`Component.h` 这个"总入口"头**只包含前 7 个**，不含 collider / rigidbody / shape —— 用到的文件需要单独 include。

### 4.3 系统接口

```cpp
enum class SystemPhase { Logic, Render, Physics, Transform };

class ISystem {
public:
    virtual void Init() {}
    virtual void Update(entt::registry& registry, float deltaTime) = 0;
    virtual SystemPhase GetPhase() const { return SystemPhase::Logic; }
    virtual ~ISystem() = default;
};
```

**约定**：新系统继承 `ISystem`，override `GetPhase()` 声明自己的阶段；`Update` 里从 `registry.view<...>()` 取数据。

---

## 5. 渲染管线（现状）

```
RendererSystem::Update(registry, dt)
  ├─ RendererCommand::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f})   // 硬编码清屏色
  ├─ RendererCommand::Clear()
  ├─ registry.view<CameraComponent, TransformComponent>()        // 找相机（取最后一个）
  ├─ Renderer2D::BeginScene(camera)  或  BeginScene()            // 无相机时用单位阵
  ├─ view<TriangleComponent>  → Renderer2D::DrawTriangle(position, color)
  ├─ view<QuadComponent>      → Renderer2D::DrawQuad(position, {100,100}, color)   // size 硬编码
  ├─ view<SpriteRendererComponent> → 按 ZOrder 排序 → DrawSprite(worldMatrix * Scale(size), color, tex)
  └─ Renderer2D::EndScene()                                      // → Flush()
```

- `Renderer2D` 内部为**4 种图元各维护一套独立的** `{VertexArray, VertexBuffer, Shader, *Vertices, *VerticesPtr, IndexCount}`（共 24 个散落成员）。
- 相机视图矩阵由 `CameraSystem` 通过 `Camera::SetView()` 灌入（`Camera` 自己不知道位置）。
- 纹理槽上限 32，精灵上限 100（`MaxSpriteCount`）。

**已知问题（详见 `refactor_plan.md` P0-1~P0-4、P1-7~P1-10）**：
- Quad 索引缓冲用未初始化数据上传（`IndexBuffer::Create(quadIndices, 2)`，而生成循环因 `QuadIndexCount == 0` 从未执行）。
- `DrawQuad` / `DrawTriangle` **忽略传入的 position/size**，写死顶点。
- `DrawTriangle` 硬编码 `/1280`、`/720`。
- 顶点缓冲按"1 个顶点"大小创建，靠 `SetData` 里的 `glBufferData` 重新分配掩盖。
- 渲染期回写 ECS（`sprite->SpriteTexture = GetNormalTexture()`）。
- 圆（Circle）分支**尚未实现**（`Renderer2D.cpp` 的 circle 块为空）。

---

## 6. 所有权模型

| 表达 | 使用位置 |
|---|---|
| `Scope<T>` = `std::unique_ptr` | `Window`、`SceneManager`、`AssetManager`、`ISystem`、`RendererAPI`、`PhysicsWorld`、`GraphicsContext` |
| `Ref<T>` = `std::shared_ptr` | `Texture`、`Shader`、`VertexArray`、`VertexBuffer`、`IndexBuffer`、`UniformBuffer`、`Asset`、`UIElement`、`UIComponent` |
| 裸 `new[]` / `delete[]` | `Renderer2D` 的 `TriangleVertices`/`QuadVertices`/`SpriteVertices`/`CircleVertices`，由 `Renderer2D::Shutdown()` 释放（**漏了 `CircleVertices`**） |

**约定**：新代码优先 `Scope<T>` + `CreateScope<T>()`；每帧临时数据用 `std::vector` 复用；**不要新增裸 `new[]`**。

---

## 7. 关键不变量（改动时必须保持）

1. **世界单位 = 1 像素**（像素相机）；`Camera::SetViewportSize` 决定可视范围。
2. **实体必须由 `Scene::CreateEntity` 创建**（否则 `Entity::m_Scene` 为空）。
3. **系统执行顺序由 `SystemPhase` 决定**；新系统必须 override `GetPhase()`，否则默认落 `Logic`。
4. **`Script`（未来的脚本组件）以外的逻辑不应写在游戏代码里**——`upgrade_plan.md` 的目标是游戏逻辑全部在 C# 里。
5. **渲染系统只读 registry**（当前违反了，见 `refactor_plan.md` P0-8；修好后要当成硬约束）。
6. **资源路径依赖编译期烤入的绝对路径**——移动仓库后必须重新 configure。
7. **新增源文件后必须重新 configure**（`file(GLOB_RECURSE)` 不会自动感知）。

---

## 8. 目录结构（实际）

```
Source/
├── CMakeLists.txt
├── Bamboo/                     # 引擎静态库
│   ├── Bamboo.h  Config.in.h  CMakeLists.txt
│   ├── Core/                   # Ref Log Assert Time UUID Input KeyCodes Base
│   │   └── FileSystem/         # FileUtlis.{h,cpp}
│   ├── Event/                  # Event.h ApplicationEvent.h KeyEvent.h
│   ├── Math/                   # Vector2/3/4 Matrix3/4 Color AABB Rect
│   │                           # IntVector2 Quaternion Random Math MathDefs
│   ├── ECS/
│   │   ├── Entity.{h,cpp}
│   │   ├── SystemRegistry.{h,cpp}      ← 阶段化系统注册表
│   │   ├── Component/          # 7 个通用组件 + Shape/ + Rigidbody/Collider
│   │   └── System/             # ISystem + Transform/Camera/RendererSystem
│   ├── Scene/                  # Scene SceneManager SceneSerializer(空) SceneCamera(空) Octree(空)
│   ├── Graphics/               # Renderer2D Camera RendererCommand Renderer
│   │                           # Shader Texture VertexArray RenderBuffer
│   │                           # UniformBuffer RendererAPI GraphicsContext
│   ├── GraphicsAPI/OpenGL/     # OpenGL{Shader,Texture,VertexArray,Buffer,
│   │                           #   Context,RendererAPI,UniformBuffer}
│   ├── Physics/                # PhysicsSystem PhysicsWorld(空) PhysicsDefine
│   ├── Assets/                 # Asset AssetManager AssetFactory ImageAsset
│   ├── UI/                     # UIElement Canvas Button Text UIComponent（空壳）
│   ├── Game/                   # Application Window
│   └── Platform/Windows/       # WindowsWindow WindowsInput PlatformWindows
├── BambooAssets/               # 资源（Shaders/ Texture2d/）
├── Sandbox/                    # 示例游戏
│   ├── main.cpp
│   └── BreakoutDemo/           # BreakoutApp + System/ + Component/
├── Editor/                     # 编辑器（空壳）
└── ThirdParty/                 # GLFW glad entt spdlog stb imgui
```

**注意**：这个结构是**当前**的，目标结构（`Render/`、`Asset/`、`Systems/` 等）见 `upgrade_plan.md` §6.2。

---

## 9. 本文档取代了什么

本文档取代了已删除的 `docs/project_structure.md`。被删除的那份文档存在以下过时内容（保留此清单是为了让读过它的人知道哪些结论作废）：

| 过时内容 | 现状 |
|---|---|
| 描述了 `SpriteRendererSystem`（含其头文件、类表、调用关系） | **已删除**，代码中不存在 |
| 声称 `CameraSystem` 负责"计算世界坐标并设置 `LocalToWorldMatrix`" | 代码中 `CameraSystem` 只做视图矩阵，不碰精灵矩阵 |
| 声称 `RendererSystem` 按 `ZOrder` 排序后绘制精灵 | 仍成立（这部分没错） |
| 目录树缺少 `SystemRegistry`、`Core/FileSystem/`、`ECS/Component/Shape/`、`Platform/Windows/`、`circle.*` 着色器 | 已补进 §8 |
| 把 `Vector4`、`Octree`、`SceneCamera` 描述为已实现的功能 | `Vector4.cpp` 是 **0 字节空文件**；`Octree.h` 是空类；`SceneCamera` 无人使用 |
| 描述了 `Scene::m_Systems` / `m_LogicSystems` 两个容器 | 已被 `SystemRegistry` 的四个阶段桶取代 |
| 描述 `Scene::Update` 的循环顺序 | 已改为 `SystemRegistry` 的四次调用（§3.3） |

**另外**：`roadmap.md` 里引用的**文件:行号已过期**（例如它说的 `Renderer2D.cpp:150` 现在是 `{`），但其中描述的 bug **大部分仍然存在**——请以 `refactor_plan.md` 的编号（P0-x/P1-x）为准。
