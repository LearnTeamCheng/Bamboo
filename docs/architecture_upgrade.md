# Bamboo 架构问题与目标架构（架构决策文档）

> **这份文档和 `upgrade_plan.md` 的区别**：
> - `upgrade_plan.md` 回答"**做什么模块、按什么顺序做**"
> - 本文档回答"**现在这个架构到底哪里不对、要变成什么样、为什么**"
>
> 之前缺的就是后者。结果是：37 项缺陷和 122 行死代码被当成"一堆待修 bug"，但它们是**同一个架构缺陷的 37 种症状**。修完症状，架构不动，新代码会继续长成同样的形状。
>
> 每一项都标注了代码证据（`文件:行`），核对于 **2026-09-18**。

---

## 目录

- [0. 一句话诊断](#0-一句话诊断)
- [1. 架构问题 1：没有"执行顺序"的权威](#1-架构问题-1没有执行顺序的权威)
- [2. 架构问题 2：没有"对象身份与生命周期"的权威](#2-架构问题-2没有对象身份与生命周期的权威)
- [3. 架构问题 3：没有"资源身份"的权威](#3-架构问题-3没有资源身份的权威)
- [4. 架构问题 4：没有"数据模式"的权威](#4-架构问题-4没有数据模式的权威)
- [5. 架构问题 5：没有"渲染契约"](#5-架构问题-5没有渲染契约)
- [6. 六个被推迟的架构问题](#6-六个被推迟的架构问题按优先级)
- [7. 目标架构](#7-目标架构)
- [8. 迁移路径（不是重写）](#8-迁移路径不是重写)
- [9. 架构守护：让退化无法通过](#9-架构守护让退化无法通过)
- [10. 什么不做（防止过度设计）](#10-什么不做防止过度设计)
- [11. 与其它文档的关系](#11-与其它文档的关系)

---

## 0. 一句话诊断

**当前引擎不是"缺功能"，而是缺四个权威（authority）。**

一个系统只要有 3 个以上模块，就必须为下面四件事指定**唯一权威**。否则每个模块都会自己形成一套局部约定，任意两两之间就能出错——bug 数量随模块数**平方增长**。

| 缺的权威 | 后果 | 证据 |
|---|---|---|
| **执行顺序** | 顺序靠隐式约定；改一处崩一处 | §1 |
| **对象身份与生命周期** | 悬垂指针、空句柄崩溃、谁负责销毁不明 | §2 |
| **资源身份** | 只能用路径字符串指代资源；改名断链 | §3 |
| **数据模式** | 字段增删无法被序列化器/编辑器感知 | §4 |
| （第五个）**渲染契约** | 渲染器既不知道要画什么，也不知道能读什么 | §5 |

**这是"37 个缺陷"的真正来源。** 例如：
- `CameraSystem` 落在 `Logic` 桶、(P1-13) 视图矩阵手写"取负角度求逆" —— 本质是**没有执行顺序权威**（谁在谁之前跑没有声明）。
- `Entity` 默认构造后一用就崩 (P0-6)、`Scene::GetMainCamera` 返回悬垂指针 (P0-7) —— 本质是**没有生命周期权威**。
- `SceneSerializer` 是空的、`Ref<Texture2D>` 存在组件里 (P4.3) —— 本质是**没有数据模式权威**。
- 渲染期回写 ECS (P0-8)、`DrawQuad` 忽略参数 (P1-8) —— 本质是**没有渲染契约**。

**所以修 bug ≠ 升级架构。** 本文档给出的是后者。

---

## 1. 架构问题 1：没有"执行顺序"的权威

### 1.1 现状

`SystemRegistry` 看起来是"阶段化"的，但**阶段集合是硬编码的封闭枚举**，而且顺序声明在别处。

**代码证据**：

```cpp
// ISystem.h:6-12 —— 阶段列表写死在枚举里
enum class SystemPhase { Logic, Render, Physics, Transform };

// SystemRegistry.h:19-22 —— 每个阶段一个独立容器
std::vector<Scope<ISystem>> m_LogicSystems;
std::vector<Scope<ISystem>> m_PhysicsSystems;
std::vector<Scope<ISystem>> m_RenderSystems;
std::vector<Scope<ISystem>> m_TransformSystems;

// SystemRegistry.h:13-16 + SystemRegistry.cpp:4-34 —— 每个阶段一个独立 Update 函数
void UpdateLogic(...); void UpdatePhysics(...); void UpdateRender(...); void UpdateTransform(...);

// Scene.cpp:39-42 —— 阶段的**先后顺序**只存在于这里
m_SystemRegistry.UpdateLogic(m_Registry, deltaTime);
m_SystemRegistry.UpdateTransform(m_Registry, deltaTime);
m_SystemRegistry.UpdatePhysics(m_Registry, deltaTime);
m_SystemRegistry.UpdateRender(m_Registry, deltaTime);
```

**加一个新阶段（比如 `PreUpdate` 或 `UI`）需要改 4 处**：枚举、容器声明、Update 声明、Update 实现。这就是"封闭集合"的代价——**引擎的骨架每加一个阶段就要被剪开一次**。

### 1.2 更深的问题：阶段的先后关系没有任何地方声明

`Logic` 必须在 `Transform` 之前跑吗？`CameraSystem` 落在 `Logic` 桶，它**读** `TransformComponent`（为了算视图矩阵）——所以它必须在 `Transform` 之后才对。

**但这条依赖关系只存在于 `Scene::Update` 那 4 行的书写顺序里。** 全仓库搜索 `DependsOn`/`After`/`Priority`/拓扑排序：**零结果**。

于是你得到的是一个**三重隐式**：
1. 阶段间的顺序 → `Scene::Update` 的书写顺序
2. 同阶段内的顺序 → `Register<T>()` 的调用顺序
3. 跨阶段的读写依赖 → 没有任何地方记录（只能靠人记住）

`CameraSystem` 现在"恰好"是对的（Transform 桶在 Logic 桶之前跑）。**但这是巧合，不是设计**：任何人把 `CameraSystem` 的 `GetPhase()` 改成 `Transform` 都会立刻得到一帧延迟的相机——而且不会有任何警告。

### 1.3 要升级成什么样

**权威归属：由"注册表 + 显式声明"决定，不由书写顺序决定。**

#### (a) 阶段变成"开放的有序列表"

```cpp
// 阶段不再是编译期封闭枚举，而是启动时注册的有序列表
// 新增阶段 = 在列表里插一个位置，不需要改 SystemRegistry 的骨架
enum class BuiltinPhase { PreUpdate, Update, PostUpdate, PreRender, Render };

class SystemRegistry {
public:
    // 内置阶段提供默认顺序；引擎/游戏都能插入自定义阶段
    void RegisterPhase(std::string_view name, int order);   // 小 order 先跑
    template <typename T, typename... Args> T& Register(Args&&...);

    void Update(entt::registry&, float dt);   // ← 单一入口，不再有 4 个 Update
private:
    struct Phase { std::string Name; int Order; std::vector<Scope<ISystem>> Systems; };
    std::vector<Phase> m_Phases;              // 按 Order 排序
};
```

`Scene::Update` 只调一次 `m_SystemRegistry.Update(...)`。**顺序知识从 `Scene` 移回注册表**，这是它该在的地方。

#### (b) 依赖关系显式声明 + 拓扑排序（关键升级）

```cpp
class ISystem {
public:
    virtual std::string_view GetName() const = 0;      // 必须命名 —— 依赖和调试都要用
    virtual SystemPhaseInfo  GetPhase() const;         // { 阶段名, 阶段内顺序 }
    virtual std::vector<std::string_view> RunsAfter()  const { return {}; }
    // ⚠️ 不要提供 RunsBefore()：它会让依赖关系变成双向，无法拓扑排序
    virtual void OnInit(Scene&) {}
    virtual void OnUpdate(Scene&, float dt) = 0;
};
```

注册表在 `Register` 完成后做一次**拓扑排序**，检测环并**启动时报错**（而不是运行时给出随机顺序）。

这解决了 §1.2 的三重隐式：

| 原来的隐式 | 升级后 |
|---|---|
| 阶段顺序靠 `Scene::Update` 书写顺序 | `RegisterPhase` 显式声明 order |
| 同阶段顺序靠注册顺序 | `GetPhase()` 返回阶段内 order |
| 读写依赖无人记录 | `RunsAfter()` 显式声明 + 启动时校验 |

#### (c) 读写权限进入类型系统（这条最关键）

目前 `Update(entt::registry&, float)` —— **写权限是默认的**，渲染系统也拿到了可写引用，于是它回写了 ECS (P0-8)。

```cpp
// 可写的系统（游戏逻辑、物理）
class ISystem { virtual void OnUpdate(Scene&, float) = 0; };

// 只读的视图系统（渲染、调试绘制、统计）
class IRenderSystem {
    virtual void OnRender(const Scene&, Renderer2D&) = 0;   // const Scene& ← 编译器强制只读
};
```

**把"渲染只读"从注释约定变成编译错误。** 这一条不需要任何运行时开销，却永久消灭了一整类 bug。

### 1.4 为什么这值得现在做

因为**它决定后面所有系统的写法**。如果等写完物理、UI、脚本系统再做，就有 8~10 个系统要跟着改。现在只有 4 个。

### 1.5 顺带修掉的症状

| 缺陷编号 | 症状 | 根因归属 |
|---|---|---|
| P2-2 | 系统被切成"内置/逻辑"两条流水线 | §1 |
| P1-13 | 相机视图矩阵手写求逆、`CameraSystem` 落在错的桶 | §1（依赖未声明） |
| P0-9 | `PhysicsSystem::Init` 从未被调用 | §1（生命周期钩子未定义） |
| P2-4 | 渲染系统找相机、硬编码清屏色 | §1 + §5 |
| §3.1 | "黑盒约定多于类型约束" | §1(c) |

---

## 2. 架构问题 2：没有"对象身份与生命周期"的权威

### 2.1 现状

**代码证据**：

```cpp
// Entity.h:16,74 —— Entity 持有 Scene 裸指针
Entity(entt::entity handle, Scene* scene);
Scene* m_Scene = nullptr;

// Entity.h:23,39,45,52 —— 所有组件操作无条件解引用它
T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, ...);

// Scene.h:48 —— 为了让 Entity 能访问，registry 被设为 public
entt::registry m_Registry;

// Entity.h:3 + Scene.h —— 于是头文件互相包含（循环依赖）
#include "../Scene/Scene.h"
```

**这里有三个纠缠在一起的问题**：

1. **`Entity` 是 `Scene` 的"视图"还是"所有者"？** 未定义。它存 `Scene*` 是因为**需要访问 `registry`**，而不是因为它在语义上属于某个场景。
2. **"无效实体"没有表示。** 默认构造的 `Entity` 是 `{entt::null, nullptr}`，但所有方法都假设它有效 → `FindEntityByName` 失败返回的空实体，一用就崩 (P0-6)。
3. **所有权与销毁责任不明。** `Scene::DestroyEntity` 先 destroy 再取 UUID (P2-12)；组件里 `Ref<Texture2D>` 的引用计数由 `shared_ptr` 顺手管理，没人知道纹理什么时候真正释放。

**这就是为什么封装必须是 public**：`Entity` 需要 `registry`，而 `registry` 属于 `Scene`，两者互不知道对方的边界，只能把内部暴露出来。

### 2.2 要升级成什么样

**权威归属：`entt::handle` —— "实体 + 它所属的 registry" 的官方打包类型。**

```cpp
class Entity {
public:
    Entity() = default;
    explicit Entity(entt::handle handle) : m_Handle(handle) {}

    bool IsValid() const { return m_Handle && m_Handle.registry()->valid(m_Handle.entity()); }
    operator bool() const { return IsValid(); }

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        BAMBOO_ASSERT(IsValid(), "AddComponent on invalid entity");
        return m_Handle.emplace_or_replace<T>(std::forward<Args>(args)...);
    }
    // 其余同理，全部先断言 IsValid()
private:
    entt::handle m_Handle;   // ← 不再需要 Scene*，不再需要 public registry，不再循环包含
};
```

**这一处改动同时解决四个问题**：

| 问题 | 如何被解决 |
|---|---|
| `Entity.h` ↔ `Scene.h` 循环包含 | `entt::handle` 只需要 `entt::registry` 的**前向声明**，不需要 `Scene.h` |
| `m_Registry` 必须 public | 不再需要——`handle` 自己带着 registry |
| 空实体用就崩 | `IsValid()` 有了明确语义；所有方法前置断言 |
| 悬垂实体（销毁后仍被引用） | `handle.registry()->valid()` 能检测（entt 支持实体版本号） |

**"谁负责销毁"也要有权威：**

```cpp
// 组件 = 纯数据，不持有资源所有权
struct SpriteRendererComponent {
    AssetHandle Texture;          // ← 句柄，不是 Ref<Texture2D>
    // ...
};

// 资源所有权归 AssetManager；实体销毁只是"少了一个引用"，不触发 GPU 释放
// 层级销毁：Scene::DestroyEntity 负责递归销毁子实体（一旦引入父子关系）
```

**关键原则：所有权只能有一个。** 现在 `Ref<T>` 的 `shared_ptr` 让所有权散落在所有持有者手里——这正是 §3 的问题。

### 2.3 顺带修掉的症状

| 缺陷编号 | 症状 |
|---|---|
| P0-6 | 空句柄解引用 |
| P0-7 | 相机指针悬垂（返回临时实体内部指针） |
| P2-1 | `Entity.h` / `Scene.h` 循环包含 + `m_Registry` 必须 public |
| P2-12 | `DestroyEntity` 先 destroy 再取 UUID；`m_EntityMap` 无自动清理 |
| §3.4 | 组件是"数据+行为+依赖"的混合体 |

---

## 3. 架构问题 3：没有"资源身份"的权威

### 3.1 现状

**代码证据**：

```cpp
// AssetManager.h:20,30,89 —— 资源的身份 = 路径字符串
Ref<T> Load(const std::string &path);
m_Assets[path] = asset;
std::unordered_map<std::string, Ref<Asset>> m_Assets;

// SpriteRendererComponent.h:11 —— 组件直接持有 GPU 对象
Ref<Texture2D> SpriteTexture;

// OpenGLShader.cpp:123 / OpenGLTexture.cpp:45 / ImageAsset.cpp:16 —— 路径拼接各写一遍
std::string(BAMBOO_ASSET_ROOT) + "/Shaders/" + fileName;
std::string(BAMBOO_ASSET_ROOT) + "/Texture2d/" + path;
```

**三个结构性问题**：

1. **身份 = 路径。** 改文件名 → 引用断链。所有引用资源的地方（组件、场景文件、着色器）都要跟着改。
2. **组件持有 GPU 对象。** `Ref<Texture2D>` 是 OpenGL 句柄的包装。这意味着：**含纹理的场景无法序列化**（这是 `SceneSerializer` 空着的根本原因之一，不只是"没写"）。同时它让组件依赖渲染层 → 分层被打破。
3. **资源生命周期无人负责。** `shared_ptr` 让"最后一个引用消失"决定释放时机——于是**在一帧中间释放**、在渲染进行中释放，都可能发生。且 `AssetManager::Unload` 和 `shared_ptr` 是**两套释放机制**（一个显式、一个自动），互相不知道对方。

### 3.2 要升级成什么样

**权威归属：`AssetDatabase`（GUID ↔ 路径）+ `AssetManager`（GUID → 实例 + 引用计数）。**

```cpp
// 1. 资源的稳定身份 = GUID，不是路径（路径只是"当前在哪"）
class AssetHandle {
    UUID m_Id{};
public:
    bool IsValid() const;
    UUID GetId() const;
};

// 2. .meta 文件承载 GUID —— 改名/移动不断链
//    Assets/Textures/player.png
//    Assets/Textures/player.png.meta  → { "guid": "a1b2...", "type": "Texture2D", "import": {...} }

// 3. 组件只存句柄（可序列化、不依赖渲染层）
struct SpriteRendererComponent {
    AssetHandle Texture{};        // ← 不再是 Ref<Texture2D>
    Color       Color = Color::White;
    // ...
};

// 4. 解析发生在渲染时，不发生在数据里
auto tex = assets.Get<Texture2D>(sprite.Texture);
if (!tex) { /* 占位纹理 + 一条警告 */ }
```

**生命周期的权威也要明确**：

| 机制 | 何时发生 | 由谁决定 |
|---|---|---|
| 引用计数减少 | 实体销毁、组件移除 | `AssetManager` 统一跟踪 |
| 真正释放 GPU 资源 | 引用计数 = 0 **且** 跨过一个安全点（帧末） | `AssetManager::CollectGarbage()` |
| 显式卸载 | 场景切换（可选） | 调用方 |

**"帧末回收"这一条很重要**：它消灭了"资源在渲染进行中被释放"这类最难复现的崩溃。

### 3.3 为什么这是"架构升级"而不是"资源模块重构"

因为它**改变了数据的形状**：组件的字段类型变了 → 序列化格式变了 → 编辑器检查器变了 → 脚本绑定变了。

**先做这个，再写序列化器**，否则你会先写一个基于 `Ref<Texture2D>` 的序列化器，然后发现它无法处理纹理，再全部推倒——这正是 `upgrade_plan.md` 把 R0（序列化）排在 R1（资源）之前的隐患。**正确顺序是资源身份先于序列化。**

> ⚠️ 这一条我要修正 `upgrade_plan.md` 的排序建议：**R0 和 R1 应当合并**，先定 `AssetHandle` 与组件字段形状，再写序列化。

### 3.4 顺带修掉的症状

| 缺陷编号 | 症状 |
|---|---|
| P2-6 | `Texture2D::Create` 依赖全局 `Application` 单例（分层倒置） |
| P2-8 | `AssetManager` 缓存键混乱、`AsyncLoad` 逻辑写错、`detach` 线程 |
| P3-4 | 资源路径是编译期烤入的绝对路径 |
| §4.3 | 组件里的 `Ref<Texture2D>` 不可序列化 |
| §4.4 | 无引用计数、无热重载、无泄漏检测 |

---

## 4. 架构问题 4：没有"数据模式"的权威

### 4.1 现状

**代码证据**：

```cpp
// SceneSerializer.cpp —— 5 行，全空（这是症状，不是原因）
#include "SceneSerializer.h"
namespace Bamboo { }

// Component.h:6-13 —— 组件清单是手写的固定列表，且不完整
#include "TransformComponent.h"
// ... 7 个，缺 Rigidbody/Collider/Shape

// 每个组件各自定义字段，没有任何元数据
struct TransformComponent {
    Vector3 Position;  // 序列化器怎么知道这个字段存在？不知道。
};
```

**问题不在"序列化器没写"，而在于：没有任何东西知道"一个组件由哪些字段组成"。**

于是：
- 序列化器要手写每个组件的读写代码（O(组件数) 的重复劳动）
- 编辑器检查器要手写每个组件的 UI（再一份）
- 复制粘贴要手写（第三份）
- 脚本绑定要手写（第四份）
- 每加一个字段，**四个地方都要改**，漏一个就是 bug

**这是"编辑器做不动"的真正原因**——不是编辑器难写，是**每加一个组件要改四个地方**，成本随组件数线性增长，而组件数必然增长。

### 4.2 要升级成什么样

**权威归属：`ComponentRegistry` —— 组件元数据的唯一来源，四处复用。**

```cpp
// 一处声明
BAMBOO_REGISTER_COMPONENT(TransformComponent, "Transform",
    BAMBOO_FIELD(Position),
    BAMBOO_FIELD(Rotation),
    BAMBOO_FIELD(Scale));

// 四处自动受益：
// 1. 序列化 —— 遍历字段生成 JSON
// 2. 编辑器检查器 —— 遍历字段生成 UI（Vector3 → 三个拖拽框）
// 3. 复制粘贴 —— 按字段拷贝
// 4. 脚本绑定 —— 暴露字段名（C# 侧按名字读写）
```

**配套的三条元数据**（决定这个机制能不能用）：

| 元数据 | 用途 | 例子 |
|---|---|---|
| **字段类型 + 反射访问器** | 序列化/UI/复制 | `float`、`Vector3`、`AssetHandle`、枚举 |
| **显示名 + 范围/步长** | 检查器 UI | `"速度"`、`Min=0`、`DragSpeed=0.1` |
| **序列化策略** | 哪些字段存盘 | `SkipSerialization`（如 `Dirty`、`WorldMatrix` 等缓存字段） |

**没有第三条会出问题**：`TransformComponent::Dirty`/`LocalMatrix`/`WorldMatrix` 是**缓存**，不该存盘。硬存进去会导致"加载后 Dirty=false 于是矩阵不重算"这类幽灵 bug。

### 4.3 为什么这是"架构升级"

因为它把**"组件的形状"从散落在 4 个地方的知识，变成 1 个地方的声明**。

这条做完之后，"加一个组件"的成本从"改 4 个文件"降到"写 1 个文件"。**编辑器、序列化、脚本系统全部变成"消费者"而不是"各自的实现"。**

### 4.4 顺带修掉的症状

| 缺陷编号 | 症状 |
|---|---|
| P0-12（部分） | `SceneSerializer` 空实现 |
| §4.3 | 无组件注册/反射 |
| P2-12 | `FindEntityByName` 靠线性扫描（可顺带加入名字索引元数据） |
| §3.4 | 字段"声明了但没人读"（元数据能报告"这个字段没有任何消费者"） |

---

## 5. 架构问题 5：没有"渲染契约"

### 5.1 现状

**代码证据**：

```cpp
// RendererSystem.cpp:26,45,55,66 —— 渲染系统每帧遍历 registry 决定画什么
registry.view<CameraComponent, TransformComponent>();
registry.view<TriangleComponent, TransformComponent>();
registry.view<QuadComponent, TransformComponent>();
registry.view<SpriteRendererComponent, TransformComponent>();

// Renderer2D.h:23-27 —— 绘制 API 的签名不一致，且部分无法使用
static void DrawQuad(const Vector2&, const Vector2&, const Color&);
static void DrawQuad(const Vector3&, const Vector2&, const Color&);   // 与上一行函数体完全相同
static void DrawSprite(const Matrix4&, const Color&, Ref<Texture2D>&);  // 非 const 引用 ← 无法传临时对象
static void DrawTriangle(const Vector3&, const Color&);               // 忽略 position

// Renderer2D.cpp:51-117 —— 4 种图元 × 6 个字段 = 24 个散落成员
```

**三个结构性问题**：

1. **"什么该被画"没有单一答案。** 渲染系统自己决定（遍历 registry、按 ZOrder 排序、补默认纹理）。所以：换一种渲染策略（比如加图层、加遮罩）就要改渲染系统；编辑器想"只画选中物体"也没法表达。
2. **绘制 API 的形状不稳定。** `DrawQuad` 有 2 个重载但行为相同；`DrawTriangle` 忽略参数；`DrawSprite` 的非 const 引用让它无法接收入参之外的东西。**API 一旦这样定型，任何改进都是破坏性变更**——而它才 4 个函数。
3. **渲染器的状态是"全局单例 + 每帧隐式重置"**（`static Renderer2DData s_Data`），没有"这次绘制属于哪个视图/哪个相机"的概念。这直接挡住多视口（编辑器必需）。

### 5.2 要升级成什么样

**权威归属：`RenderView`（"要画什么"的数据）+ `Batch<T>`（"怎么画"的封装）。**

```cpp
// 1. "画什么"变成显式数据，由生产者填充、渲染器只消费
struct RenderItem {
    Matrix4     Transform;
    AssetHandle Texture;        // 可为空 → 白纹理
    Color       Tint;
    int         ZOrder;
    uint32_t    Layer;
    // ...
};

struct RenderView {              // 一个视口一次渲染
    Camera               View;
    FrameBuffer*         Target = nullptr;   // null = 默认帧缓冲
    std::vector<RenderItem> Items;           // 排序后
};

// 2. 渲染器只做提交，不知道 ECS 的存在（← 这是解耦的关键）
class Renderer2D {
public:
    static void BeginFrame(const RenderView&);   // 取代 BeginScene/EndScene 的手工配对
    static void Submit(const RenderItem&);
    static void EndFrame();
};
```

**升级后的数据流**：

```
SpriteRendererComponent + TransformComponent
        ↓  （一个"收集系统"，只读 ECS）
       RenderItem
        ↓  （排序：Layer → ZOrder → 稳定序）
      RenderView
        ↓  （渲染器只消费，不认识 ECS）
      Renderer2D::Submit
```

**收益**：
- 渲染器**不再依赖 ECS** → 可单测（headless 后端直接验证 `RenderItem` 列表）
- 换排序策略/加图层 = 改收集系统，不动渲染器
- 编辑器"只画选中物体" = 过滤 `Items`，不动渲染器
- 多视口 = 多个 `RenderView`，不动渲染器

**同时把批次状态结构化**（消灭 24 个散落成员）：

```cpp
template <typename TVertex>
struct Batch {
    Ref<VertexBuffer> Buffer;
    Ref<VertexArray>  Array;
    Ref<Shader>       Shader;
    std::vector<TVertex> Vertices;      // ← 不再裸 new[]/delete[]
    uint32_t Capacity = 0, IndexCount = 0;
    bool CanFit(size_t n) const;
    void Reset();
};
```

### 5.3 顺带修掉的症状

| 缺陷编号 | 症状 |
|---|---|
| P0-2/P0-3/P0-4 | Quad 索引未初始化、缓冲尺寸错误、精灵超限无检查（`Batch` 统一管理容量） |
| P0-8 | 渲染期回写 ECS（收集与提交分离后，渲染器根本拿不到 ECS） |
| P1-7/P1-8 | `DrawQuad`/`DrawTriangle` 忽略参数（新 API 无此形状） |
| P1-9 | `Ref<Texture2D>&` 非 const（`RenderItem` 里是值语义） |
| P1-10 | 索引计数语义不一致（`Batch` 统一） |
| P2-4 | 渲染系统职责过载 |
| §3.7 | 24 个散落成员（结构化） |

---

## 6. 六个被推迟的架构问题（按优先级）

不是每个都要现在解决，但**要知道它们存在**，否则会做出挡住它们的决定。

| # | 问题 | 现状 | 为什么可以推迟 | 但要注意 |
|---|---|---|---|---|
| 1 | **无父子层级** | `TransformComponent` 无 `Parent`，`WorldMatrix = LocalMatrix` | 前面的游戏（Breakout、平台跳跃）可以不用层级 | 一旦做编辑器，层级是拖拽改父级的**前置**。`TransformSystem` 的迭代顺序要能处理父子（拓扑序） |
| 2 | **无实体启用/禁用** | 只有 `SpriteRendererComponent::Visible`（且渲染器不读，P3-4） | 可用"删除/重建"替代 | 池化需要它（对象池回收后要"禁用而非销毁"） |
| 3 | **无事件/消息系统** | `Event` 只用于窗口/输入，没有游戏层事件 | 游戏逻辑可轮询替代 | UI 交互、碰撞回调、脚本通信都需要它。**别把 Event 硬编码进 Scene**，要能独立扩展 |
| 4 | **`Window`/`Input` 是全局静态 + 单例链** | `Input::IsKeyPressed` → `Application::GetInstance()->GetWindow()`（P2-6 同类） | 单窗口够用 | 多窗口/多视口/编辑器 Game 视图都会撞上它。至少把 `nullptr` 检查补上 |
| 5 | **无图层/遮罩（Layer/Mask）** | 只有 `ZOrder` | 简单 2D 够用 | 一旦有 UI + 世界 + 调试叠加，就需要"分层渲染"。`RenderView` 的 `Layer` 字段是为它预留的 |
| 6 | **无帧缓冲/离屏渲染** | 完全没有 | 游戏不需要 | **编辑器视口必需**。这是 R3/R4 的硬前置 |

---

## 7. 目标架构

### 7.1 分层与依赖方向（唯一权威的落点）

```
┌──────────────────────────────────────────────────────────────┐
│ Game / Editor / Sandbox                                      │
│   Application · LayerStack · Panels                          │
├──────────────────────────────────────────────────────────────┤
│ Scene                       编排权威                          │
│   Scene · SystemRegistry(阶段+依赖+拓扑) · SceneSerializer    │
├──────────────────────────────────────────────────────────────┤
│ ECS                         数据权威                          │
│   Entity(entt::handle) · ComponentRegistry(模式) · Component  │
├──────────────────────────────────────────────────────────────┤
│ Systems                     行为                                  │
│   Logic物理 · 收集(RenderItem) · 相机                            │
├──────────────────────────────────────────────────────────────┤
│ Render                      渲染契约权威                       │
│   RenderView · RenderItem · Renderer2D(Batch) · Camera        │
├──────────────────────────────────────────────────────────────┤
│ Asset                       资源身份权威                       │
│   AssetDatabase(GUID) · AssetManager(实例+引用计数)            │
├──────────────────────────────────────────────────────────────┤
│ Platform / RHI              平台与后端                        │
│   Window · Input · OpenGL 实现                                │
├──────────────────────────────────────────────────────────────┤
│ Core                        无依赖基础                        │
│   Math · Log · Assert · Ref · UUID                            │
└──────────────────────────────────────────────────────────────┘
```

**四条硬性依赖规则**（违反即架构退化）：

| 规则 | 现状违反 |
|---|---|
| `Core` 不依赖任何其它层（含 entt/GLFW/GL） | 无违反 ✅ |
| `Render` 不依赖 `Game`/`Scene`/`ECS` | `Texture.cpp` 依赖 `Application` ❌ |
| `ECS` 不依赖 `Render`/`Asset` | `SpriteRendererComponent` 持有 `Ref<Texture2D>` ❌ |
| `Asset` 不依赖 `Scene`/`Game` | `Texture2D::Create(path)` 依赖 `Application` ❌ |

### 7.2 "权威"对照表（本文档的核心结论）

| 要决定的事 | 现在的权威 | 目标权威 |
|---|---|---|
| 系统按什么顺序跑 | 无（`Scene::Update` 的书写顺序） | `SystemRegistry` + `RunsAfter()` 拓扑排序 |
| 系统能不能写数据 | 无（都能写） | `ISystem`（可写）/ `IRenderSystem`（`const` 只读） |
| 实体是否有效 | 无（约定"必须来自 Scene"） | `Entity::IsValid()`（`entt::handle`） |
| 谁拥有组件里的资源 | `shared_ptr` 的所有持有者 | `AssetManager` 引用计数 |
| 资源的身份 | 路径字符串 | GUID（`.meta`） |
| 组件的字段有哪些 | 每个消费方各自手写 | `ComponentRegistry` 元数据 |
| 什么该被画 | `RendererSystem` 自己遍历 ECS | 收集系统产出 `RenderView` |
| 渲染器能读什么 | 整个 `registry`（可写） | 只有 `RenderItem`（不依赖 ECS） |

---

## 8. 迁移路径（不是重写）

**关键：这五件事可以按顺序做，每一件都让引擎保持可编译、可运行。** 不需要"停三个月重写"。

| 步骤 | 内容 | 改动范围 | 可运行的验证 |
|---|---|---|---|
| **A1** | `Entity` 改用 `entt::handle`；`m_Registry` 私有化 | `Entity.h`、`Scene.h`（~120 行） | 现有代码编译通过；`FindEntityByName` 失败不再崩 |
| **A2** | `SystemRegistry` 改为"开放阶段列表 + 单一 `Update` + 拓扑排序" | `ISystem.h`、`SystemRegistry.{h,cpp}`、`Scene.cpp`（~200 行） | Breakout 跑通；故意写一个循环依赖 → **启动时报错** |
| **A3** | 拆出 `IRenderSystem`（`const Scene&`），渲染系统改只读 | `ISystem.h` + `RendererSystem`（~80 行） | 渲染系统**编译期无法**回写 ECS |
| **B1** | 引入 `AssetHandle` + `AssetDatabase` + `.meta`；组件字段改句柄 | 新建 ~4 文件；改 `SpriteRendererComponent`、`AssetManager` | 贴图仍能显示；改名贴图后场景不断链 |
| **B2** | `ComponentRegistry` 元数据 + 首批 8 个组件注册 | 新建 ~3 文件 | 打印所有已注册组件与字段名 |
| **B3** | `SceneSerializer` 基于 B1+B2 实现 | 新建 ~2 文件 | **往返测试**：Save→Load→Save 字节一致 |
| **C1** | 引入 `RenderItem`/`RenderView`；收集系统与提交分离；`Batch<T>` 结构化 | `Renderer2D.{h,cpp}` 重写 + 新建收集系统 | 画面与重构前一致（截图对比）；渲染器可 headless 单测 |
| **C2** | `FrameBuffer` + 相机惰性重算 | 新建 ~2 文件 | 渲染到 512×512 FBO 并 `ReadPixels` 截图正确 |

**顺序理由**：
- **A 组（架构骨架）先做**：它决定后面所有代码的形状，而且只有 4 个系统要改。
- **B1 必须早于 B3**：资源身份先于序列化（否则序列化器要重写）。
- **C 组可以晚**：它是"渲染质量"投资，游戏玩法不受阻。但 **C2 的 FrameBuffer 必须早于编辑器**。

**总量**：约 **1,400~1,800 行**新增/重写。不是"重写引擎"（引擎才 4,600 行），而是**把四个权威补上**。

### 8.1 对 `upgrade_plan.md` 排序的修正

| 原排序 | 修正 | 原因 |
|---|---|---|
| R0 数据基座 → R1 资源 | **合并为一步，资源身份在前** | 序列化器的形状取决于组件字段类型（§3.3） |
| R1 渲染批次 → R3 | **C1 提前到 A 组之后** | `RenderItem` 的解耦会影响收集系统与编辑器视口的设计 |
| 编辑器在 R4 | **不变，但强调 FrameBuffer 是硬前置** | §6 第 6 条 |

---

## 9. 架构守护：让退化无法通过

架构文档最大的问题是**没人读、没人执行**。所以要把规则变成**自动化检查**。

### 9.1 编译期守护（最有效）

```cpp
// 依赖规则用 static_assert + 前置声明表达
// — Render 层禁止 include Scene/ECS：
//   做法：Render 的公开头文件不 include 任何 ECS/Scene 头，若有人加了，
//         因缺少类型定义而编译失败。
```

### 9.2 CI 脚本守护（低成本，覆盖剩下的）

```powershell
# tools/check_architecture.ps1
$rules = @(
  @{ Dir='Bamboo/Graphics';    Forbid='ECS/|Scene/|Game/'; Msg='Render 层不得依赖 ECS/Scene/Game' }
  @{ Dir='Bamboo/ECS';         Forbid='Graphics/|GraphicsAPI/|Platform/'; Msg='ECS 层不得依赖渲染/平台' }
  @{ Dir='Bamboo/Core';        Forbid='glad|GLFW|entt|Scene/|Graphics/'; Msg='Core 层必须无依赖' }
  @{ Dir='Bamboo/ECS/Component'; Forbid='Graphics/|GraphicsAPI/'; Msg='组件必须是纯数据' }
)
# 另外三条硬性检查：
# 1. 所有 .h 必须有 #pragma once
# 2. 任何 .cpp 不得出现裸 new[]/delete[]（除 ThirdParty）
# 3. 每个 ISystem 子类必须 override GetName()（否则拓扑排序没法报错）
```

### 9.3 把规则写进 `AGENTS.md`

（已部分完成——`AGENTS.md` 有注释约定；需要追加"依赖规则"一节，指向本文档。）

---

## 10. 什么不做（防止过度设计）

**引擎现在只有 4,600 行、单人开发。** 架构升级的目标是"减少未来的返工"，不是"看起来专业"。以下明确不做：

| 不做 | 理由 |
|---|---|
| ❌ 拆成多个 CMake target（Core/Render/ECS 各自一个库） | 编译期依赖强制很美好，但会显著增加构建配置成本。**用 CI 脚本 grep 代替**，效果 80%，成本 5% |
| ❌ 引入事件总线（EventBus）/ 消息系统 | 目前没有需求。等 UI 交互和碰撞回调真的需要时再加（§6 第 3 条） |
| ❌ 组件式"系统调度器"（如 C++ 版的 SystemGraph 框架） | 拓扑排序 + 显式声明已经够了。一层间接 = 一层调试成本 |
| ❌ 反射代码生成（用 Python/CMake 生成注册代码） | 手工注册 8 个组件的成本 < 写生成器的成本。等组件超过 20 个再考虑 |
| ❌ ECS 换库 / 自己写 ECS | entt 没问题。问题在**怎么用它**，不在它本身 |
| ❌ 立即模式 → 保留模式的大改 | `RenderItem` 列表已经够（每帧重建，但结构清晰）。真正的保留模式（脏标记 + 增量更新）等在 10k+ 实体实测有瓶颈时再说 |
| ❌ 多后端渲染抽象 | 见 `refactor_plan.md` D1，建议"诚实降级" |

**判断标准**：**这个抽象能消灭一类未来的 bug 吗？** 能 → 做；只是"更规范" → 不做。

---

## 11. 与其它文档的关系

| 文档 | 定位 | 本文档与它的关系 |
|---|---|---|
| `docs/architecture.md` | **现状**（对照代码核对） | 本文档描述"现状哪里不对"；那份描述"现状是什么" |
| `docs/refactor_plan.md` | **缺陷清单** + S0~S8 修复 | 37 项缺陷是**症状**；本文档的 §1~§5 是**根因**。修复阶段 S0~S3 与本文档的 A 组高度重叠——**以本文档的 A/B/C 分组为准** |
| `docs/upgrade_plan.md` | **模块施工计划** + R0~R7 | 本文档修正了它的排序（§8.1）；模块任务清单仍然有效 |
| `docs/games_and_editor.md` | **游戏计划 + 编辑器设计** | 编辑器设计依赖本文档的 §5（渲染契约）与 §6 第 6 条（FrameBuffer） |
| `docs/roadmap.md` | 学习路线 | 本文档的每一条都可作为"学架构"的案例 |

### 建议的阅读顺序（对未来的你或任何接手的人）

1. `architecture.md` —— 现在是什么样
2. **本文档** —— 哪里不对、要变成什么样、怎么迁
3. `upgrade_plan.md` —— 具体任务清单
4. `games_and_editor.md` —— 用哪个游戏验收

---

## 附：本文档自己也会过时

**维护约定**：
- 每完成 A1/A2/A3/B1~B3/C1~C2 中的一项，在 §8 对应行打 ✅ 并记录 commit。
- 如果某个"架构问题"被证明是误判（实践中不影响），**删掉它并说明原因**——不要留着凑数。
- 目标架构（§7）如果发生变化，**必须同时更新 §9 的守护脚本**，否则规则会失效。
