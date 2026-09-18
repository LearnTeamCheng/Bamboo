# Bamboo 引擎升级计划 — 从「渲染跑通」到「可用引擎」

> 定位：`refactor_plan.md` 是**体检报告**（有什么病、怎么治），本文件是**施工蓝图**（按什么顺序建、每步交付什么、怎么验收）。
> 范围：渲染管线、编辑器、场景序列化、资源系统、C# 脚本、测试体系、调试工具、打包分发。
> 时间基准：2026-09-18（当前工具链：VS 2022 x64 / C++17 / 本机 dotnet SDK **9.0.305**）
> 原则：**每个阶段必须交付一个"能演示的东西" + 一组可复现的数字**。没有 demo 的阶段不算完成。

---

## 目录

- [0. 「可用」的定义（全局验收标准）](#0-可用的定义全局验收标准)
- [1. 七个模块的现状 → 目标 → 差距](#1-七个模块的现状--目标--差距)
- [2. 两个决定成败的架构选择](#2-两个决定成败的架构选择)
- [3. 模块升级详解 A：渲染管线](#3-模块升级详解-a渲染管线)
- [4. 模块升级详解 B：场景序列化与数据驱动](#4-模块升级详解-b场景序列化与数据驱动)
- [5. 模块升级详解 C：资源系统](#5-模块升级详解-c资源系统)
- [6. 模块升级详解 D：编辑器](#6-模块升级详解-d编辑器)
- [7. 模块升级详解 E：C# 脚本系统](#7-模块升级详解-ec-脚本系统)
- [8. 模块升级详解 F：测试体系](#8-模块升级详解-f测试体系)
- [9. 模块升级详解 G：调试工具与可观测性](#9-模块升级详解-g调试工具与可观测性)
- [10. 总验收：垂直切片 demo](#10-总验收垂直切片-demo)
- [11. 阶段路线图（R0~R7）与依赖](#11-阶段路线图r0r7与依赖)
- [12. 工作量估算与里程碑](#12-工作量估算与里程碑)
- [13. 风险登记册](#13-风险登记册)
- [14. Definition of Done（每个任务完成的定义）](#14-definition-of-done每个任务完成的定义)

---

## 0. 「可用」的定义（全局验收标准）

先把"可用引擎"变成可测量的东西，否则永远做不完。以下每条都是**可执行、可复现**的，最终由 §10 的垂直切片 demo 一次性验收。

### 0.1 功能验收

| # | 验收项 | 判定标准 | 怎么测 |
|---|---|---|---|
| F1 | 启动不崩 | 连续启动/退出 50 次零崩溃、零 GL 错误 | 脚本循环 + GL 调试回调计数 = 0 |
| F2 | 分辨率无关 | 800×600 / 1280×720 / 1920×1080 / 2560×1440 四种分辨率下，同一场景的**相对布局与截图一致**（像素级对比误差 < 1%） | 自动截图对比 |
| F3 | 窗口缩放正确 | 拖动缩放窗口，画面不变形、不拉伸、黑边策略一致 | 手动 + 截图 |
| F4 | 渲染正确 | 精灵/四边形/圆/线/文字的正确性；Z 排序稳定；透明度混合正确 | 参照场景截图 |
| F5 | 物理正确 | 自由落体 `v=g·t` 与解析解误差 < 1%；球撞墙反射角误差 < 1°；能量衰减单调 | 物理单测 + 记录曲线 |
| F6 | 场景存读 | 编辑 → 保存 → 重启 → 重新打开，截图与编辑态**一致** | 自动往返对比 |
| F7 | 序列化鲁棒 | 损坏/截断/字段缺失/未知组件/版本更旧的场景文件**不崩溃**，给出明确错误并加载可用部分 | fuzz 测试（见 §8.4） |
| F8 | 资源热重载 | 改贴图/着色器文件，**2 秒内**运行中自动生效，不用重启 | 文件监听测试 |
| F9 | 编辑器可用 | 能在编辑器里完成"新增实体 → 加组件 → 改属性 → 拖到位置 → 存盘"全流程 | 端到端手动脚本 |
| F10 | 脚本可用 | 能用 C# 写移动/碰撞/生成逻辑，**改脚本后重载 < 5 秒**（NativeAOT 路线）或**< 1 秒**（CoreCLR 路线） | 计时 |
| F11 | 发布可用 | 一条命令产出可分发目录，在**未装任何 SDK 的干净机器**上能跑 | 干净 VM 验证 |

### 0.2 性能验收（必须有前后对比数字）

| # | 场景 | 目标 | 备注 |
|---|---|---|---|
| P1 | 静态精灵渲染 | 10,000 精灵，DrawCall ≤ 20，CPU 帧耗时 < 4ms | 1080p/Debug 构建基线另记 |
| P2 | 动态精灵渲染 | 2,000 精灵逐帧移动，稳定 60 FPS（帧耗时 < 16.6ms） | |
| P3 | 物理 | 1,000 动态刚体 + 100 静态，步进 < 4ms | 对比暴力 O(n²) 的加速比 |
| P4 | 场景加载 | 1,000 实体的场景加载 < 500ms | 含资源解析 |
| P5 | 编辑器空闲 | 编辑器打开 5,000 实体场景不操作时 < 8ms/帧 | UI 不卡 |
| P6 | 内存 | 反复"进场景 → 出场景"20 次，显存与内存**不增长**（无泄漏） | 任务管理器 + GL 资源计数 |
| P7 | 脚本互操作 | 1,000 个脚本实体 Update 总耗时 < 2ms | 跨语言调用开销要量化 |

### 0.3 工程验收

| # | 验收项 | 判定标准 |
|---|---|---|
| E1 | 构建 | 全新 clone → 一条命令 configure + build 成功；`/W4` 零警告 |
| E2 | 测试 | `ctest` 全绿；核心模块行覆盖率 ≥ 70%（Math/ECS/序列化/物理 ≥ 85%） |
| E3 | CI | push 后自动构建 + 测试 + 性能基线对比；回归会红 |
| E4 | 分层 | 架构守护脚本零违反（Render 不依赖 Game 等） |
| E5 | 文档 | README 能让人 30 分钟内跑起来；每个公开 API 有注释；`docs/` 无过时文档 |
| E6 | 提交规范 | Conventional Commits；每个 feature 独立分支；无大文件入库 |

---

## 1. 七个模块的现状 → 目标 → 差距

一页看清全貌。**"差距"列 = 我估计的需要新建/重写的文件数**，用来判断工作量量级。

| 模块 | 现状（2026-09） | 目标 | 差距 |
|---|---|---|---|
| **渲染管线** | 4 种图元各自独立批次状态（24 个散落成员）；缓冲尺寸错误；硬编码 1280×720；无 FrameBuffer；无文字；无统计 | 统一批次模板 + 可配置容量 + 离屏渲染 + 文字 + 完整统计 + 相机可控 | 重写 3 文件 + 新建 8 文件 |
| **场景序列化** | `SceneSerializer` 5 行空实现；无格式；无反射 | 组件注册表 + JSON + 版本迁移 + 往返一致 + 引用解析 | 新建 6 文件 |
| **资源系统** | `AssetManager` 只有同步缓存（且 `AsyncLoad` 逻辑写错）；路径编译期烤死；组件存 `Ref<Texture2D>` | GUID + `.meta` + 依赖图 + 引用计数 + 异步 + 热重载 + 运行期路径解析 | 重写 2 文件 + 新建 7 文件 |
| **编辑器** | 独立 `Application`，绕过引擎；ImGui 没接 GL3 后端渲染不出来；无面板 | 引擎内 Layer；视口 FBO；层级/检查器/内容浏览器；Gizmo；Undo/Redo | 重写 3 文件 + 新建 20+ 文件 |
| **C# 脚本** | 无 | 生命周期回调 + 组件访问 + 热重载 + GC 可控 | 新建 ~15 文件 + C# 运行时工程 |
| **测试** | 零 | gtest 单测 + 集成 + 物理验证 + 渲染快照 + 性能基线 + fuzz | 新建 ~25 测试文件 + 基础设施 |
| **调试/打包** | 无 FPS/DrawCall 显示；无控制台；无资源随包 | Debug 叠层 + 控制台命令 + 崩溃报告 + 一键打包 | 新建 ~10 文件 |

---

## 2. 两个决定成败的架构选择

这两件事如果选错，后面所有模块都要返工。**先定，再动手。**

### 2.1 决策一：编辑器是「引擎内的一个 Layer」，不是「另一个程序」

**现状**：`Editor/Source/Core/Application.cpp` 自己 `glfwInit` + `glfwCreateWindow`，完全不用引擎的 `Bamboo::Application` / `Window` / `Scene` / `Renderer2D`。这导致编辑器**根本不是引擎的编辑器**，而是"碰巧也用了 ImGui 的另一个程序"。

**必须改成**：

```
Bamboo::Application
 ├─ LayerStack
 │   ├─ GameLayer        ← 运行场景（Game/Editor 共用同一份）
 │   └─ EditorLayer      ← 只在编辑器模式下挂载
 │        ├─ ViewportPanel   (渲染到 FBO 再贴到 ImGui)
 │        ├─ HierarchyPanel
 │        ├─ InspectorPanel
 │        ├─ ContentBrowserPanel
 │        ├─ ConsolePanel
 │        └─ GizmoLayer
 └─ Scene / AssetManager / Renderer2D
```

**三种运行模式**（可以是两个 target，但**共用同一个 Application 骨架**）：

| 模式 | 入口 | 挂载 | 用途 |
|---|---|---|---|
| Editor | `Editor.exe` | GameLayer + EditorLayer，**暂停/单步/播放**可切换 | 编辑场景 |
| Game | `Sandbox.exe` | 仅 GameLayer，直接进游戏循环 | 最终游戏 |
| Headless/Test | `BambooTests.exe` | 无 Window/无 GL，只有 Scene + 逻辑 | CI 自动化测试 |

**为什么这是关键**：只有"编辑器跑在和游戏完全相同的 Scene/Renderer 上"，编辑器里看到的东西才**保证**和游戏里一致。否则你会永远在修"编辑器里对、游戏里不对"的 bug。这也是 Unity/Godot/Unreal 的共同选择。

**前置**：需要先补 `LayerStack`（约 150 行）和 `FrameBuffer`（约 200 行，渲染层目前**完全没有**）。

### 2.2 决策二：脚本先做对"边界"，再选语言；热重载优先级高于性能

你把 C# 写进了目标，但**不要一上来就上 C#**。原因：

1. 脚本系统的难点**不在语言**，而在**边界设计**：实体/组件怎么暴露、生命周期怎么触发、GC 怎么和 C++ 对象生命周期对齐、异常怎么处理、热重载时旧对象怎么办。这套边界**用 Lua 做只需要 1/5 的时间**，而且改起来快得多。
2. 边界一旦定型，**换语言只是重写绑定层**；边界没定型就上 C#，等于把 3 个难问题（边界设计 + 跨语言 ABI + .NET 运行时管理）叠在一起调，出了问题无法定位。
3. C# 的两条技术路线（NativeAOT 导出 vs CoreCLR 托管）**在热重载能力上差异巨大**，而这个差异会反过来影响边界设计（详见 §7.4）。

**建议路径**：

```
S1 定义脚本边界（C++ 接口 + 生命周期 + 组件访问协议，纯 C++，可用一个假脚本层验证）
   ↓
S2 用 Lua/sol2 落地这套边界（1~2 天级工作量，验证边界是否够用）
   ↓
S3 边界稳定后 → 上 C#（NativeAOT 或 CoreCLR，按 §7.4 的决策树）
```

**如果时间紧、只想尽快有脚本**：直接停在 Lua。Lua 对 2D 游戏的游戏逻辑（移动、碰撞响应、关卡脚本、对话）**完全够用**，社区成熟、嵌入成本极低。

> ### ✅ 决策已定（2026-09-18）：脚本走 **C#**（CoreCLR + 可卸载 ALC）
> 因此本节的"先用 Lua 验证边界"改为**降级为可选**：如果 G1 的时间允许，Lua 原型仍值得做（1~2 天换一个验证过的边界），但**不再作为 C# 的前置阻塞**。
> 直接上 C# 时必须遵守的纪律见 `docs/games_and_editor.md` §11（AOT-clean 禁令 + 结构体布局自检 + 脚本类显式注册）。


---

## 3. 模块升级详解 A：渲染管线

### 3.1 目标形态

```
Renderer2D
 ├─ 统一的 Batch<VertexT>（消灭 4 套重复状态）
 ├─ 可配置容量（Init(spec)：MaxQuads / MaxSprites / MaxTextureSlots）
 ├─ 自动换批（顶点满 / 纹理槽满 / 状态切换）
 ├─ 图元：Quad / Circle / Line / Polygon / Sprite（含 9-slice / 平铺）
 ├─ 文字：位图字体图集（先） → SDF（后）
 ├─ FrameBuffer（离屏渲染，编辑器视口 + 后处理 + 截图的基础）
 ├─ 渲染状态（Blend / Depth / Cull / Scissor）显式化，走 RendererState
 ├─ 统计：DrawCalls / Batches / 各图元数 / 顶点数 / 纹理绑定数 / CPU 耗时
 └─ 坐标系契约：对外只有"世界坐标（1 单位 = 1 像素）"，NDC 只在内部顶点常量里
```

### 3.2 工作分解

| # | 任务 | 说明 | 依赖 |
|---|---|---|---|
| A1 | `Batch<TVertex>` 结构体 | 打包 `{Ref<VertexBuffer>, Ref<VertexArray>, Ref<Shader>, std::vector<TVertex>, uint32_t Count, uint32_t Capacity, uint32_t IndexCount}`；提供 `PushVertex`、`CanFit(n)`、`Reset()` | — |
| A2 | `VertexBuffer` 契约修正 | 记录 `m_Size`；`SetData` 断言 `size <= m_Size`；更新用 `glBufferSubData`；构造期 `GL_DYNAMIC_DRAW` 一次性分配 | A1 |
| A3 | 索引表预生成 | Quad/Sprite/Circle 的索引按 `Capacity` 预生成一次上传，之后永不重传 | A1 |
| A4 | 自动换批 | `DrawXxx` 入口统一检查容量与纹理槽，满了先 `Flush` 再继续 | A1~A3 |
| A5 | 图元参数化 | 所有绘制走 `Draw*(const Matrix4& transform, ...)`；删除忽略参数的重复重载 | A1 |
| A6 | 线/多边形 | `DrawLine(a, b, thickness, color)`、`DrawPolygon(points, count, ...)`（圆环也用它实现） | A5 |
| A7 | `RendererState` | Blend/Depth/Cull/Scissor 的显式开关；`RendererCommand` 只做转发 | — |
| A8 | `FrameBuffer` 抽象 + OpenGL 实现 | 颜色/深度附件、`Resize`、`Bind/Unbind`、`ReadPixels`（截图用）、`GetColorAttachmentID`（贴 ImGui 用） | A7 |
| A9 | 相机系统重做 | `Camera` 纯数学；`SetWorldHeight`（缩放唯一来源）+ 中心 + 旋转；惰性重算投影；`ScreenToWorld`/`WorldToScreen` | — |
| A10 | 渲染统计接入 | `Renderer2DStats` 全字段 + `Renderer2D::GetStats()`；帧结束不清零，由 HUD 读取 | A1 |
| A11 | GL 调试回调 | Debug 构建启用 `glDebugMessageCallback` + `KHR_debug`，错误立即可见 | — |
| A12 | 文字渲染 | 位图字体（BMFont 格式）+ 图集加载 + `DrawText(font, text, transform, color, size)`；依赖 A5 | A5、C3 |
| A13 | 9-slice / 平铺 | `DrawSprite9Slice(transform, size, borders, tex)`；UI 与可缩放面板必备 | A5 |

### 3.3 验收标准

| 验收 | 判定 |
|---|---|
| 无 GL 错误 | Debug 构建下跑 5 分钟参照场景，`glDebugMessageCallback` 计数 = 0 |
| 容量正确 | `MaxSprites = 3` 的极端配置下画 10 个精灵，画面与不限制时**完全一致**（自动换批生效） |
| 分辨率无关 | F2（四分辨率截图对比误差 < 1%） |
| 性能 | P1（10k 静态精灵 DrawCall ≤ 20、CPU < 4ms）、P2（2k 动态 60FPS） |
| 统计准确 | HUD 显示的 DrawCall 数与 RenderDoc/帧分析工具读数一致 |
| 离屏渲染 | 把场景渲染到 512×512 FBO，`ReadPixels` 保存 PNG，内容与直接渲染一致 |
| 文字 | 中英文（含 UTF-8）都能正确显示；改字体文件热重载生效 |

---

## 4. 模块升级详解 B：场景序列化与数据驱动

### 4.1 目标形态

```jsonc
// Assets/Scenes/level01.bamboo.json —— 人类可读、可 diff、可手改
{
  "version": 3,                       // 格式版本，用于迁移
  "engine": "0.3.0",
  "scene": {
    "name": "Level01",
    "settings": { "gravity": [0, -980], "background": "#1A2B3CFF" }
  },
  "assets": [                          // 场景引用的资源（GUID → 相对路径，路径只是提示）
    { "guid": "a1b2c3d4-...", "type": "Texture2D", "path": "Textures/player.png" }
  ],
  "entities": [
    {
      "id": "7f3e...",                 // 稳定 UUID
      "name": "Player",
      "active": true,
      "parent": null,                  // 层级：父实体 id
      "components": {
        "Transform":      { "position": [100, 200, 0], "rotation": [0,0,0], "scale": [1,1,1] },
        "SpriteRenderer": { "texture": "a1b2c3d4-...", "color": "#FFFFFFFF",
                            "size": [64, 64], "zOrder": 0, "visible": true },
        "Rigidbody2D":    { "type": "Dynamic", "mass": 1.0, "restitution": 0.6 },
        "BoxCollider2D":  { "size": [64,64], "offset": [0,0], "isTrigger": false },
        "Script":         { "class": "PlayerController", "fields": { "speed": 300.0 } }
      }
    }
  ]
}
```

### 4.2 关键设计

**组件注册表（反射的最小可用形态）**

```cpp
// Scene/ComponentRegistry.h
struct ComponentMeta {
    std::string_view         Name;
    SerializeFn              Serialize;     // (writer, registry, entity)
    DeserializeFn            Deserialize;   // (value, registry, entity)
    DrawInspectorFn          DrawInspector; // (registry, entity) —— 编辑器面板复用同一份元数据
    CopyFn                   Copy;          // 编辑器复制粘贴
    std::function<void(entt::registry&, entt::entity)> AddDefault;  // "Add Component" 菜单
};

// 注册宏，一处声明三处受益（序列化 + 检查器 + 复制）
BAMBOO_REGISTER_COMPONENT(TransformComponent, "Transform",
    .Field("position", &TransformComponent::Position)
    .Field("rotation", &TransformComponent::Rotation)
    .Field("scale",    &TransformComponent::Scale));
```

**"一处声明、多处受益"是这个设计的核心价值**：组件加一个字段，序列化、编辑器检查器、复制粘贴、脚本绑定**四处自动跟上**。没有它，编辑器每加一个组件都要改 4 个地方。

**其他必须做对的点**：

| 点 | 要求 |
|---|---|
| 资源引用 | 组件存 `AssetHandle{ UUID }`，**绝不存 `Ref<Texture2D>`**；反序列化时通过 `AssetManager` 解析；解析失败保留 GUID 并警告（不丢数据） |
| 稳定 UUID | 实体 ID 在编辑期间不变；复制实体生成新 ID；存盘不重排 |
| 版本迁移 | `v1→v2→v3` 逐级迁移函数链；**旧文件永远能打开**（这条决定项目能不能长期迭代） |
| 未知组件 | 读到不认识的组件名 → 保留原始 JSON 到"未知数据"里，存盘时**原样写回**（避免用旧版本打开新场景后丢数据） |
| 写盘安全 | 先写 `.tmp` → `fsync` → 原子 rename；避免写一半崩溃毁掉场景 |
| 往返一致 | `序列化 → 反序列化 → 序列化` 两次输出**字节相同**（唯一的可信验收） |

### 4.3 工作分解

| # | 任务 | 依赖 |
|---|---|---|
| B1 | JSON 库选型并 vendored（建议 `nlohmann/json` 单头，见 `refactor_plan.md` D2） | — |
| B2 | `JsonWriter`/`JsonValue` 轻封装（隔离具体库，便于以后换） | B1 |
| B3 | `ComponentRegistry` + 注册宏 + 首批 8 个组件注册 | B2 |
| B4 | `AssetHandle` + `AssetHandle` 的序列化 | C1 |
| B5 | `SceneSerializer::Serialize`（实体 + 组件 + 资源表 + 设置 + 版本） | B3 |
| B6 | `SceneSerializer::Deserialize`（含未知组件保留、缺字段用默认值、错误收集） | B3 |
| B7 | 版本迁移链 `Migrate(json, fromVersion)` | B6 |
| B8 | 原子写盘（tmp + rename）+ 备份（`.bak`） | B5 |
| B9 | 往返一致性测试（见 §8.2） | B5、B6 |
| B10 | 组件"从注册表读默认值"（Add Default），供编辑器与反序列化共用 | B3 |
| B11 | 场景引用（场景间跳转、Prefab 引用）—— 后期 | B5 |

### 4.4 验收标准

| 验收 | 判定 |
|---|---|
| 往返一致 | `Save → Load → Save`，两文件 SHA256 相同 |
| 完整往返 | 100 实体 × 8 组件的场景，往返后每个字段逐项对比相等 |
| 旧版本兼容 | 手写 v1 文件能被当前版本打开，字段正确迁移 |
| 损毁鲁棒 | 截断/乱码/字段类型错误/未知组件的文件，**不崩溃**且有明确错误信息（F7） |
| 手改可用 | 手工编辑 JSON 里的位置数值，重新加载后实体位置正确变化 |
| 可 diff | 只移动一个实体，git diff 只显示该实体的相关行（顺序稳定） |

---

## 5. 模块升级详解 C：资源系统

### 5.1 目标形态

```
Assets/
 ├── Textures/player.png            ← 源文件（美术提交）
 ├── Textures/player.png.meta       ← 旁挂元数据（GUID + 导入设置），提交进版本库
 └── Scenes/level01.bamboo.json

AssetDatabase（磁盘侧）：GUID ↔ 路径 映射、.meta 读写、导入器分发、依赖扫描
AssetManager （内存侧）：已加载实例注册表、引用计数、加载状态机、热重载、异步加载
```

### 5.2 关键设计

**1. `.meta` 文件是稳定引用的唯一基础**

```jsonc
// player.png.meta
{ "guid": "a1b2c3d4-...", "type": "Texture2D",
  "importSettings": { "filter": "Linear", "wrap": "Clamp", "mipmap": false } }
```

- GUID 一旦生成**永不变**；文件改名/移动只改路径，GUID 不变 → **场景引用不断链**。
- `.meta` 必须提交进 Git；`.meta` 丢失 = 引用丢失（要在 CI 里检查"每个资产都有 .meta"）。

**2. "假空"（fake null）与加载状态机**

这是 Unity 的经典教训：资源被卸载后，`Ref<T>` 可能非空但对象已无效，导致"看起来有引用、用起来崩"。**必须显式建模状态**：

```cpp
enum class AssetState { NotLoaded, Loading, Loaded, Failed };
// 组件里存 AssetHandle（GUID）
// 渲染时：auto tex = assets.Get<Texture2D>(handle);
//         if (!tex) → 用占位纹理（粉白格子）+ 记录一次警告
```

**3. 引用计数与生命周期**

- `AssetManager` 持有 `Ref<Asset>`；组件持有 `AssetHandle`（弱引用语义）。
- 提供 `AssetManager::UnloadUnused()`：引用计数为 0 且超过 N 帧的资源才真正释放（避免"这一帧没用下一帧又用"的抖动）。
- **检测泄漏**：退出时报告仍未释放的资源清单（对应验收 P6）。

**4. 异步加载**

- 用线程池（不是每次 `std::thread` + `detach`，见 `refactor_plan.md` P2-8）。
- 主线程轮询完成的 future，**GPU 上传必须在主线程**（GL 上下文单线程）。
- 加载状态可供 UI 查询（内容浏览器显示进度条）。

**5. 热重载**

- `ReadDirectoryChangesW`（Windows）监听 `Assets/` 目录。
- 文件变化 → 重新导入 → 替换 `AssetManager` 里的实例 → **通知所有引用者**（重新解析 handle）。
- 纹理/着色器热重载是最有价值的两个（美术/TA 迭代速度直接翻倍）。

### 5.3 工作分解

| # | 任务 | 依赖 |
|---|---|---|
| C1 | `AssetHandle`（GUID 包装）+ `AssetType` | B2 |
| C2 | `AssetDatabase`：GUID 生成、路径↔GUID、`.meta` 读写、目录扫描 | — |
| C3 | 导入器接口 `IAssetImporter` + `TextureImporter` + `ShaderImporter`（含 `.meta` 导入设置） | C2 |
| C4 | `AssetManager` 重写：实例注册表 + 状态机 + 引用计数 + 线程安全 | C2 |
| C5 | 运行期资源根解析（环境变量 → exe 相对路径 → 编译期默认） | — |
| C6 | 线程池（或固定 worker 数队列） | — |
| C7 | 异步加载 API（`LoadAsync` 返回状态，主线程完成 GPU 上传） | C4、C6 |
| C8 | 依赖图（着色器依赖、场景依赖资源、Prefab 依赖） | C3 |
| C9 | 文件监听 + 热重载 + 通知机制 | C3、C4 |
| C10 | `UnloadUnused` + 退出泄漏报告 | C4 |
| C11 | 占位资源（粉白格子纹理、缺失着色器 fallback） | C4 |
| C12 | 内容浏览器与资源导入 UI（→ 见 §6） | C4 |

### 5.4 验收标准

| 验收 | 判定 |
|---|---|
| 引用稳定 | 把 `player.png` 改名成 `hero.png`（`.meta` 跟着改），场景**不需要改动**仍能正确加载 |
| 热重载 | 改 PNG 内容 → 2 秒内运行中画面变化（F8） |
| 无泄漏 | P6：反复进出场景 20 次，GL 纹理数/内存不增长 |
| 失败可见 | 删掉一个贴图文件 → 场景仍能加载，缺失处显示占位纹理 + 控制台一条 error（不是崩溃、不是静默黑块） |
| 异步不卡 | 加载 50 张 2K 贴图时主线程帧率不低于 50 FPS |
| 路径可移植 | 整个仓库换目录后**不重新 configure** 也能加载资源（F11 的前置） |

---

## 6. 模块升级详解 D：编辑器

### 6.1 目标形态（面板清单）

| 面板 | 功能 | 优先级 |
|---|---|---|
| **Viewport** | 渲染场景到 FBO 贴到 ImGui；可缩放/平移；点击选中；Gizmo 拖拽 | P0（没有它编辑器不成立） |
| **Hierarchy** | 实体树（含父子层级）、搜索、重命名、增删、拖拽改父级、显示/隐藏 | P0 |
| **Inspector** | 从组件注册表自动生成属性编辑器；增删组件；字段级 Undo | P0 |
| **Content Browser** | 浏览 `Assets/`；GUID 显示；拖到场景创建精灵；新建/重命名/删除；导入设置 | P0 |
| **Console** | 日志分级过滤、搜索、点击跳转、命令输入 | P1 |
| **Stats / Profiler** | FPS、帧耗时曲线、DrawCall、实体数、物理步进耗时、内存 | P1 |
| **Scene Settings** | 重力、背景色、相机默认参数 | P1 |
| **Play / Pause / Step** | 运行时状态隔离（编辑态与运行态分离） | P0（"能试玩"是编辑器的核心价值） |
| **Asset Preview** | 选中贴图预览、Sprite 帧切片 | P2 |
| **Build Settings** | 场景列表、启动场景、打包 | P2 |

### 6.2 三个必须先确认的技术事实（我核查了仓库现状）

| 事实 | 现状 | 影响 | 处理 |
|---|---|---|---|
| **ImGui 版本** | vendored 的是 **1.92.4 WIP**，`IMGUI_VERSION_NUM = 19233`，头文件里**没有 `IMGUI_HAS_DOCK`** → 这是 **master 分支，不支持停靠（Docking）** | §6.1 想要的"面板自由停靠 + 工作区布局"**做不到**；`ImGui::DockSpace` 不存在 | **见下方"Docking 决策"** |
| **ImGui 新纹理 API** | 1.92 起 `ImGui::Image` 走 `ImTextureRef`，旧的"把 `ImTextureID` 直接当 GL id 传"的写法已变 | 视口贴图（D5）写法与网上多数老教程不同，照抄会编译失败 | D5 以 vendored 头文件为准（用 `ImTextureRef` 或当前等价写法） |
| **ImGui GL3 后端已编译但未接入** | `ThirdParty/CMakeLists.txt:38-43` 编译了 `imgui_impl_opengl3.cpp`；`Editor/Source/Core/Application.cpp:59` **只调用了 `ImGui_ImplGlfw_InitForOpenGL`**，没调 `ImGui_ImplOpenGL3_Init` | 这是"ImGui 渲染不出来"的**直接原因**；后端不用重新引入，只差调用 | D3 补全四件套：`ImGui_ImplOpenGL3_Init` / `NewFrame` / `RenderDrawData` / `Shutdown` |

**Docking 决策（需要你定）**：

| 选项 | 做法 | 代价 | 收益 |
|---|---|---|---|
| **A. 换到 docking 分支**（推荐） | 用 ImGui 的 `docking` 分支替换 `ThirdParty/imgui` | 一次性替换（约 1 小时）+ 适应新 API | 自由停靠面板、可保存工作区布局、**编辑器体验直接达到可用水准** |
| **B. 保持 master，自己算布局** | 用 `SetNextWindowPos/Size` 手动摆放固定布局，或引入第三方 docking 替代 | 面板不可拖拽重组；每加面板要重算布局 | 不换依赖 |
| **C. 保持 master，先不做布局** | 面板用浮动窗口，位置存 `imgui.ini` | 重叠遮挡，5,000 实体场景下很难用 | 最省事 |

**建议 A**。理由：编辑器有 6+ 个面板，没有停靠几乎无法使用；ImGui docking 分支长期存在且稳定，迁移成本是"替换目录 + 重编译"。**如果你不想碰第三方目录**，退到 **B**（固定布局也能做出能用的编辑器）。

### 6.3 关键设计

**1. 编辑态 vs 运行态隔离（最容易做错的地方）**

点"Play"时必须能把场景**恢复到编辑前状态**。两种方案：

| 方案 | 做法 | 取舍 |
|---|---|---|
| **快照法** | Play 前把场景序列化成内存 JSON；Stop 时反序列化回来 | 简单可靠，**推荐先用这个**（复用 B 模块）；大场景有开销 |
| 双场景法 | 编辑用 Scene A，Play 时深拷贝出 Scene B 跑 | 快，但要写完整的场景深拷贝 |

**建议**：先用"快照法"。它顺带**强制验证了序列化的完整性**——如果 Play/Stop 后场景变了，说明序列化漏了字段。这是免费的测试。

**2. Undo/Redo（命令模式）**

```cpp
class Command {
public:
    virtual void Undo() = 0;
    virtual void Redo() = 0;
    virtual std::string_view GetName() const = 0;
};
class CommandHistory {  // 栈 + 合并（连续拖动合并成一条）
    void Push(Scope<Command>);
    void Undo(); void Redo();
};
```

- 所有修改场景的编辑器操作**必须**走命令（先做"改属性/增删实体/拖拽"三类，覆盖 90% 场景）。
- 合并策略：拖动 Gizmo 期间的所有修改合并为一条命令（否则 Undo 一次只退 1 像素）。

**3. Gizmo**

- 平移/旋转/缩放三种；屏幕空间恒定大小（不随缩放变大）；吸附（Ctrl 格点吸附、Shift 角度吸附）。
- 实现方式：ImGui 的 `InvisibleButton` + 自己算世界坐标下的轴线段与拖拽平面求交。**依赖渲染层的线绘制（A6）**。

**4. 选中与拾取**

- 世界坐标 AABB/圆 与鼠标世界坐标的相交测试（复用物理的形状模块）。
- 多选（Ctrl/框选）→ 批量操作。

**5. 运行时状态与编辑器状态分离**

- `Scene` 加一个 `SceneState`（Editing / Playing / Paused），`ISystem::OnUpdate` 在 Editing 时只跑编辑器需要的系统（不跑物理/脚本）。
- 编辑器自己的状态（面板开关、相机位置、选中集合）**不放进 Scene**，放在 `EditorState` 里。

### 6.4 工作分解

| # | 任务 | 依赖 |
|---|---|---|
| D1 | `Layer` / `LayerStack` + 事件按层传播 | — |
| D2 | `Editor::Application` 改为继承 `Bamboo::Application`，删掉自建窗口（**先做这一步**） | D1 |
| D3 | ImGui GL3 后端完整接入（`ImGui_ImplOpenGL3_Init` / `NewFrame` / `RenderData` / shutdown） | D2 |
| D4 | 布局与外观：按 §6.2 的 **Docking 决策**执行（选项 A：换 docking 分支并启用 DockSpace；选项 B：固定布局）；主题 + 字体（含中文，需自带 ttf 并合并字形范围） | D3 |
| D5 | Viewport 面板：FBO + `TextureID` 贴图 + 尺寸跟随 + DPI 处理 | A8、D3 |
| D6 | 鼠标坐标 → FBO 内世界坐标 映射 + 点击拾取 | D5、A9 |
| D7 | Hierarchy 面板（树 + 增删 + 重命名 + 拖拽改父级） | B3、D3 |
| D8 | Inspector 面板（从 `ComponentMeta::DrawInspector` 生成 UI） | B3、D3 |
| D9 | Content Browser（目录树 + 缩略图 + 拖拽到场景/Inspector） | C4、D3 |
| D10 | Console 面板（日志 sink 接 ImGui + 命令输入） | G2、D3 |
| D11 | Stats 面板（渲染/物理/内存统计曲线） | A10、G1 |
| D12 | Play / Pause / Step + 快照隔离 | B5、B6、D3 |
| D13 | `CommandHistory` + 三类命令（改属性/增删实体/拖拽） | D8 |
| D14 | Gizmo（平移/旋转/缩放 + 吸附） | D6、A6 |
| D15 | 多选 + 框选 + 批量编辑 | D6 |
| D16 | Scene Settings 面板 | B5 |
| D17 | 自动布局持久化（`imgui.ini`）与工作区预设 | D4 |
| D18 | 编辑器快捷键（Ctrl+S/Z/Y/D、F 聚焦、Delete 删除） | D13 |

### 6.5 验收标准

| 验收 | 判定 |
|---|---|
| 完整流程 | 打开场景 → 新建实体 → 加 SpriteRenderer + Rigidbody2D → 拖到位置 → 改颜色 → 存盘 → 重启 → 完全一致（F9） |
| Play/Stop 无损 | 点 Play 玩 30 秒，点 Stop，**场景与编辑前逐字段一致**（用 §4 的往返比较工具验证） |
| Undo 完整 | 任意编辑操作序列，连续 Undo 到底再 Redo 到顶，场景与初始一致 |
| Gizmo 正确 | 在缩放相机（0.5x / 2x）下拖拽 100 像素，世界位移与预期一致（误差 < 1px） |
| 面板不卡 | P5：5,000 实体场景，Hierarchy 展开 + Inspector 显示，UI 帧耗时 < 8ms |
| 中文可用 | ImGui 界面显示中文不乱码；Console 显示中文日志正常 |
| 无 ImGui 崩溃 | 反复开关面板、切换场景 100 次不崩 |

---

## 7. 模块升级详解 E：C# 脚本系统

> 本节的技术细节（.NET 版本、NativeAOT 导出写法、热重载的真实能力）已按联网调研核实，完整报告归档在 **[`docs/research_csharp_embedding.md`](research_csharp_embedding.md)**（807 行、91 条引用、5 段可复制代码、11 项"未能验证"清单）。**动手写代码前请先读那份报告。**

### 7.0 前置事实：目标 .NET 版本（已核实）

来源：微软官方文档 [Releases and support for .NET](https://learn.microsoft.com/en-us/dotnet/core/releases-and-support)（页面更新于 2026-05-15）+ [dotnet/core/releases.md](https://github.com/dotnet/core/blob/main/releases.md)

| 版本 | 支持类型 | 状态与时间线 | 结论 |
|---|---|---|---|
| **.NET 10** | **LTS** | GA 2025-11-11，最新补丁 10.0.12（2026-09-08），EOL **2028-11-14** | **脚本层应以此为目标 TFM**（`net10.0`） |
| .NET 11 | STS | **RC 阶段**（11.0.0-rc.1，2026-09-08），GA 2026-11-10，EOL 2028-11-09 | 现在不要用（RC），2026-11 之后可作为"尝鲜"选项 |
| .NET 9 | STS | EOL **2026-11**（约 2 个月后） | 本机当前只装了 SDK 9.0.305；**不要把它作为长期目标** |
| .NET 8 | LTS | EOL 2026-11 | 不要用（比 9 更早到期） |

**行动项（写进 R5 的第一个任务）**：

1. **安装 .NET 10 SDK，脚本工程 TFM 用 `net10.0`。** 别用本机的 9.0.305 作目标——它在 R5 开始前就会失去安全更新支持，等于一开始就选了过期运行时。（若 R5 恰好在 2026-11 之后开工，可评估 .NET 11；否则稳在 10 LTS。）
2. 在脚本工程里开启 `CheckSdkVulnerabilities`（MSBuild 属性，SDK 处于生命周期末尾时构建给出 `NETSDK1239` 警告），避免以后再踩同样的坑。
3. 把"目标 .NET 版本"写进 `README.md` 与 CI 环境说明，避免换机器时版本漂移。

### 7.1 先定义边界（与语言无关）

```cpp
// Scripting/ScriptInterface.h —— 这是真正的设计工作，换语言也不用改
class IScriptEngine {
public:
    virtual ~IScriptEngine() = default;
    virtual bool  Initialize(const ScriptEngineConfig&) = 0;
    virtual void  Shutdown() = 0;
    virtual bool  LoadAssembly(const std::filesystem::path&) = 0;
    virtual bool  ReloadAssembly() = 0;                       // 热重载
    virtual ScriptHandle Instantiate(std::string_view class, Entity) = 0;
    virtual void  Destroy(ScriptHandle) = 0;
    virtual bool  HasClass(std::string_view) const = 0;
    virtual std::vector<std::string_view> GetClassNames() const = 0;  // 编辑器下拉用
    virtual void  Update(float dt) = 0;                        // 批量调用所有实例
    virtual void  OnEvent(ScriptHandle, const ScriptEvent&) = 0;
};
```

**引擎暴露给脚本的 API（宿主 API 表）**：用一个 C 结构体把函数指针交给脚本层，**不要用 `DllImport` 反向依赖 exe 符号**（更慢、更难版本化、AOT 下受限）：

```c
// host_api.h —— 纯 C ABI，脚本层按名字/序号取用
typedef struct BambooHostAPI {
    uint32_t abi_version;              // 版本号，脚本层先检查
    // 实体
    uint64_t (*entity_create)(const char* name);
    void     (*entity_destroy)(uint64_t e);
    bool     (*entity_valid)(uint64_t e);
    // 组件（用注册表 ID + 类型名）
    void*    (*component_get)(uint64_t e, const char* typeName);
    bool     (*component_has)(uint64_t e, const char* typeName);
    bool     (*component_add)(uint64_t e, const char* typeName);
    void     (*component_remove)(uint64_t e, const char* typeName);
    // 输入
    bool     (*input_key_down)(int keyCode);
    void     (*input_mouse_pos)(float* outX, float* outY);
    // 日志
    void     (*log_info)(const char* msg);
    void     (*log_error)(const char* msg);
    // 资源
    uint64_t (*texture_load)(const char* path);   // 返回 handle
    // 场景
    void     (*scene_load)(const char* name);
} BambooHostAPI;

// 脚本层必须导出这个入口，宿主用它换取自己的 API 表
typedef void (*BambooScriptInitFn)(const BambooHostAPI* host, uint32_t hostAbiVersion);
```

**为什么这样设计**：
- **纯 C ABI**：C# / Lua / 未来的 Rust 都能用同一张表。
- **`abi_version` 检查**：脚本 DLL 与引擎版本不匹配时**明确报错**，而不是随机崩溃。
- **`component_get` 返回裸指针 + 类型名**：配合 §4 的组件注册表，脚本能访问的组件 = 注册过的组件，**自动同步**，不需要为每个组件写绑定代码。这是"用注册表换掉手写绑定"的关键收益。
- **`component_get` 的对象生命周期**：指针只在本次调用有效（**不能**跨帧缓存），在文档和头文件里写死这条规则。

**实体寻址必须用"代际句柄"，不要用裸指针**

```c
// 64-bit 句柄：低 32 位是 slot 索引，高 32 位是 generation（代数）
typedef uint64_t EntityHandle;   // 例：0x0000_0003_0000_0011
```

- 引擎侧维护 `slot → 对象` 的数组 + 每个 slot 的 generation 计数器；销毁时 generation++。
- **好处**：能检测"悬垂句柄"（generation 不匹配即报错，而不是访问已释放内存）；**句柄是纯整数，所以热重载后依然有效** —— 这对 §7.4 的 ALC 重载是关键性质（如果用裸指针，重载后所有缓存指针都失效）。
- 绝不允许把 C++ 对象指针包装成托管对象送进 GC；反过来 C# 对象要被 C++ 长期持有，必须显式 `GCHandle` 钉住并**记得 `Free()`**（官方文档明确警告"忘记释放即泄漏"），且**外部强 `GCHandle` 会阻止 ALC 卸载**（§7.4）。

### 7.2 组件访问的两条路（重要取舍）

| 方案 | 做法 | 优点 | 缺点 |
|---|---|---|---|
| **裸指针 + 注册表**（推荐起步） | `component_get` 返回 `void*`，脚本侧用 `unsafe` + 结构体映射 | 零绑定代码、零序列化开销、任何注册组件自动可用 | C# 侧要手写结构体布局（必须和 C++ 一致，错了就是内存错误） |
| **生成绑定层** | 从组件注册表/C++ 头文件生成 C# 结构体 + 访问器 | 类型安全、IDE 补全、不怕布局错 | 需要写生成器（约 1 周），但之后一劳永逸 |

**建议**：先用"裸指针 + 手写 C# 结构体"跑通（1~2 个组件验证），**同时用 `static_assert(sizeof(T) == N)` 在 C++ 侧锁住布局**，并写一个 C# 侧自检（启动时比较 `Marshal.SizeOf<T>()` 与宿主上报的 `sizeof`）。等组件超过 5 个再上生成器。

### 7.3 生命周期

```
C++ 侧实体生命周期                脚本侧
CreateEntity + AddComponent        →  查 ScriptComponent.class → 实例化脚本对象 → Awake()
每帧                               →  Start()（第一次 Update 前，仅一次）
                                   →  Update(dt)
                                   →  LateUpdate(dt)（渲染/物理之后）
实体销毁 / 场景卸载                →  OnDestroy() → 释放脚本对象
组件/属性被编辑器修改              →  OnValidate()（可选，只编辑器）
物理碰撞                           →  OnCollisionEnter/Stay/Exit(collisionInfo)
```

**执行顺序要显式定义并写进文档**：`所有 Awake → 所有 Start → 逐个 Update（按实体 ID 稳定序）→ 物理 → 逐个 LateUpdate`。`Update` 的实体顺序必须**稳定**（否则帧间行为不确定，调试噩梦）。

### 7.4 热重载：必须诚实面对的现实

这是 C# 嵌入最重要、也最容易被低估的问题。**结论先说：NativeAOT 与"改脚本不重启"是互斥的。**

**已核实的事实（微软官方文档明确写了）**：

- **NativeAOT 编译出的原生库不支持卸载**：官方文档原文为 *"Unloading Native AOT libraries (via `dlclose` or `FreeLibrary`, for example) is not supported."* —— 见 [Building native libraries with NativeAOT](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/libraries)。NativeAOT 的官方示例 README 也复述了这条限制：[dotnet/samples NativeLibrary README](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/README.md)。
- **AOT 程序集不支持热重载**（微软测试基础设施的原话）：[dotnet/runtime#54617](https://github.com/dotnet/runtime/issues/54617)。
- NativeAOT 还**禁用 `Assembly.LoadFile` 与 `Reflection.Emit`** —— 见 [Native AOT deployment](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)。

**这意味着**：选 NativeAOT = 每次改脚本都要**重编译 + 重启编辑器**（或整个宿主重载）；选"秒级热重载" = **必须走 CoreCLR 托管**。

| 路线 | 机制 | 热重载能力 | 启动/性能 | 分发 |
|---|---|---|---|---|
| **NativeAOT 导出原生库** | 把 C# 编译成原生库，导出 C 函数 | **不可能**（官方不支持卸载）→ 只能重编译 + 重启 | 启动快、无 JIT 预热、性能接近 C++、**无运行时依赖** | 单文件，最简 |
| **CoreCLR 托管（hostfxr/nethost）** | 进程内启动 .NET 运行时，用**可卸载 ALC** 加载脚本程序集 | **可行**（卸载旧 ALC → 加载新的，亚秒级） | 启动稍慢、有 JIT 预热、GC 在进程内 | **需要随包分发 .NET 运行时** |

**CoreCLR 路线的两个坑（官方文档已写明，务必避开）**：

1. **不要用 `load_assembly_and_get_function_pointer` 那条路的返回指针做热重载**。微软在 [native-hosting 设计文档](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md) 里明确写着：该委托拿到的函数指针**没有释放方式**（*"Currently there's no way to unload the managed component or otherwise free the native function pointer"*），生命周期等同进程。
   → **正确做法**：用 `hdt_load_assembly` 把程序集加载进**你自己的 collectible `AssemblyLoadContext`**，之后通过反射 / `MethodInfo` 调用。卸载 ALC 时脚本侧一切失效。
2. **hostfxr/nethost 托管 API 只支持框架依赖（framework-dependent）部署**（见 [Host .NET from a native process](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)）——**不能用这条路分发自包含运行时**。这直接影响 R7 的打包方案（要么让用户装运行时，要么换分发方式，要么发布版改走 NativeAOT）。

**⚠️ 最关键的一条工程约束：跨边界函数指针只能指向"不可回收的桥接程序集"**

这条如果不遵守，CoreCLR 路线的热重载**必然失败**（而且是静默失败）。原因是函数指针的"来源"决定了 ALC 能否卸载：

| 拿函数指针的方式 | 对 ALC 卸载的影响 | 后果 |
|---|---|---|
| `Marshal.GetFunctionPointerForDelegate(delegate)` | 运行时为该委托生成 loader-allocator stub 并持有**强句柄** | **ALC 永远卸不掉**（内存单向增长） |
| 裸 `delegate* unmanaged<...>`（`&Method`） | 不产生 GC 引用，**不阻止卸载** | ALC 一死，指针立刻**悬垂** → 下次调用**必然崩溃** |

**两条路都通向灾难**。所以正确的架构是：

```
引擎进程
 ├─ C++ 宿主（glfw / 渲染 / 物理 / ECS）
 ├─ Bootstrap 程序集（不可回收，走 Default ALC）   ← 跨边界函数指针只能指向这里的代码
 │    暴露：LoadScripts(path, HostApi*) / ReloadScripts() / Tick(dt) / Shutdown()
 │    内部：创建 collectible ALC → LoadFromAssemblyPath → 反射取 MethodInfo
 └─ 脚本程序集（collectible ALC，可整块丢弃）      ← 只放游戏逻辑，绝不被 C++ 直接持有指针
      只能通过 HostApi 表回调宿主（表指向 C++ 函数，永远不在 ALC 内）
```

- **C++ 侧只认识一个稳定的 bootstrap 方法指针**（用 `hdt_get_function_pointer` 拿到，.NET 5+），整个热重载逻辑写在 C# 侧；C++ 侧只调一个 `reload()`。
- **不要用 `hdt_load_assembly_and_get_function_pointer`**：它用的隔离 ALC 你拿不到引用、无法卸载，返回指针是进程生命周期且官方明说没有释放方式 —— **这条 API 不能用于热重载**。`hdt_load_assembly` 加载进 Default ALC（不可回收），同样不行。
- 卸载是**协作式**的，且**失败是静默的**（不报错，只是内存不释放）。已知的钉死来源：任何线程栈上有 ALC 帧、任意一侧的强 `GCHandle`、静态字段、事件订阅、`RegisteredWaitHandle`、**以及你自己的 ALC 子类的字段**（卸载期间运行时持有该 ALC 的强句柄，必须把它们置空）。
- **第一道防线**：每次重载后用 `WeakReference.IsAlive` + `GC.Collect()`/`WaitForPendingFinalizers()` 循环**断言旧 ALC 真的被回收**，不回收就告警。这条必须写进代码，不能靠"看起来没事"。

**已知的真实风险**：.NET 10 上出现过"反复 load/unload 脚本程序集 3~4 次后泛型虚派发挂死一个 CPU 核"的回归（[dotnet/runtime#132562](https://github.com/dotnet/runtime/issues/132562)，.NET 8.0.29 上 100 次正常）。这说明**反复重载在 .NET 上是测试覆盖不足的路径** —— 所以 §7.7 的"重载 50 次"和 CI 里的"连续重载 200 次"压测不是可选项。

**由这个限制推出的架构顺序**：

- **热重载是硬需求** → 必须走 **CoreCLR + 可卸载 ALC**，并且：
  - 脚本 → 宿主的调用**只能**通过宿主 API 表（§7.1），**不能**让宿主长期持有脚本侧的函数指针；
  - 宿主每帧调用脚本 `Update` 的委托/`MethodInfo` **必须在卸载 ALC 前释放**；
  - 脚本实例是纯托管对象，卸载 ALC 后全部失效 → 重载后**重新实例化**（状态丢失，要写进文档，或用 §4 的序列化把 `fields` 存下来恢复）。
- **接受"重编译式重载"** → 可以选 NativeAOT，但要明确：改一次脚本 ≈ 一次 AOT 编译（秒级到十几秒）+ 编辑器/宿主重启，**迭代体验会明显变差**。

**推荐的组合方案（编辑器用 B、发布用 C）**：**编辑器期走 CoreCLR 拿热重载，发布期走 NativeAOT 拿"零依赖单文件 + 高性能"**。代价是要维护两条脚本构建路径，但两者的**脚本源码完全一致**——因为边界（§7.1）是纯 C ABI，不绑定任何一条路线。这正是"先把边界做对"的回报。

**如果不想背 .NET 运行时分发包袱，还有第三条路（B'）**：

> **NativeAOT 脚本 + 独立的"脚本宿主子进程"**
>
> 脚本层编成一个**独立进程**（NativeAOT 或 CoreCLR 都行），通过 IPC（命名管道 / 本地 socket / 共享内存环）和引擎通信；编辑器里点"重载"= **杀掉并重启脚本宿主进程**（毫秒级），引擎进程与场景状态**完全不受影响**。
>
> - 优点：绕开"原生库不可卸载"的限制；引擎进程无需 .NET 运行时；崩溃隔离（脚本崩了不拖垮编辑器）。
> - 缺点：所有跨边界调用变成 IPC（要设计好批量协议，否则 P7 的性能目标难达）；架构更复杂。
> - 适用：**脚本只负责游戏逻辑、不需要每帧高频访问组件**的场景——对 2D 游戏通常够用。
>
> 这条路把"热重载"从"运行时能力"变成"进程管理"，是本项目里我认为**值得认真考虑**的替代方案（尤其是你最终想发布 NativeAOT 时）。

### 7.5 工作分解

| # | 任务 | 依赖 |
|---|---|---|
| E1 | `IScriptEngine` 接口 + `ScriptComponent`（`class` 名 + `fields` JSON） | B3 |
| E2 | 宿主 API 表（§7.1）+ `abi_version` 校验 | B3 |
| E3 | 组件注册表暴露"按名字取组件"的查询（含 `sizeof` 上报，供脚本自检） | B3 |
| E4 | **Lua/sol2 原型**：用最少代码把 E1+E2 跑通，验证边界是否够用 | E1、E2、E3 |
| E5 | C# 运行时工程（`Bamboo.Scripting`）：csproj + 构建脚本 + 输出约定 | E4 |
| E6 | 宿主加载脚本程序集 + 类发现（反射枚举脚本基类的子类） | E5 |
| E7 | 生命周期回调接线（Awake/Start/Update/LateUpdate/OnDestroy） | E6 |
| E8 | 组件访问（`component_get` + C# 侧结构体 + `static_assert` 布局锁 + 启动自检） | E6 |
| E9 | 输入 / 日志 / 场景 的绑定 | E6 |
| E10 | **热重载**（按 §7.4 的路线实现 + 卸载正确性压测） | E7 |
| E11 | 脚本字段（`fields`）序列化 + 编辑器里自动生成字段编辑器 | E1、B3、D8 |
| E12 | 异常处理：脚本异常 → 捕获 → 日志带托管堆栈 → **禁用该脚本实例**（不让一个坏脚本拖垮引擎） | E7 |
| E13 | 性能：跨语言调用开销测量 + 批量 Update 优化 | E7 |
| E14 | 脚本 API 文档 + 示例脚本集 | E7 |
| E15 | （可选）C# 绑定生成器 | E8 |
| E16 | **架构 spike（正式开发前，限 3 天，做不完就换方案）**：按 §7.4/§7.6 选定路线，**必须**跑通"加载 → 卸载 → 重载 50 次，内存不增长"，并产出数字到 `docs/perf/`。若选 CoreCLR：这一步就要验证 `WeakReference.IsAlive` 能观察到 ALC 归零 | E4 |
| E17 | **桥接程序集骨架（CoreCLR 路线的命门）**：建立不可回收的 `Bamboo.Bootstrap` 程序集，暴露 `LoadScripts / ReloadScripts / Tick / Shutdown`；C++ 侧只持有它的一个稳定方法指针；**所有跨边界函数指针只允许指向这里**（§7.4） | E16 |
| E18 | **ALC 卸载护栏**：每次重载后 `GC.Collect()` + `WaitForPendingFinalizers()` + `WeakReference.IsAlive` 断言；失败即告警（静默泄漏的第一道防线）；重载时重置所有静态状态 | E17 |
| E19 | **绑定方式的 ABI 约束**：`component_get` 返回的指针必须是**纯 C 结构体数据**，**不能**是含 C++ 虚函数表的对象（跨语言传虚表在 AOT 下有额外约束且宿主无法版控）；组件布局变更要能被 §7.2 的自检立刻发现 | E8 |

### 7.6 需要你决策：脚本语言路线

> ### ✅ 已决策（2026-09-18）：选 **B（CoreCLR + 可卸载 ALC）**，脚本语言 = **C#**
> 发布阶段（G5/v1.0）再评估 **D**（同一份脚本源码加一条 NativeAOT 构建路径）。下表保留作为决策依据。
> **不做 A（只用 Lua）**，但 Lua 原型仍可作为边界验证的可选步骤。

| 选项 | 热重载 | 上手成本 | 性能 | 分发复杂度 | 学习价值 | 适合 |
|---|---|---|---|---|---|---|
| **A. 只用 Lua** | 极好（亚秒级） | 1~2 天 | 中 | 低（自带源码） | 嵌入/栈式 API | **想尽快有脚本、专注游戏逻辑** |
| **B. CoreCLR + 可卸载 ALC** | **好（亚秒级）** | 2~3 周 | 中高（JIT） | **中高（要分发运行时，且只能框架依赖部署）** | .NET 托管、IL、ALC 生命周期 | **要 C# 且要热重载**（推荐） |
| **B'. NativeAOT + 脚本宿主子进程** | **好（重启子进程，毫秒级）** | 3~4 周 | 中（受 IPC 限制） | **低（引擎侧零依赖）** | 进程隔离、IPC、崩溃隔离 | **要零依赖分发 + 要热重载**（见 §7.4） |
| **C. NativeAOT 导出原生库** | **不可能**（官方不支持卸载）→ 重编译 + 重启 | 2~3 周 | 高（AOT） | **最低（零运行时依赖单文件）** | 原生 ABI、AOT 约束 | **发布形态**，不适合编辑器迭代 |
| **D. B（编辑器）+ C（发布）** | 编辑器热重载，发布零依赖 | 3~4 周 | 高 | 中 | 两条都学 | **理想终态**（脚本源码不用改，只换构建目标） |

**建议**：`E4（用 Lua 验证边界）→ B（拿热重载）→ R7 时再评估 D（加一条 NativeAOT 发布路径）`。

**为什么不直接上 C**：你的目标是"慢慢升级成一个可用的引擎"，而**迭代速度**是这类项目最大的杠杆。NativeAOT 会把"改一行脚本 → 看效果"从 1 秒变成十几秒 + 重启，这个代价会在几百次迭代里累积成数天。等游戏逻辑稳定、要发布时，再切 D 的 C 路线才有意义。

**为什么不直接用 A（停下 Lua）**：如果你确认"只想做游戏、不想练 .NET 嵌入"，那么停在 A 是**完全合理**的工程决策——Lua 做 2D 游戏逻辑够用，且省下 2~4 周。这个取舍只有你能定。

### 7.7 验收标准

| 验收 | 判定 |
|---|---|
| 基本可用 | 用 C#（或 Lua）写一个"按键左右移动 + 边界反弹"，能跑（F10） |
| 热重载 | 改脚本数值/逻辑 → 重载 → 新逻辑生效（< 1 秒 CoreCLR / < 15 秒 NativeAOT） |
| 状态隔离 | 热重载后脚本实例状态按文档定义的行为表现一致 |
| 卸载正确 | 反复重载 **50 次**内存不增长、不崩；**CI 里跑连续 200 次**（针对 [dotnet/runtime#132562](https://github.com/dotnet/runtime/issues/132562) 那类"反复重载才暴露"的 bug） |
| 卸载可观测 | 每次重载后 `WeakReference.IsAlive` 断言旧 ALC 已回收；**回收失败要告警**（不是静默通过）——静默泄漏是 ALC 路线最大的坑 |
| 异常隔离 | 脚本抛异常 → 引擎不崩、日志有托管堆栈、该实例被禁用、其他脚本继续跑 |
| 布局自检 | 故意改错一个 C# 结构体字段 → 启动时**明确报错**而不是随机崩溃 |
| 性能 | P7：1,000 脚本实体 Update 总耗时 < 2ms |
| 版本不匹配 | 用旧版脚本 DLL 配新版引擎 → 明确报 ABI 版本错误 |

---

## 8. 模块升级详解 F：测试体系

### 8.1 分层策略

```
┌─ 性能基准（benchmark）──── 数字回归：帧耗时/DrawCall/物理步进，阈值超标即红
├─ 渲染快照（snapshot）──── 参照场景截图比对（容差 1%），拦视觉回归
├─ 集成测试（integration）─ Scene+系统+序列化+资源的端到端（headless，无 GL）
├─ 单元测试（unit）──────── Math / ECS / 序列化 / 物理 / 工具（占 80% 数量）
└─ 静态检查（static）────── /W4+/WX、clang-tidy、分层守护脚本、.meta 完整性
```

**关键约束**：**单元/集成测试不依赖 GL 和窗口**。为此需要"headless 模式"——`Scene` 能脱离 `Window` 运行，渲染相关测试改用"CPU 侧批次内容断言"（渲染器支持一个 Null 后端或可注入的顶点消费者）。**这是让引擎可测的核心设计，不是测试的附属品。**

### 8.2 测试清单（按模块）

| 模块 | 测试内容 | 类型 |
|---|---|---|
| `Math` | Vector2/3/4 全运算符、Matrix3/4 乘法结合律与单位元、**`Matrix3 * Vector3` 与手算比对**、`RotateXYZ` 角度制、`AABB::Merge`、`Color::FromHex` 全格式、`Vector2::Cross` 标量语义、除零行为、`Inverse` 与 `Determinant` 一致性 | 单元 |
| `ECS` | 组件增删查、空 `Entity` 所有操作不崩、`DestroyEntity` 后 map 无残留、系统按阶段稳定排序 | 单元 |
| `Transform` | 层级矩阵、`Dirty` 传播、平移/旋转/缩放组合与手算一致 | 单元 |
| `序列化` | 往返字节一致、100 实体全字段往返、旧版本迁移、**fuzz（随机截断/篡改）不崩** | 单元+集成 |
| `物理` | 自由落体 vs 解析解、各形状两两碰撞检测、响应后的能量与动量、穿透修正、宽相与暴力法结果**完全一致** | 单元 |
| `资源` | GUID 稳定性（改名不断链）、引用计数增减、卸载后无泄漏、`.meta` 缺失检测 | 集成 |
| `场景` | 创建/销毁 10k 实体、`FindEntityByName`、Play/Stop 快照无损 | 集成 |
| `脚本` | 边界调用正确性、异常隔离、重载 50 次无泄漏、ABI 版本校验 | 集成 |
| `渲染` | 批次容量边界（画 N 个恰好换批）、顶点数据正确性（CPU 侧断言）、截图快照 | 单元+快照 |
| `编辑器` | 命令 Undo/Redo 一致性、Gizmo 数学（纯函数部分） | 单元 |

### 8.3 工作分解

| # | 任务 | 依赖 |
|---|---|---|
| F1 | GoogleTest vendored（或 FetchContent）+ `Source/Tests` + `BAMBOO_BUILD_TESTS` 开关 | — |
| F2 | Headless 模式（`Scene` 不依赖 Window/GL）+ 渲染器 Null 后端或可注入批次消费者 | A1 |
| F3 | 单元测试：Math（最高优先，含所有已知 P1 bug 的回归） | F1 |
| F4 | 单元/集成测试：ECS + Transform | F1 |
| F5 | 集成测试：序列化往返 + fuzz | B5、B6 |
| F6 | 单元测试：物理（积分 + 碰撞 + 响应） | 物理模块 |
| F7 | 集成测试：资源（GUID/引用/泄漏/热重载） | C4 |
| F8 | 渲染快照测试框架（渲染参照场景 → 保存 PNG → 与基线比对） | A8 |
| F9 | 性能基准框架（固定场景 + 计时 + CSV + 阈值判定） | A10、G1 |
| F10 | CI（GitHub Actions）：configure → build → ctest → benchmark 对比 | F1、F9 |
| F11 | 覆盖率统计 + 覆盖率门槛 | F10 |
| F12 | 静态检查：`/WX`、clang-tidy、分层守护脚本、`.meta` 完整性检查 | — |

### 8.4 验收标准

| 验收 | 判定 |
|---|---|
| 全绿 | `ctest` 100% 通过（E2） |
| 覆盖率 | 总行覆盖 ≥ 70%；Math/ECS/序列化/物理 ≥ 85% |
| 回归防护 | 把 `Matrix3::operator*(Vector3)` 故意改回错误版本 → **测试必须红**（验证测试真的有效） |
| fuzz 不崩 | 10,000 个随机损毁的场景文件，零崩溃 |
| 性能门槛 | 基准超标（如帧耗时 +20%）→ CI 红 |
| 无 GL 依赖 | 单元/集成测试在无显卡/无窗口的 CI 环境能跑 |

---

## 9. 模块升级详解 G：调试工具与可观测性

### 9.1 内容

| 工具 | 内容 | 优先级 |
|---|---|---|
| **Debug 叠层（HUD）** | FPS / 帧耗时（含最大/平均）/ DrawCall / 批次数 / 实体数 / 物理步进耗时 / 内存 / 鼠标世界坐标 | P0 |
| **控制台 + 命令系统** | `CommandRegistry`：`scene.load <name>`、`entity.list`、`physics.debug on`、`renderer.stats`、`timescale 0.5`；命令可注册，脚本也能注册 | P0 |
| **时间控制** | 暂停 / 单步 / 时间缩放 / 慢动作（调试物理必备） | P0 |
| **场景调试绘制** | 碰撞体线框、速度矢量、网格、原点/相机框、UI 边界 | P1 |
| **日志系统升级** | 分级可配、按模块 channel、文件 sink + 轮转、异步写入、ImGui Console sink | P0 |
| **崩溃报告** | SEH 异常处理器 + minidump + 自动记录最近日志与场景路径 | P1 |
| **运行时资源检查** | 未释放资源清单、纹理总显存估算、DrawCall 明细 | P1 |
| **自动化截图/录像** | 启动参数 `--screenshot <scene> <out.png>`、固定帧率录制 GIF/MP4 | P1 |
| **性能计时器** | `BAMBOO_PROFILE_SCOPE` 宏 + 层级计时收集 + 输出到 HUD/CSV | P0 |

### 9.2 验收标准

| 验收 | 判定 |
|---|---|
| HUD 准确 | 显示的 DrawCall / 实体数 / FPS 与外部工具读数一致（P1、P4 的数据来源就是它） |
| 命令可用 | 控制台能执行 5 个以上命令并即时生效 |
| 时间控制 | 暂停后物理完全静止；`timescale 0.2` 下慢动作平滑 |
| 崩溃可诊断 | 故意制造空指针 → 生成 minidump + 日志尾部含出错场景路径，可在 VS 里直接打开定位 |
| 计时可用 | 在每个系统外层加 `BAMBOO_PROFILE_SCOPE`，能看出哪一步最慢 |

---

## 10. 总验收：垂直切片 demo

**用一个贯穿所有模块的小游戏一次性验收整条链路。** 建议目标：**一个 2D 横版小平台跳跃**（比 Breakout 更能压测：需要关卡编辑、物理、脚本、资源、相机跟随、UI 文字）。

| # | 内容 | 验收点 |
|---|---|---|
| 1 | 角色：贴图精灵 + 刚体 + 碰撞盒 | F4、F5 |
| 2 | 角色控制：C#（或 Lua）脚本，跑/跳/重力/落地判定 | F10、P7 |
| 3 | 关卡：20 个平台 + 10 个金币 + 危险区，**全部在编辑器里摆** | F9 |
| 4 | 相机跟随：平滑跟随 + 边界限制 | A9 |
| 5 | 收集金币：碰撞触发器 + 计分 | F5、物理触发器 |
| 6 | HUD：分数文字（中英）+ 生命 | A12 |
| 7 | 音效（若做了音频模块） | — |
| 8 | 关卡存盘 → 重启编辑器 → 加载 → 一致 | F6、F7 |
| 9 | 改一张贴图 → 热重载生效 | F8 |
| 10 | 改脚本数值 → 热重载生效 | F10 |
| 11 | 一键打包 → 干净机器运行 | F11 |
| 12 | 全程 60 FPS，性能数字记录在 `docs/perf/` 下 | P2、E5 |

**这个 demo 就是"可用引擎"的证明。** 建议把它做成 `Sandbox/Games/Platformer/`，并在 README 里放 GIF。

---

## 11. 阶段路线图（R0~R7）与依赖

> ### ⚠️ 本节已被 `docs/games_and_editor.md` 的 §4 覆盖（2026-09-18 决策更新）
>
> 已确认的方向：**游戏优先 + 编辑器必须能用（Unity 式）+ 脚本走 C#**。据此对下面的 R0~R7 做了三处调整：
>
> | 调整 | 原因 |
> |---|---|
> | **编辑器从 R4 提前到 R2** | 它是"造游戏"的直接生产力工具。G2（第二个游戏）就要用它摆关卡，等不到第 4 位 |
> | **脚本系统拆成 R3a（提前，服务 G1）+ R3b** | G1 的验收标准就是"游戏逻辑用 C# 写"，所以最小脚本必须紧跟渲染之后；而且它是**风险最高的模块**，越早证伪越好 |
> | **资源热重载从 R1 降级到 R4** | 游戏早期不需要它；先保证"路径可移植"即可 |
>
> **以 `docs/games_and_editor.md` §4 的版本路线图（v0.1.0 ~ v1.0.0，以游戏为里程碑）为准。** 本节的 R 编号仍可用于查阅各模块的任务清单与出口条件。

命名用 `R`（升级阶段）以区别于 `refactor_plan.md` 的 `S`（止血/修复阶段）。**R0 直接承接 S0~S3**。

```
refactor_plan.md:  S0 止血 → S1 渲染 → S2 应用骨架 → S3 ECS/场景
                                                            │
本文件:  R0 数据基座 ────────────────────────────────────────┤
              │                                              │
              ├─→ R1 资源与热重载 ──┐                         │
              ├─→ R2 测试与 CI ─────┤（与 R1 并行）           │
              │                     ↓                        ↓
              └─→ R3 渲染升级 ──→ R4 编辑器 ──→ R5 脚本系统 ──→ R6 物理完善 ──→ R7 发布
```

| 阶段 | 名称 | 交付物（必须有 demo） | 出口条件 | 依赖 | 估时 |
|---|---|---|---|---|---|
| **R0** | 数据基座 | JSON + 组件注册表 + 场景存读 + 往返测试 | 100 实体场景往返字节一致；损坏文件不崩 | S3 | 1 周 |
| **R1** | 资源与热重载 | `.meta`/GUID + AssetManager 重写 + 热重载 | 改名不断链；改贴图 2 秒生效；进出场景 20 次无泄漏 | R0 | 1.5 周 |
| **R2** | 测试与 CI | gtest + headless + CI + 覆盖率 + 性能基线 | `ctest` 全绿；覆盖率达标；CI 拦回归 | R0（可与 R1 并行） | 1.5 周 |
| **R3** | 渲染升级 | 批次重构 + FrameBuffer + 相机 + 图元 + 文字 + 统计 | F2/F4 通过；P1/P2 达标；GL 零错误 | S1 | 1.5 周 |
| **R4** | 编辑器 | Layer + 6 个面板 + Play/Stop + Undo + Gizmo | F9 完整流程；Play/Stop 无损；P5 达标 | R0、R1、R3 | 3~4 周 |
| **R5** | 脚本系统 | **先 3 天 spike 验证重载可行性** → 边界 + Lua 验证 + C#（按 §7.6 选型）+ 热重载 | F10；P7；重载 50 次无泄漏 | R0、R4 | 2~4 周 |
| **R6** | 物理完善 | 窄相 + 响应 + 触发器 + 碰撞层 + 调试绘制 | F5 全部；P3 达标 | S3、R3 | 1.5~2 周 |
| **R7** | 发布与打磨 | 打包 + 崩溃报告 + 资源随包 + README/GIF + 干净机验证 | F11；垂直切片 demo 全过 | R4、R5、R6 | 1.5 周 |

**总计约 14~19 周**（单人、每周 15~20 小时投入）。若投入减半，按 7~9 个月规划。

### 11.1 建议的并行与穿插

- **R2 与 R1 并行**：测试"边做边写"最省力，且 R2 的 headless 能力是后续所有自动化的前置。
- **R3 的一部分（FrameBuffer）必须早于 R4**：编辑器视口依赖它。若想早点看到编辑器，可以先只做"FrameBuffer + 相机 + 批次修复"，把线/多边形/文字放到 R4 之后。
- **R0 必须早于所有人**：序列化是编辑器、脚本字段、Play/Stop、资源引用**四个模块的共同前置**。这也解释了为什么它排第一。

### 11.2 垂直切片的时间点

建议**在 R4 结束时做第一次垂直切片**（用 Lua 或硬编码逻辑代替脚本），这样能**提前 2~4 周**看到"引擎到底能不能用"，而不是等到最后。R5/R6 再把这个切片替换成脚本驱动 + 完整物理。

---

## 12. 工作量估算与里程碑

### 12.1 按模块汇总（人·天，单人）

| 模块 | 最小可用 | 完整 | 备注 |
|---|---|---|---|
| 渲染管线 | 5 | 12 | 批次重构是地基，必须先做 |
| 序列化 | 5 | 8 | 注册表是复用地基，别省 |
| 资源系统 | 6 | 12 | 热重载价值高，异步可后置 |
| 编辑器 | 12 | 25 | 最大的单块；Undo/Gizmo 是"专业度"分水岭 |
| 脚本系统 | 10（Lua） | 20~28（C#） | 边界设计占 1/3，别跳过 |
| 测试体系 | 6 | 12 | 与开发并行，不是收尾工作 |
| 调试工具 | 4 | 8 | 投入产出比极高，早做 |
| 打包发布 | 3 | 6 | |
| **合计** | **~51 天** | **~103 天** | 按每周 3 天投入 ≈ 5~8 个月 |

### 12.2 里程碑与"看得见的成果"

| 里程碑 | 时间点 | 你能演示什么 |
|---|---|---|
| **M1** | R0 末（+1 周） | 手工写 JSON 场景 → 引擎加载出画面；存盘再读一字不差 |
| **M2** | R1+R2 末（+3 周） | 改贴图 2 秒生效；`ctest` 全绿；CI 徽章 |
| **M3** | R3 末（+4.5 周） | 1080p 下 10,000 精灵 60 FPS；HUD 显示 DrawCall |
| **M4** | R4 末（+8 周） | **在编辑器里摆出一个关卡并试玩**（第一次真正"可用"） |
| **M5** | R5 末（+11 周） | 用 C# 写游戏逻辑并热重载 |
| **M6** | R6+R7 末（+14 周） | 一个能分发的小平台游戏 + GIF + 性能报告 |

---

## 13. 风险登记册

| # | 风险 | 影响 | 概率 | 缓解措施 |
|---|---|---|---|---|
| K1 | **C# 热重载需求落空** | 高（核心需求） | **已确认为硬约束**（非风险） | **已核实：NativeAOT 官方不支持卸载**（[文档](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/libraries)），与热重载互斥 → 按 §7.4 选 CoreCLR + 可卸载 ALC；并在 R5 第一个任务做 3 天 spike：跑通"加载→卸载→重载 50 次无泄漏"再投正式开发。**次要风险**：ALC 卸载被原生回调阻止 → 靠 §7.1 的"宿主不持有脚本函数指针"约束规避 |
| K1b | **CoreCLR 路线只能框架依赖部署**（不能自带运行时），影响发布 | 中 | 已确认 | 发布期改走 NativeAOT（§7.6 选项 D）；或用安装器检测/引导安装 .NET 运行时；**R7 之前必须做一次干净机验证** |
| K2 | **编辑器工作量失控**（面板无穷无尽） | 高（拖垮整个计划） | 高 | 严格按 §6.1 的 P0/P1/P2 分级；P0 之外**一律不做**；M4 之后才允许碰 P1 |
| K3 | **序列化格式反复改** | 中（返工） | 中 | R0 就把"版本号 + 迁移链 + 未知组件保留"三件事做对；先用 8 个组件验证覆盖度 |
| K4 | **物理越做越复杂**（想做"真"物理引擎） | 中（时间黑洞） | 中高 | 明确**不做**：软体/关节/连续碰撞/旋转摩擦精确解；只用"半隐式欧拉 + 冲量法 + SAT"，够 2D 平台游戏 |
| K5 | **性能优化过早** | 中 | 中 | 先出数字（R2 的基准框架），**对着数字优化**；R3 之后再谈批处理/多线程 |
| K6 | **脚本 C++/C# 结构体布局不一致** → 随机内存错误 | 高（极难调试） | 中 | `static_assert` 在 C++ 侧锁死 + 启动时 C#↔C++ 尺寸自检（§7.2） |
| K7 | **只有你一个人**，生病/忙碌就停摆 | 中 | 高 | 每阶段收敛到"可提交、可运行"状态；不留半成品分支；用文档（本文件）接续上下文 |
| K8 | **引擎没有"用户"**，做着做着失去方向 | 中（动力） | 中 | 用 §10 的垂直切片做北极星；每个里程碑产出可演示的 GIF |
| K9 | **图形 API 抽象层重构与 R3 冲突** | 低 | 中 | 按 `refactor_plan.md` D1 先决定降级或兑现，**在 S3.12 一次性做掉**，R3 只做能力不做搬迁 |
| K10 | 打包/运行时分发踩坑（.NET 运行时、VC++ 运行库） | 中 | 中 | R7 之前**至少做一次**完整的"干净机器"验证，别留到最后 |

---

## 14. Definition of Done（每个任务完成的定义）

每个任务（表格里的一行）只有在满足以下**全部**条件时才算完成。这条约定是为了防止"代码写完了但没法用"。

1. **能编译**：`/W4` 零警告、零错误（Release 与 Debug 都过）。
2. **有测试**：纯逻辑有单元测试；集成部分有集成测试；渲染部分有快照或 CPU 侧断言。**没有测试的功能视为未完成**（唯一例外：纯 UI 布局，用手动测试清单代替）。
3. **有验收证据**：在本文件对应章节的验收表里能填上"通过 + 具体数字/截图路径"。
4. **有文档**：公开 API 有注释；如果改变了使用方式，更新 `README.md` / `AGENTS.md` / 本文件。
5. **有提交**：独立分支、Conventional Commit 信息、`git status` 干净（无 `build/`、无临时文件）。
6. **没有留下死代码**：注释掉的旧实现删除（`git log` 里有）；`TODO` 必须带名字和具体内容。
7. **性能数字**：如果任务声明了性能目标，必须附前后对比数据（CSV/截图），不能只写"更快了"。
8. **不破坏已有验收**：改动后 F1~F11、P1~P7、E1~E6 中受影响的项目仍需通过。

---

## 附：文档分工

| 文档 | 定位 | 什么时候看 |
|---|---|---|
| `docs/architecture.md` | **现状架构**（对照代码核对过）：分层与依赖、启动/帧循环、系统阶段与执行顺序、组件清单、所有权模型、关键不变量、实际目录树 | 想知道"代码现在是什么样"时 |
| `docs/architecture_upgrade.md` | **架构决策文档**：五大架构根因（缺"执行顺序/对象生命周期/资源身份/数据模式/渲染契约"权威）、目标架构、A/B/C 迁移路径、架构守护脚本。**它修正了本文件 §11 的排序（资源身份须先于序列化）** | **决定"为什么这么改"时必读** |
| `docs/refactor_plan.md` | **体检报告**：37 项缺陷 + S0~S8 止血/修复阶段。⚠️ 这些是**症状**；根因见 `architecture_upgrade.md` | 修 bug、判断改动是否安全 |
| `docs/upgrade_plan.md`（本文件） | **施工蓝图**：七个模块怎么建 + R0~R7 升级阶段 + 验收标准 | 决定"下一步做什么" |
| `docs/games_and_editor.md` | **游戏计划 + 编辑器设计**：G1~G5 游戏阶梯（每个游戏验收一个引擎版本）、Unity 式编辑器布局与工作流、版本协同规则。**已确认"游戏优先 + 编辑器可用 + C# 脚本"，其 §4 覆盖本文件 §11 的排序** | 决定"先做哪个游戏 / 编辑器怎么做" |
| `docs/research_csharp_embedding.md` | **C# 嵌入技术调研报告**（807 行、91 条引用）：.NET 版本、NativeAOT 导出规则、hostfxr 序列、可回收 ALC 的完整护栏、5 段可复制代码、11 项未验证清单 | **动手做 R5 之前必读** |
| `docs/roadmap.md` | **学习路线**：每个 bug 对应哪个 C++ 知识点、GitHub 工作流。⚠️ **文件:行号已过期**，缺陷请以 `refactor_plan.md` 的编号为准 | 想学东西的时候 |

> **已删除**：`docs/project_structure.md`（原 22KB 架构文档）——它描述的是已不存在的代码（`SpriteRendererSystem`、旧的 `Scene::m_Systems` 双容器、把 `Vector4`/`Octree`/`SceneCamera` 说成已实现）。其仍有价值的结构信息已核对后并入 `docs/architecture.md`，作废清单见该文档 §9。
