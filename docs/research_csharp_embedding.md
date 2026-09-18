# 在原生 C++ 2D 引擎（Windows x64 / MSVC / C++17-20）中嵌入 C# 脚本
## 技术调研报告 —— 事实截止日 **2026-09-18**

> 调研方法：全部结论来自 2026-09-18 现场抓取的官方文档 / 官方仓库 / 一手 issue。
> 凡是我**未能验证**的，都在文末「未能验证项」里单独列出，并在正文对应位置标 **⚠️未验证**。
> **环境限制**：本机沙箱无法访问 NuGet（`Invoke-WebRequest https://api.nuget.org/v3/index.json` 失败），
> 因此**没能真机跑一次 NativeAOT 构建**去实测体积/构建时间/启动开销。相关数字全部标注来源。

---

## 0. 先说最重要的三条结论

1. **NativeAOT 与「热重载脚本」在技术上互斥。** Microsoft 文档原文：
   *"Unloading Native AOT libraries (via `dlclose` or `FreeLibrary`, for example) is not supported."*
   —— [Building native libraries](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/libraries)
   官方样例 README 再说一次：*"the .NET Runtime does not support unloading. Once a handle to the shared library is created, the library cannot be closed with `dlclose`/`FreeLibrary`."*
   —— [NativeLibrary sample README](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/README.md)
   Microsoft 自己的测试基建也写死了：*"hot reload is not supported for AOTd assemblies"*
   —— [dotnet/runtime#54617](https://github.com/dotnet/runtime/issues/54617)
   **所以「NativeAOT 编译的脚本 DLL」这条路，每次改一行脚本都必须重启引擎进程**（或重启承载脚本的进程）。
   这不是"难度大"，是设计上不提供。

2. **要真正热重载，唯一可行路径是托管运行时（CoreCLR）宿主 + 可回收 `AssemblyLoadContext`。**
   `hostfxr`/`nethost` 是当前官方推荐的宿主 API。但注意：`hdt_load_assembly_and_get_function_pointer`
   返回的函数指针**进程生命周期内无法释放、其 ALC 也无法卸载**，所以**不能用它做热重载**；
   必须自己用 `hdt_load_assembly`（.NET 8+）加载到自建 collectible ALC，再用反射取 `MethodInfo` 调用。
   —— [native-hosting.md](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md)

3. **`AssemblyLoadContext` 的卸载是"协作式"的，而且 .NET 10 上刚出现过反复重载场景的真实回归 bug。**
   —— [unloadability 文档](https://learn.microsoft.com/en-us/dotnet/standard/assembly/unloadability)、
   [dotnet/runtime#132562](https://github.com/dotnet/runtime/issues/132562)

---

## 1. 当前 .NET LTS / STS 版本（含确切版本号与日期）

事实来源：[dotnet/core `releases.md`](https://github.com/dotnet/core/blob/main/releases.md)、
[.NET 10 release notes](https://github.com/dotnet/core/blob/main/release-notes/10.0/README.md)、
[.NET 11 release notes](https://github.com/dotnet/core/blob/main/release-notes/11.0/README.md)

| 版本 | 类型 | GA 日期 | 最新补丁 | 支持截止 |
|---|---|---|---|---|
| **.NET 10.0** | **LTS** | **2025-11-11** | **10.0.12（2026-09-08）** | 2028-11-14 |
| **.NET 11.0** | **STS（当前最新/预览）** | **2026-11-10** | **11.0.0-rc.1（2026-09-08）** | 2028-11-09 |
| .NET 9.0 | STS | 2024-11-12 | 9.0.20 | 2026-11-10（即将 EOL） |
| .NET 8.0 | LTS | 2023-11-14 | 8.0.31 | 2026-11-10（即将 EOL） |

**结论（"mid-2026" 语境）：**
- **当前 LTS = .NET 10**（GA 2025-11-11，最新 10.0.12 发布于 2026-09-08）。**新项目就选它。**
- **当前最新版本 = .NET 11**，但**还在 RC**（11.0.0-rc.1，2026-09-08），GA 定在 2026-11-10。
  .NET 每年 11 月发一版；.NET 11 是 STS（支持 2 年）。
- 如果你在 2026 年 6 月看这份材料：当时 LTS 仍是 .NET 10（当月补丁 10.0.9 / 2026-06-09），
  11 处于 preview.5 阶段（2026-06-09）。
- .NET 8 / .NET 9 **都在 2026-11-10 EOL**，不要再用。
- 注意 .NET 11 里有一处与 NativeAOT 原生库相关的**破坏性变更**：Unix 上原生库输出默认加 `lib` 前缀
  （`libmylib.so`），可用 `UseNativeLibPrefix=false` 关闭。Windows 不受影响。
  —— [.NET 11 breaking change](https://learn.microsoft.com/en-us/dotnet/core/compatibility/interop/11/nativeaot-lib-prefix)

---

## 2. .NET NativeAOT「native library 导出」路径

### 2.1 csproj 实际设置

官方样例的最小可工作 csproj 只有 `<PublishAot>true</PublishAot>`：
—— [NativeLibrary.csproj](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/NativeLibrary.csproj)

但对你的场景，建议显式写全（下面是各属性的依据）：

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <OutputType>Library</OutputType>          <!-- 类库，不是 Exe -->
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks><!-- 导出签名要用指针 -->
    <Nullable>enable</Nullable>

    <!-- === NativeAOT 原生库三件套 === -->
    <PublishAot>true</PublishAot>             <!-- 开 AOT；官方文档 -->
    <NativeLib>Shared</NativeLib>             <!-- 出 .dll/.so/.dylib，而不是可执行文件 -->
    <SelfContained>true</SelfContained>       <!-- .NET 9+ 显式写上，见下方说明 -->
    <RuntimeIdentifier>win-x64</RuntimeIdentifier>

    <!-- === 体积/启动优化（可选）=== -->
    <InvariantGlobalization>true</InvariantGlobalization> <!-- 不带 ICU、省体积；若需区域性排序/格式化则别开 -->
    <OptimizationPreference>Speed</OptimizationPreference><!-- 见 aka.ms/OptimizeNativeAOT -->
  </PropertyGroup>
</Project>
```

依据与坑：

- `PublishAot`：*"This property enables Native AOT compilation during publish."*
  —— [Native AOT overview](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)
- `NativeLib`：**没有找到正式的 MSBuild 属性参考页**（⚠️未验证：官方 `msbuild-props` 文档里没查到）。
  它的取值 `Shared` / `Static` 只在样本 README 和 issue 里出现：
  *"`dotnet publish /p:NativeLib=Static --use-current-runtime`"* 与 *"`/p:NativeLib=Shared`"*
  —— [sample README](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/README.md)、
  [dotnet/runtime#69369](https://github.com/dotnet/runtime/issues/69369)。
  该 issue 的结论是：`OutputType != Exe` 时 SDK 会**默认**成 `Shared`，所以 `NativeLib` 通常可省；
  但显式写更安全（历史上有过 "No entrypoint module" 的 ILCompiler 报错）。
- **`SelfContained` 必须显式设**：.NET 9 起 `dotnet msbuild /restore -t:publish` 走 AOT 会失败：
  `error NETSDK1102: Optimizing assemblies for size is not supported for the selected publish configuration. Please ensure that you are publishing a self-contained app.`
  —— [dotnet/runtime#109154](https://github.com/dotnet/runtime/issues/109154)
  （`dotnet publish -r <RID>` 会自动推导 self-contained，用 `dotnet msbuild` 则不会。）
- **静态库官方不建议**：*"Only 'shared libraries' ... are supported. Static libraries are not officially supported and may require compiling Native AOT from source."*
  —— [Building native libraries](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/libraries)

### 2.2 确切 publish 命令行（win-x64）

```powershell
# 推荐（显式、可复现）
dotnet publish -c Release -r win-x64 `
  -p:PublishAot=true -p:NativeLib=Shared -p:SelfContained=true

# 官方样式的简写（也能出共享库）
dotnet publish --use-current-runtime -c Release
```

输出落在 `bin\Release\net10.0\win-x64\publish\<AssemblyName>.dll`。
—— [sample README](https://github.com/dotnet/samples/blob/main/core/nativeaot/NativeLibrary/README.md)

前置条件（Windows）：需要 VS 2022+ 的 **Desktop development with C++** 工作负载（提供 MSVC 链接器）。
—— [Native AOT prerequisites](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)
x64 与 MSVC 工具链位数必须一致（README 专门警告过 x86/x64 混用）。

### 2.3 函数如何导出：`[UnmanagedCallersOnly]` 的确切规则

**规则（官方 API 文档 + NativeAOT 文档 + 样本 README 三处一致）：**

| 规则 | 依据 |
|---|---|
| 必须是 `static` | [UnmanagedCallersOnlyAttribute](https://learn.microsoft.com/en-us/dotnet/api/system.runtime.interopservices.unmanagedcallersonlyattribute) |
| **不能被托管代码调用**（在 C# 里调它会抛异常） | 同上 + README *"cannot be called from regular managed C# code, an exception will be thrown"* |
| 参数必须 **blittable**（`byte/sbyte/short/ushort/int/uint/long/ulong/float/double/nint/nuint`、非托管指针、`Sequential`/`Explicit` 布局且字段全 blittable 的 struct） | 同上 + [blittable 说明](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#blittable-types) |
| 不能有泛型参数，不能位于泛型类中 | API 文档 |
| **不能用常规 C# 异常处理**，应改为返回错误码 | README |
| 引用类型参数必须自己手动 marshal（不能靠运行时） | README *"they have to marshal all reference type arguments"* |
| **必须给 `EntryPoint` 才会生成具名导出**（`EntryPoint` 省略 ⇒ 不导出符号） | API 文档 Fields 表 |
| **只导出「被发布程序集」里的方法**；项目引用 / NuGet 包里的方法**不会**被导出 | [Native code interop with Native AOT](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/interop) |

最后一条对引擎架构影响很大：**脚本层想给引擎暴露的每个入口，都必须写在那个被 publish 的程序集里。**

**`CallConvs` 什么时候需要？**
x64 / ARM64 上**只有一种调用约定，`CallConvs` 写了也没有效果**；只有 Windows x86 才需要区分 `Stdcall` / `Cdecl`：
*"On x64, ARM, and ARM64 architectures, there is only one calling convention, so specifying one explicitly is unnecessary."*
—— [Unmanaged calling conventions](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/calling-conventions)
所以你 win-x64 上写不写 `CallConvs = new[]{ typeof(CallConvCdecl) }` 都行（写了更"明确"、跨平台更安全）。
可用的还有 `CallConvSuppressGCTransition`（省掉 GC 转换开销）。**但要注意**：
搜索命中过一篇「给 P/Invoke 加 `SuppressGCTransition` 后把 `UnmanagedCallersOnly` 函数指针传给 C++ 导致崩溃」的
StackOverflow 帖，**我抓那页时被 403 挡住、没能读正文**（⚠️未验证）。
保守做法：只在"确定不会阻塞、不会触发 GC、不会回调进托管"的纯计算导出上考虑它，其余一律不用。

`CallConvMemberFunction` 也存在，可用于 C++ 成员函数指针风格。
—— [calling-conventions 文档](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/calling-conventions)

### 2.4 能不能不用 `UnmanagedCallersOnly`，走 `[DllImport]` 那种"普通 C 导出"？

**不能。** NativeAOT 的导出机制只有一条路：

> *"The Native AOT compiler exports methods annotated with `UnmanagedCallersOnlyAttribute` with a nonempty `EntryPoint` property as public C entry points. ... Only methods marked `UnmanagedCallersOnly` in the published assembly are considered."*
> —— [Native code interop with Native AOT](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/interop)

旁证：Microsoft Q&A 上"我 publish 成 native AOT DLL 了，为什么 dll 里没有函数？"的官方回答就是
*"Native aot libraries only support entry points with the `[UnmanagedCallersOnly()]` attribute."*
—— [Microsoft Q&A](https://learn.microsoft.com/en-gb/answers/questions/1659174/how-to-export-function-in-dll-to-aot)

至于第三方 `[DllExport]`（UnmanagedExports / DllExport NuGet 之类）：它们普遍依赖 **IL 重写 / JIT 侧 stub 注入**
来伪造导出（例如 `ilasm` 后处理、或运行时 hook），这与 NativeAOT 的"发布期全静态编译、无 JIT"模型天然冲突。
**⚠️未验证**：我没有找到任何一手资料说明某个 `[DllExport]` 实现能在 NativeAOT 下工作 —— 请当作"没有证据支持"，
不要依赖。

另一个副作用：`UnmanagedCallersOnly` 方法**自己不能被托管代码调用**。所以脚本层不能在 C# 内部直接调自己的导出函数，
必须从 C++ 侧绕回来（这其实是个好事：强制了单向的边界）。

### 2.5 启动开销 / DLL 体积 / 构建时间

**我能给出的最可靠一手数字（⚠️注意：是 exe 不是 dll，且是 hello-world）：**

| .NET | 默认 console hello-world 的 AOT 可执行文件体积 |
|---|---|
| .NET 7 | 3.65 MB |
| .NET 8 | 1.39 MB（−61.9%） |
| .NET 9 | 1.22 MB（−12.2%） |
| **.NET 10** | **1.05 MB（−13.9%）** |

—— [State of Native AOT in .NET 10, 2025-11-10](https://code.soundaranbu.com/state-of-nativeaot-net10)

**"小型 native library" 的体积/构建时间/首次调用开销：没有官方数字，我也没能实测（沙箱无 NuGet）。以下全部是估算或第三方数据，置信度都不高：**

- **体积**：一个几百行、只有几个导出的 NativeAOT **共享库**，按上面的 trimming 程度推断应在
  **1–3 MB 量级**。⚠️未验证（我自己未测）。
- **构建时间**：一个社区研究文档给出「Windows 上 NativeAOT 游戏应用 ≈ 2–3 分钟」、体积 ≈ 10–15 MB
  —— [keen-eye cross-platform deployment research, 2024-12](https://github.com/orion-ecs/keen-eye/blob/a83a9264f30b0f079e413a6bc9a5d6ab89671213/docs/research/cross-platform-deployment.md)
  （这份文档是仓库内的调研报告，非官方，且写于 2024-12）。你的小库应该**远快于此，估计 20–60 秒**，⚠️未验证。
- **启动/首次调用开销**：NativeAOT vs JIT 的**进程**启动对比在社区被反复测量为 **≈14–17 ms vs ≈70–80 ms（≈5x）**，
  —— 同上 keen-eye 文档；另有独立博客给出 "600ms → 50ms"、"约 70ms → 约 14ms" 的 API 冷启动数字，
  并明确说 *"Treat numbers like that as a direction, not a guarantee"*
  —— [vensas, 2026-08-14](https://vensas.de/en/blog/dotnet-native-aot-startup-cost)
  **注意**：这些是「进程启动」。NativeAOT **原生库**是 `LoadLibrary` 时做运行时/GC 初始化，
  **很可能比 exe 更快**，但我没有找到任何原生库形态的公开测量（⚠️未验证）。
  对你的引擎来说，这个开销只在 **引擎启动时付一次**（因为不能卸载），所以影响不大。
- 官方对数字的态度是刻意模糊的（"The benefit ... is most significant for workloads with a high number of deployed instances"）
  —— [Native AOT overview](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)

### 2.6 NativeAOT 的硬性限制（与引擎相关的）

官方限制清单 —— [Native AOT overview, Limitations](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)：

- **No dynamic loading**（`Assembly.LoadFile` 不可用）← 直接杀死"运行期换脚本程序集"
- **No runtime code generation**（`System.Reflection.Emit` 不可用）
- **No C++/CLI**、Windows 上 **No built-in COM**
- 必须 trimming（继承 trimming 的各种不兼容）
- 隐含 single-file
- `System.Linq.Expressions` **永远是解释模式**（比 JIT 慢很多）
- 泛型实例化全部预生成 ⇒ 磁盘体积膨胀
- 诊断/调试支持有限
- **平台限制**：Windows 官方支持 **x64 / Arm64**（.NET 9+ 表格）
- 另有一条与宿主互为镜像的：**native hosting 与 trimming 不兼容**（"Native hosting support on managed side is disabled by default on trimmed apps"）
  —— [native-hosting.md](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md)

> 附注：.NET 10.0.12 的包清单里**存在** `Microsoft.NETCore.App.Runtime.NativeAOT.win-x86` 包
> （我直接读了 [10.0.12 release notes](https://github.com/dotnet/core/blob/main/release-notes/10.0/10.0.12/10.0.12.md) 的包表）。
> 包存在 ≠ 官方支持 x86 AOT。⚠️未验证，而且你在 x64 上用不到。

---

## 3. C# → C++（反向 P/Invoke）

### 3.1 两种方式对比，以及引擎该选哪个

| 方式 | 机制 | 适合引擎吗 |
|---|---|---|
| `[DllImport]` / `[LibraryImport]` 静态声明 | C# 侧声明 `extern`，运行时按名字找 DLL 导出 | ❌ 不推荐。它要求**引擎可执行文件的符号被导出**（要做 dllexport/`/EXPORT`，还要保证 C# 找得到模块名），而且把"宿主 API"硬编码进了脚本层，无法在运行期换宿主 |
| **宿主把 C 函数指针表/结构体传进脚本** | C++ 建一个 blittable struct，字段是 `delegate* unmanaged[...]`；通过一个 `gs_init` 导出函数传进去 | ✅ **推荐**。这是官方 best-practice 的方向 |

官方明确推荐函数指针 + `UnmanagedCallersOnly`，而不是 `Delegate` / `Marshal.GetFunctionPointerForDelegate`：

> *"DO prefer using function pointers and `UnmanagedCallersOnlyAttribute` as opposed to `Delegate` types, when passing callbacks to unmanaged functions in C#."*
> —— [Native interoperability best practices](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices)

**推荐形态（引擎专为游戏脚本设计时的最佳模式）：**

```
C++ 引擎                                  C# 脚本 DLL
─────────                                ────────────
HostApi g_api { CreateEntity,            [UnmanagedCallersOnly(EntryPoint="gs_init")]
                GetComponent,             static int Init(HostApi* host, byte* root)
                GetAxis, Log, ... }             ↓ 存下来
        │                                        │
        └── LoadLibrary + GetProcAddress ────────┘
            gs_init(&g_api, "…/scripts")
```

对你有三个好处：
1. **一次导入一批函数**，不是每个函数一次 P/Invoke 查找；
2. 脚本层不依赖任何"引擎 DLL 名字"，引擎换成一个 exe、换路径、甚至换进程都不影响；
3. 以后要热重载时，同一张 `HostApi` 表可以原样交给新 ALC 里的新脚本 —— **宿主侧零改动**。

参考实现：官方 `Delegate* unmanaged` 与 `UnmanagedCallersOnly` 的回调示例
—— [UnmanagedCallersOnlyAttribute 示例](https://learn.microsoft.com/en-us/dotnet/api/system.runtime.interopservices.unmanagedcallersonlyattribute)、
[calling-conventions 示例](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/calling-conventions)

### 3.2 跨边界传 C++ 对象：opaque handle vs 裸指针

**结论：用不透明句柄（`int32` 索引或 `void*` 到引擎自有对象池），绝不要把 C++ 对象指针当成"托管对象"来管。**

理由与内存管理规则（全部有据）：

- C++ 对象**不是** GC 对象，C# 侧永远只应该拿 `void*` / `nint` / `int` 句柄，**不要**尝试把它包装成 managed class 让它进 GC。
- 反过来，**C# 对象**要给 C++ 长期持有时，必须自己把它钉住/根住，否则 GC 会回收或搬走它。
  官方给的两种 `GCHandle` 模式：
  - 钉住取地址：`GCHandle.Alloc(obj, GCHandleType.Pinned)` → `AddrOfPinnedObject()` → 用完 `handle.Free()`
  - 跨原生代码往返传引用（回调典型）：`GCHandle.Alloc(obj)` + `GCHandle.ToIntPtr(handle)` →
    回调里 `GCHandle.FromIntPtr(param).Target` → 最后 `handle.Free()`
  —— [Native interoperability best practices § Keeping managed objects alive](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#keeping-managed-objects-alive)
- **`GCHandle` 必须显式 `Free()`，否则泄漏**（同一文档原话：*"Don't forget that `GCHandle` needs to be explicitly freed to avoid memory leaks."*）
- 生命周期要交给 `SafeHandle`，**不要**用 finalizer。
  —— 同上 *"DO use `SafeHandle` handles to manage lifetime of objects"*
- 传 `Delegate` 给原生代码时，GC **不跟踪**"函数指针 → 委托"的关系，必须 `GC.KeepAlive` 或把委托存在 `static` 字段里：
  *"the garbage collector does not track the relationship between the returned pointer and the source delegate. If the delegate is eligible for collection before the native code finishes using the pointer, the application will crash."*
  —— 同上 § Prevent delegate collection with GC.KeepAlive
- ⚠️ 关键互动（对第 5 节热重载极重要）：**从外部施加的 strong `GCHandle`（Normal 或 Pinned）会阻止 collectible ALC 卸载。**
  *"None of the types ... are referenced by: ... Strong garbage collector (GC) handles (`GCHandleType.Normal` or `GCHandleType.Pinned`) from both inside and outside of the `AssemblyLoadContext`."*
  —— [Assembly unloadability](https://learn.microsoft.com/en-us/dotnet/standard/assembly/unloadability)

**给引擎的具体建议**：句柄用 `uint32` 索引 + generation 计数器 进 64-bit `EntityId`，
引擎侧维护 `slot → T*` 的数组。这比裸 `void*` 安全（能检测 use-after-free / 悬垂句柄），
而且**跨 ALC 重载天然稳定**（句柄是纯整数，重载后依然有效）。

---

## 4. 双向数据 marshalling 最佳实践

### 4.1 核心分层原则

**导出侧（`[UnmanagedCallersOnly]`）只能用 blittable 类型**，所以那里**没有"marshalling"这回事** ——
所有"高级类型"都必须手工降级成指针 + 长度。真正能用到 marshalling 的只有**调用侧（P/Invoke）**。

### 4.2 字符串（UTF-8）

- **导出侧（C# 收 C++ 的字符串）**：签名用 `byte*` + `int32 len`，然后
  `Encoding.UTF8.GetString(new ReadOnlySpan<byte>(ptr, len))`。
  **不要**指望 `string` 参数 —— `string` 不是 blittable，`UnmanagedCallersOnly` 不接受。
- **调用侧（C# 调 C++）**：用 `[LibraryImport]` + `StringMarshalling.Utf8`：

  ```csharp
  [LibraryImport("engine", EntryPoint = "eng_load_file",
                 StringMarshalling = StringMarshalling.Utf8)]
  internal static partial int LoadFile(string path);
  ```
  *"UTF-8 is now available as a first-class option"*（`CharSet` 被 `StringMarshalling` 取代，ANSI 被移除）
  —— [P/Invoke source generation](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation)
- `LibraryImport` 支持 `StringMarshalling.Utf16` / `Utf8`。`string` 按值（非 `ref`/`out`）传且是 UTF-16 时，
  **运行时是钉住而不是拷贝**（性能好）。
  —— [best practices § String parameters](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#string-parameters)
- ❌ **绝不要用 `[Out] string`**：*"String parameters passed by value with the `[Out]` attribute can destabilize the runtime if the string is an interned string."*（同上）
- 取返回值字符串优先用 `Marshal.PtrToStringUTF8`。
- 避免 `StringBuilder`（每次调用 4 次分配）；改用 `ArrayPool<byte>` 的 `byte[]`/`char[]` + `[Out]`。（同上）
  `LibraryImport` **完全不支持** `StringBuilder`。

### 4.3 数组 / Span

- **导出侧**：`T*` + `int32 count`。C# 里用 `new Span<T>(ptr, count)` / `MemoryMarshal.Cast` 零拷贝包装。
- **调用侧**：`LibraryImport` 只支持**一维数组**；`SafeArray`、`SizeConst`/`SizeParamIndex`/`ArraySubType` 用在非数组类型上都不支持；
  ≤256 字节的按值/只读引用数组会**分配在栈上**而不是 `AllocCoTaskMem`。
  —— [LibraryImportGenerator Compatibility.md](https://github.com/dotnet/runtime/blob/main/docs/design/libraries/LibraryImportGenerator/Compatibility.md)
- 缓冲池优先用 `ArrayPool<T>` —— [best practices](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices)

### 4.4 结构体（按值 / 按引用）

- 结构体应尽量 **blittable**：`[StructLayout(LayoutKind.Sequential)]`（默认）或 `Explicit`，
  字段全是 blittable 值类型。
- blittable struct 按 `in`/`ref`/`out` 或按值传时，marshaller 是**钉住**而不是拷贝；按值传的 managed struct 本身就相当于已钉住
  *"Managed structs are created on the stack and are not removed until the method returns. By definition then, they are 'pinned'"*
  —— [best practices § Structs](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#structs)
- blittable struct 用 `sizeof(T)` 而**不是** `Marshal.SizeOf<T>()`。（同上）
- **`bool` 是雷**：它**不是 blittable**（默认 marshal 成 4 字节 Windows `BOOL`）。跨边界用 `byte` 或 `int`。
  —— [best practices § Boolean parameters and fields](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#boolean-parameters-and-fields)
- ❌ 不要用 `System.Delegate` / `MulticastDelegate` 字段表示函数指针字段，要用具名委托或 `delegate* unmanaged<...>`。（同上）
- C/C++ `long` ≠ C# `long`（Windows 上都是 32 位 `long`）。用 `CLong` / `CULong`。（同上）

### 4.5 `LibraryImport` vs `DllImport`，以及 NativeAOT 专属约束

| | `DllImport` | `LibraryImport`（.NET 7+，默认开启的源生成器） |
|---|---|---|
| marshalling 代码 | **运行期生成 IL stub，然后 JIT** | **编译期生成 C# 源码**，可内联、可断点调试 |
| 是否可用于 NativeAOT | ⚠️ 官方口径是"不适用" | ✅ 可以 |
| `CharSet` | 有 | 无 → 用 `StringMarshalling`（新增 UTF-8） |
| `CallingConvention` | 有 | 无 → 用 `[UnmanagedCallConv]` |
| `ExactSpelling` / `PreserveSig` | 有 | 无 |
| `StringBuilder` / `ICustomMarshaler` / `CriticalHandle` / `HandleRef` / `SafeArray` | 支持 | **不支持** |
| 要求 | — | 项目需 `AllowUnsafeBlocks`，方法必须是 `static partial` |

依据：[P/Invoke source generation](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation)、
[Compatibility.md](https://github.com/dotnet/runtime/blob/main/docs/design/libraries/LibraryImportGenerator/Compatibility.md)、
[best practices](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices)

**NativeAOT 特有的 marshalling 约束：**

1. IL stub 在 AOT 下**不存在**，所以带非 blittable marshalling 的 `DllImport` 不可用。
   原文：*"Since this IL stub is generated at runtime, it isn't available for ahead-of-time (AOT) compiler or IL trimming scenarios. ... Using `DllImport` isn't an option for platforms that require full Native AOT scenarios."*
   —— [P/Invoke source generation](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation)
2. NativeAOT 下 P/Invoke **默认是运行期懒绑定**；想更好性能/静态链接，用 `<DirectPInvoke>` / `<DirectPInvokeList>` / `<NativeLibrary Include="....lib" />` / `<LinkerArg>`。
   ⚠️ 但直接 P/Invoke 由**操作系统动态加载器**解析，**不再遵守 `DefaultDllImportSearchPathsAttribute`**。
   —— [Native code interop with Native AOT](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/interop)
3. `[assembly: DisableRuntimeMarshalling]` 会关掉大部分内建 marshalling。禁用后类型映射变得极简：
   `bool` → 1 字节 `bool`（**不归一化**）、`char` → `char16_t`（`CharSet` 无效）、
   非托管 user-defined struct 视为 blittable、**其他类型一律不支持**；
   并且 `SetLastError=true`、`LCIDConversion`、`BestFitMapping`、varargs、
   **以及 `in`/`ref`/`out` 参数** 会被列为"无效果或抛异常"。
   —— [Disabled runtime marshalling](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/disabled-marshalling)
   **实践建议**：在导出侧（`UnmanagedCallersOnly`）你本来就必须 blittable，所以加不加它区别不大；
   但在**调用侧**加了它之后，`in`/`ref`/`out` 的行为就变得微妙 —— **稳妥做法是显式用指针 `T*`**，不要用 `ref`/`out`。
4. NativeAOT 下 `Activator.CreateInstance` 有 trimming 问题，因此 `LibraryImport` 的 `ref`/`out`/返回值 marshaller
   **要求类型必须有公开无参构造**（.NET 8 起从"建议"变成"要求"）。
   —— [Compatibility.md, Version 3](https://github.com/dotnet/runtime/blob/main/docs/design/libraries/LibraryImportGenerator/Compatibility.md)

---

## 5. 热重载 / 脚本重载

### 5.1 .NET 的 "Hot Reload" 到底覆盖什么？对 NativeAOT 有效吗？

**覆盖范围（官方）：**
- 由 **调试器 + Roslyn 编译器** 驱动，走 **Edit and Continue / metadata update** 机制（`MetadataUpdater.ApplyUpdate` 那一套）。
  *"The .NET Hot Reload experience is powered by the debugger and C# compiler (Roslyn)."*
  —— [Visual Studio Hot Reload](https://learn.microsoft.com/en-us/visualstudio/debugger/hot-reload)
- 支持的类型：Console / WPF / WinForms / ASP.NET / Blazor / MAUI 等，需要 **.NET 6+**（无调试器场景）或 **.NET 8+**（Linux/macOS 容器）。
- CLI 侧是 `dotnet watch`：改文件 → 判断能否 hot reload → **不能就报 "rude edit" 并询问是否重启**；
  可以 `--no-hot-reload` 关掉，或 `DOTNET_WATCH_RESTART_ON_RUDE_EDIT=1` 直接重启。
  —— [dotnet watch](https://learn.microsoft.com/en-us/dotnet/core/tools/dotnet-watch)
- **明确不支持的配置**：`PublishTrimmed=true`、`PublishReadyToRun=true`、Release/自定义配置、F#、
  混合模式调试（mixed-mode debugging）、Attach to Process。
  —— [Visual Studio Hot Reload § Unsupported](https://learn.microsoft.com/en-us/visualstudio/debugger/hot-reload)

**对 NativeAOT：明确不支持。** 三处独立证据：
1. *"hot reload is not supported for AOTd assemblies."* —— [dotnet/runtime#54617](https://github.com/dotnet/runtime/issues/54617)
2. runtime 仓库有个 PR 把 `IsMetadataUpdateSupported` 在 NativeAOT 场景下的判定改成用"aggressive trimming"来替代，
   就是为了给 Apple 移动端的 AOT 测试挂 `ActiveIssue` —— 说明**AOT ⇒ metadata update 不可用**是被当作既定事实处理的。
   —— [dotnet/runtime#127055](https://github.com/dotnet/runtime/pull/127055)（2026-04-20 合并）
3. NativeAOT 不支持 `Reflection.Emit`、不支持动态加载 —— metadata delta 的落地机制本身不存在。
   —— [Native AOT overview](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/)

> 补充观察：.NET 10.0.12 的包清单里出现了 `Microsoft.DotNet.HotReload.Agent.Host`、
> `Microsoft.DotNet.HotReload.Web.Middleware`、`Microsoft.DotNet.HotReload.Watch.Aspire` 等包
> —— [10.0.12 release notes](https://github.com/dotnet/core/blob/main/release-notes/10.0/10.0.12/10.0.12.md)。
> 说明热重载 agent 已经是正式交付组件。但它依然只服务于**非 AOT**的托管执行。

### 5.2 三条现实路线的可行性 / 延迟 / 坑

#### (a) `AssemblyLoadContext` collectible + 整程序集卸载/重载（CoreCLR 宿主路径）—— ✅ 可行，是唯一真方案

**怎么做**：
```csharp
class ScriptAlc : AssemblyLoadContext {
    public ScriptAlc() : base(isCollectible: true) { }
    protected override Assembly? Load(AssemblyName name) => null; // 依赖进 Default
}
```
—— [unloadability 文档](https://learn.microsoft.com/en-us/dotnet/standard/assembly/unloadability)

**延迟**：官方样例用 "调用 `Unload()` 后 `GC.Collect()` + `WaitForPendingFinalizers()` 循环" 等卸载完成，
*"In most cases, just one pass through the loop is required"*。参照 Godot 的实现，一次脚本重载的实际量级是
**数百毫秒到 1–2 秒**（编译 + 序列化 + 卸载 + 重载 + 反序列化），Godot 官方承认
**"State is currently not saved and restored when hot-reloading, with the exception of exported variables."**
—— [Godot C# basics](https://raw.githubusercontent.com/godotengine/godot-docs/master/tutorials/scripting/c_sharp/c_sharp_basics.rst)
Flax（自研 Mono fork 时代的实测日志）给出 **154 ms** 的完整脚本重载，并承认
*"Unsafe – user need to free event handlers and use static code with care / May result in crashes"*
—— [Flax Facts #16](https://flaxengine.com/blog/flax-facts-16-scripts-hot-reload/)

**"收集式 ALC 真的能被完全卸载吗？"—— 这是你问题的核心，答案是"能，但必须满足一长串条件，且很容易失败"：**

卸载是**协作式**的（不像 AppDomain 那样强制），必须同时满足
—— [unloadability 文档](https://learn.microsoft.com/en-us/dotnet/standard/assembly/unloadability)：

1. **没有任何线程的调用栈上有该 ALC 里的方法**（连 JIT 引入的临时栈槽引用都算）；
2. 没有任何来自外部的强引用指向该 ALC 里的 assembly / type / instance（弱引用除外）；
3. **没有任何来自内部或外部的 strong GC handle（`Normal` 或 `Pinned`）** 指向它们；
4. 没有 `RegisteredWaitHandle` 之类的挂起回调；
5. 自定义 `AssemblyLoadContext` 子类的**字段**不能引用 ALC 内的东西 ——
   卸载过程中 runtime 会持一个 strong handle 到 ALC 本身，所以**必须主动把这些字段清空**；
6. 在 ALC 内创建的非 collectible ALC 实例也不行；
7. `Unload()` 只是"发起"卸载，真正完成要看 GC 什么时候收掉。

**已知真实坑（.NET 10，2026-08 报告）：**
反复 load/call/unload/collect 同一个小组件时，**泛型虚方法派发会挂死**（一个核 100% 占用，不是等锁）。
.NET 8.0.29 上 100 次循环通过；.NET 10.0.10 上第 3–4 次就挂。
根因定位到进程级 `GenericCache` 的 flush sentinel 没被正确识别（关联 #89331，由 PR #106843 暴露）。
Issue 于 **2026-09-03 关闭**（`state_reason=completed`，assignee VSadov）。
—— [dotnet/runtime#132562](https://github.com/dotnet/runtime/issues/132562)
⚠️ **未验证**：我查了 [10.0.12 release notes](https://github.com/dotnet/core/blob/main/release-notes/10.0/10.0.12/10.0.12.md)，
里面只列了 6 个 CVE，**没有提这个修复**。所以"10.0.12 是否已修"我无法确认，请自己复现验证。

**"原生代码持有指向该 ALC 的函数指针时，ALC 还能卸载吗？"—— 分两种情况，这是最关键的区分：**

| 拿到函数指针的方式 | 会不会阻止卸载 | 后果 |
|---|---|---|
| `Marshal.GetFunctionPointerForDelegate(delegate)` | **会**。运行时为该委托生成了一个 stub，并持有一个指向它的 **strong（loader allocator 级）句柄** | ALC **永远卸不掉**。而且如果原生侧长期持有该指针，之后调用它就是**悬垂函数指针** |
| `delegate* unmanaged<...>`（`&StaticMethod`，C# 9 函数指针语法，方法带 `[UnmanagedCallersOnly]`） | 只是一个**裸地址**，不产生 GC 引用 | 不阻止卸载 —— 但这更危险：**卸载完成后那个地址就是野指针，再调用必崩** |

依据：`GetFunctionPointerForDelegate` 的委托必须被 root（*"If native code stores the function pointer beyond the call ... the delegate must be rooted for its entire lifetime"*
—— [best practices](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices#prevent-delegate-collection-with-gckeepalive)），
以及卸载条件 3（strong GC handle 阻止卸载）。

**所以工程上的铁律是：**
> **所有跨边界的函数指针都必须指向「非 collectible 的宿主/桥接程序集」，绝不指向可回收 ALC 里的方法。**
> 可回收 ALC 只放"纯逻辑"，通过 `MethodInfo`/委托在**受控的调用点**进入，退出时确保栈已清空、句柄已释放。

**关于「有 `UnmanagedCallersOnly` 导出的程序集能否卸载」的文档化 gotcha：**
我**没有**找到专门讨论这个组合的官方文档或 issue（⚠️未验证）。能确定的是两条相邻事实：
① `UnmanagedCallersOnly` 方法**不能被托管代码调用**，所以它天然是"只能从原生进来的入口"；
② 卸载条件里"没有线程的调用栈上有该 ALC 的方法"意味着：**只要还有一次调用在栈上，卸载就不会完成**
（而不是抛异常）。这会导致"看起来没报错、但内存一直不释放"的隐性泄漏 —— 用 `WeakReference` + `IsAlive` 轮询来验证卸载是否真的完成
（官方推荐循环：`for (int i = 0; ref.IsAlive && i < 10; i++) { GC.Collect(); GC.WaitForPendingFinalizers(); }`）。

**其他会 100% 断掉卸载的东西**（同上文档）：静态字段、闭包、事件订阅、
你自己 ALC 子类的字段、`Thread` 里跑到该 ALC 的代码、`RegisteredWaitHandle`。
调试手段：WinDbg + SOS，`!dumpheap -type LoaderAllocator` → `!gcroot <addr>` → `~*e !clrstack`。

#### (b) 重新跑 NativeAOT 编译 —— ❌ 对你的目标不可行

- 首先**构建延迟**本身就是灾难：即使 20–60 秒（⚠️未验证估算），对调一个 `player.speed = 5.0f` 也完全不可接受。
- 其次**旧 DLL 卸载不了**（本文档开头引用的官方声明：`FreeLibrary` 不支持）。
  你只能不断 `LoadLibrary` 新的 AOT DLL。**同一个进程里并存多个 NativeAOT 共享库是否被支持，我找不到任何官方说明（⚠️未验证）**，
  考虑到每个库都内嵌一份 runtime + GC，我倾向于认为这是未定义行为。
- **唯一"能工作"的变体**：把脚本层放到**独立子进程**里，改脚本 → 重新 AOT → 杀掉子进程 → 起新的。
  引擎主进程和世界状态留在 C++ 侧，通过共享内存/IPC 传指令。
  这是可行的，但**延迟以秒计**、且要写一套 IPC 协议 —— 这是我自己综合出来的方案，**没有任何文档背书**（⚠️未验证）。

#### (c) "Edit and Continue 式 delta 补丁" —— ❌ 不可行

- 走 `MetadataUpdater.ApplyUpdate` / `dotnet watch` / VS Hot Reload 那一套；
  **AOT 下不支持**（见 5.1 的三条证据）；`PublishTrimmed` / `PublishReadyToRun` 也不支持；
  还要求 **Debug 配置**、需要调试器或有 named-pipe agent（`DOTNET_HOTRELOAD_NAMEDPIPE_NAME`）。
  —— [dotnet watch](https://learn.microsoft.com/en-us/dotnet/core/tools/dotnet-watch)、
  [VS Hot Reload](https://learn.microsoft.com/en-us/visualstudio/debugger/hot-reload)
- 而且 EnC 只能改方法体一类的"温和编辑"，加字段/改类型/加方法都是 rude edit。
- 对游戏引擎来说，"只能改方法体"价值有限，还有一堆"必须挂调试器"的运维负担。
  我的判断：**不值得为它做架构设计**。

---

## 6. CoreCLR 宿主路径（hostfxr / nethost）作为 NativeAOT 的替代

### 6.1 当前推荐 API

官方教程就是答案 —— [Write a custom .NET host](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)
（最后更新 2025-11-10）。设计文档见
[native-hosting.md](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md)，
头文件：`nethost.h`、`hostfxr.h`、`coreclr_delegates.h`。

**流程（4 步）：**

1. **定位 hostfxr**：`nethost` 库的 `get_hostfxr_path(buffer, &size, params)`（`__stdcall`）。
   `params.assembly_path` 或 `params.dotnet_root` 可选。
2. **初始化 host context**：`hostfxr_initialize_for_runtime_config(runtimeconfig_path, params, &ctx)`。
   它**只读 `.runtimeconfig.json`、解析框架、准备初始化，不加载 CoreCLR**。
3. **取运行时委托**：`hostfxr_get_runtime_delegate(ctx, type, &fn)`。
   可用的 `hostfxr_delegate_type`：
   - `hdt_load_assembly_and_get_function_pointer` —— .NET Core 3.0+，
     **把程序集加载到独立的 ALC**，返回指向某个 static 方法的原生函数指针
   - `hdt_get_function_pointer` —— .NET 5+，在 **Default ALC** 里找方法
   - `hdt_load_assembly` / `hdt_load_assembly_bytes` —— .NET 8+，**加载到 Default ALC**，不执行
4. **收尾**：`hostfxr_close(ctx)`。

**关键签名：**
```c
int load_assembly_and_get_function_pointer_fn(
    const char_t *assembly_path,
    const char_t *type_name,
    const char_t *method_name,
    const char_t *delegate_type_name,
    void         *reserved,      // 必须 NULL
    /*out*/ void **delegate);
```
- `delegate_type_name` 传 `NULL` ⇒ 使用默认签名
  `public delegate int ComponentEntryPoint(IntPtr args, int sizeBytes);`
  即原生 `int component_entry_point_fn(void*, int32_t)`。
- `delegate_type_name` 传 `UNMANAGEDCALLERSONLY_METHOD`（= `(const char_t*)-1`）⇒
  表示托管方法带 `[UnmanagedCallersOnly]`。**这是 NativeAOT 与 CoreCLR 两条路可以共用同一份脚本签名约定的地方。**
- `AssemblyDependencyResolver` 处理 `.deps.json` 依赖解析。

**⚠️ 三个必读陷阱（全部来自 native-hosting.md）：**
1. **进程内只能有一个 runtime。** `hostfxr_initialize_for_runtime_config` 二次调用会尝试
   "attach"到已有 runtime，不兼容就失败。官方直接说：*"Only one runtime can be loaded inside a single process."*
   （[教程 § Limitations](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)）
2. **`hdt_load_assembly_and_get_function_pointer` 返回的函数指针是进程生命周期的，
   而且"目前没有办法卸载这个托管组件或释放这个原生函数指针"。**
   原文：*"The returned native function pointer to managed method has the lifetime of the process and can be used to call the method many times over. Currently there's no way to unload the managed component or otherwise free the native function pointer."*
   ⇒ **不要用它做热重载**。做热重载请用 `hdt_load_assembly`（加载到 Default ALC，.NET 8+）
   + **你自己的 collectible ALC**（在托管侧写一个 bootstrap，由 `hdt_get_function_pointer` 拿到入口）。
   —— [native-hosting.md § Loading and calling managed components](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md)
3. **hostfxr / nethost 只支持 framework-dependent 部署。**
   *"The `nethost` and `hostfxr` hosting APIs only support framework-dependent deployments. Self-contained deployments should be treated as stand-alone executables."*
   —— [教程 § Hosting APIs](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)
   ⇒ 你必须让用户机器（或你的安装包）装上 **.NET 10 Desktop/Runtime**。

另有一条部署/调试的坑：**trimming 与 native hosting 不兼容**（"Native hosting support on managed side is disabled by default on trimmed apps"），
因为 trimmer 无法分析"从原生被调用的方法"。—— [native-hosting.md 末尾](https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md)

### 6.2 与 NativeAOT 的取舍对照

| 维度 | NativeAOT native library | CoreCLR + hostfxr |
|---|---|---|
| **启动时间** | 最快（社区量级 ≈14–17 ms，⚠️非原生库实测） | JIT 冷启动，社区量级 ≈70–80 ms（同源） |
| **执行模型** | 全 AOT，无 JIT | JIT + 分层编译 + PGO（长跑吞吐**可能更好**，也可能更差，需实测） |
| **GC 控制** | 内嵌精简 runtime | 完整 runtime，可调 |
| **实时性/帧时间** | 无 JIT 抖动，行为确定 | **有 JIT 编译抖动、GC 暂停、后台 GC 线程争 CPU** —— 对 2D 小游戏是真实风险 |
| **热重载** | ❌ 完全不可能 | ✅ 可行（collectible ALC），但有 5.2 那一堆坑 |
| **部署复杂度** | 极低：一个 DLL，零依赖 | 高：需要 .NET runtime；且**只能 framework-dependent** |
| **可用的库生态** | 必须 AOT-clean（无 `Assembly.LoadFile`、无 `Reflection.Emit`、`Linq.Expressions` 慢） | 全生态 |
| **`AssemblyLoadContext` collectible 卸载** | 不适用 | ✅ **可行，这就是本路径存在的理由** |

**GC 相关的可调项**（在 `runtimeconfig.json` 的 `configProperties` 或环境变量里配；
注意：**这些只在 GC 初始化时读一次，进程起来后改环境变量无效**）
—— [GC config settings](https://learn.microsoft.com/en-us/dotnet/core/runtime-config/garbage-collector)：

```json
{
  "runtimeOptions": {
    "configProperties": {
      "System.GC.Server": false,               // 默认就是 false；游戏进程别开 server GC
      "System.GC.Concurrent": true,            // 后台 GC，减少长暂停
      "System.GC.HeapHardLimit": 209715200,    // 200 MiB 硬上限，保护引擎内存预算
      "System.GC.ConserveMemory": 5            // 0-9，省内存换更多/更长的 GC
    }
  }
}
```
- `DOTNET_GCHeapHardLimit` 用**十六进制**（`0xC800000`），JSON 里用**十进制**（`209715200`）。
- **DATAS**（Dynamic Adaptation to Application Sizes）**自 .NET 9 起默认开启**，`DOTNET_GCDynamicAdaptationMode=0` 可关。
- .NET 10 新增 `System.GC.RegionSize` / `System.GC.RegionRange`（64 位 Windows/Linux 上 GC 堆是 region 而非 segment）；
  小堆进程可以考虑把 region size 调到 1 MB 以省 native 内存。
- `System.GC.Path`（.NET 9+）/ `System.GC.Name`（.NET 7+）可换 standalone GC（`clrgc.dll`）—— 对你大概是过度设计。

### 6.3 `AssemblyLoadContext` collectible 卸载在这条路径上可行吗？

**可行，而且这是它的核心价值。** 但要注意谁在加载什么：

- `hdt_load_assembly_and_get_function_pointer` 用的是**它自己创建的隔离 ALC**，
  你**拿不到那个 ALC 的引用**，也就**无法 `Unload()` 它**，而且返回的指针终身有效（见 6.1 陷阱 2）。
  ⇒ **不能用于脚本热重载。**
- `hdt_load_assembly`（.NET 8+）加载到 **Default ALC**（不可回收）。
  ⇒ 也不能用于热重载。
- **正确做法**：用 `hdt_get_function_pointer` 拿一个**稳定驻留**的 bootstrap 方法指针
  （例如 `Engine.Bootstrap.ReloadScripts(string path, HostApi* api)`），
  让这个 bootstrap **在托管侧**创建 collectible `AssemblyLoadContext`、`LoadFromAssemblyPath`、
  取 `MethodInfo`、调用、卸载。**整个热重载逻辑放在 C# 侧**，C++ 侧只调一个 `reload()`。
  这样既绕开了"引擎不能卸载 ALC"的限制，又把 ALC 生命周期的复杂度收敛到托管代码里。
  （这是我对文档的综合，⚠️非官方示例。）

- **卸载的硬约束重复一次**：`UnmanagedCallersOnly` 导出/`GetFunctionPointerForDelegate` stub 只要
  被原生侧长期持有，就会**钉死 ALC**。所以"宿主函数指针表"必须指向**不可回收的桥接程序集**，
  可回收 ALC 里只放逻辑。

---

## 7. 现有开源先例

### 7.1 大型引擎（作为架构参考，不是"可抄的代码"）

| 项目 | C# 集成方式 | 热重载做法 | 链接 |
|---|---|---|---|
| **Godot 4.x (.NET)** | C++ 引擎 + CoreCLR，`modules/mono` | 重编译 → **程序集重载**；官方承认 *"State is currently not saved and restored when hot-reloading, with the exception of exported variables."*；Godot 4.5 需 .NET 8+，Android 导出需 .NET 9+（**不能导 Web**） | [C# basics（docs 源）](https://raw.githubusercontent.com/godotengine/godot-docs/master/tutorials/scripting/c_sharp/c_sharp_basics.rst)、[csharp_script.cpp](https://raw.githubusercontent.com/godotengine/godot/4.7.1-stable/modules/mono/csharp_script.cpp) |
| **Unity** | 自有 Mono/IL2CPP 后端 | **domain reload**：重编译脚本 → **序列化托管对象** → 卸载/销毁程序集 → 重载 → 反序列化恢复状态；有 `[OnCodeUnloading]`/`[OnCodeInitializing]` 等生命周期回调 | [Code reload and the code lifecycle](https://docs.unity3d.com/6000.6/Documentation/Manual/programming-code-lifecycle.html)、[禁用 Domain/Scene Reload](https://docs.unity3d.com/6/Documentation/Manual/configurable-enter-play-mode-details.html) |
| **Flax Engine** | C++ 引擎 + C# 脚本（现用 .NET 8、C# 12） | 最有参考价值的写实记录：**只卸载用户程序集、不卸载整个 domain**（当年靠**自研 Mono fork**，因为标准 .NET 做不到）；实测 **154 ms**；自评缺点：*"Unsafe / May result in crashes / Requires custom Mono fork"* | [Flax Facts #16 – Scripts Hot-Reload](https://flaxengine.com/blog/flax-facts-16-scripts-hot-reload/)、[Flax C# Scripting 文档](https://docs.flaxengine.com/manual/scripting/csharp/index.html) |
| **Stride** | 纯 C# 引擎（.NET） | 有 `ScriptComponent.LiveScriptingMask`、`ScriptSystem` 等概念 | [Stride API ScriptComponent](https://doc.stride3d.net/latest/en/api/Stride.Engine.ScriptComponent.html)、[Asset hot reload 讨论](https://github.com/stride3d/stride/discussions/2388) |

**从 Flax 的经验里能直接学到的一件事**：他们为了"只卸载用户程序集"而**不得不 fork Mono**。
标准 .NET 上你能做到的最接近的东西就是 collectible ALC —— 而 ALC 的语义比"卸载单个 dll"更受限（见 5.2）。

### 7.2 小引擎 / 一手技术写实

| 项目 / 资料 | 内容 | 链接 |
|---|---|---|
| **keen-eye (orion-ecs)** | 一个用 **C# + Silk.NET + OpenGL** 做的游戏引擎，仓库内有专门的部署调研：**NativeAOT 全兼容（Silk.NET ≥ v2.18.0）**，给出 Windows AOT 构建 ≈2–3 min、体积 ≈10–15 MB、启动 ≈15–17 ms；并明确列出 AOT 代价：*"Plugin systems - Not supported; use compile-time composition instead"*、无 `Type.GetType()`、序列化必须用 source generator | [cross-platform-deployment.md](https://github.com/orion-ecs/keen-eye/blob/a83a9264f30b0f079e413a6bc9a5d6ab89671213/docs/research/cross-platform-deployment.md) |
| **State of Native AOT in .NET 10**（2025-11-10） | 唯一我找到的**逐年 hello-world 体积对比**（.NET 7 3.65 MB → .NET 10 1.05 MB）；列出生态里已支持 AOT 的库；也诚实指出 JIT 长期吞吐可能更高、`Linq.Expressions` 在 AOT 下永远是解释模式 | [code.soundaranbu.com](https://code.soundaranbu.com/state-of-nativeaot-net10) |
| **Native AOT in .NET: Faster Startup, Lower Cloud Costs**（2026-08-14） | 2026 年视角的取舍分析；强调"数字只能当方向不能当保证"，并给出 AOT 在生态上的实际缺口 | [vensas.de](https://vensas.de/en/blog/dotnet-native-aot-startup-cost) |
| **官方 NativeAOT 原生库样例** | 最直接的 C/C++ 加载 + `GetProcAddress` 参考；`LibraryFunctions.cs` 展示了在"不能抛异常、只能 blittable"约束下怎么写 | [dotnet/samples core/nativeaot/NativeLibrary](https://github.com/dotnet/samples/tree/main/core/nativeaot/NativeLibrary) |
| **LÖVE (love2d)** | 反例参照：一个成熟的 **C++ 2D 引擎，脚本层用 Lua** —— 说明"小 2D 引擎 + 嵌入式脚本"这个组合里，Lua 是经过大规模验证的默认选择 | [love2d.org](https://love2d.org/) |

**我没有找到**：任何一篇"**小型 C++ 引擎通过 NativeAOT 原生库嵌入 C# 脚本**"的完整技术写实（⚠️未验证是否存在）。
网上的 NativeAOT↔C++ 文章大多是"把 C# 算法库包成 DLL 给 C++ 调"（例如
[CSDN 实战指南](https://blog.csdn.net/sprite/article/details/149733366)，它自己也提到 .NET 8 时代
**x86 目标不受支持**、报 `NETSDK1201`，并强调 x64）。这与你的场景接近但不等于"游戏脚本层"。

---

## 8. 具体建议

### 8.1 先把矛盾摊开

你的五个约束里，**(i) 热重载很重要** 与 **NativeAOT** 直接冲突。
所以问题不是"选哪个技术"，而是"**接受哪个代价**"：

- 接受"改脚本要重启引擎" ⇒ NativeAOT
- 接受".NET 运行时依赖 + JIT/GC 在进程内" ⇒ hostfxr + collectible ALC

对一个 **小 2D 引擎 + 单人开发** 来说，**迭代速度的权重远高于启动时间和部署体积**。
一个 2D 游戏引擎的 exe 多 3 MB、启动多 60 ms，用户完全感知不到；
但"改一行要等 40 秒重建 + 重启引擎、丢掉所有运行状态"会**直接杀死你的开发效率**。
所以我下面的排序是按"目标 (i) 热重载优先"给的。

### 8.2 排序

#### 🥇 第 1 名：**CoreCLR 宿主（hostfxr/nethost）+ collectible ALC**
- **为什么**：唯一能真正满足"不重启引擎换脚本"的路径（5.2a + 6.3）。
- **主要风险**：
  1. **部署**：只能 framework-dependent（6.1 陷阱 3），必须让 .NET 10 装到目标机 —— 这对"发游戏"是不小的负担。
  2. **ALC 卸载极易泄漏**：一条静态字段、一个事件订阅、一个 `GCHandle`、一个 `GetFunctionPointerForDelegate`
     stub，就能让 ALC 永远卸不掉。而且**失败是静默的**（不报错，只是内存不释放）。
  3. **.NET 10 上有过真实的反复重载回归**（#132562，泛型虚派发挂死一个核）。
     这说明"反复 load/unload"在 .NET 上是**被测试覆盖得不算充分**的路径。⚠️是否已修未验证。
  4. **帧时间**：JIT 抖动 + GC 暂停 + 后台 GC 线程抢 CPU。2D 小游戏也可能被 GC 尖峰打断。
     缓解：`System.GC.Concurrent=true`、`HeapHardLimit`、减少每帧分配（用 struct/池）。
- **必须有这些护栏才敢上**：① 所有跨边界函数指针只指向非 collectible 桥接程序集；
  ② 用 `WeakReference.IsAlive` 断言每次重载后 ALC 真的被回收，**失败就告警**；
  ③ 每次重载后重置所有静态状态（Godot 的教训：状态不会自动恢复）；
  ④ CI 里跑"连续重载 200 次"的压测（正是 #132562 那种 bug 的暴露方式）。

#### 🥈 第 2 名：**先做 Lua（或同类嵌入式小 VM），C# 以后再说**
- **为什么我把它排这么高**：从上面已验证的事实反推，Lua 把三个最难的问题**在架构上一次性消掉**：
  - **热重载**：脚本是数据，状态本来就该住在 C++ 侧；重载 = 重新 `luaL_loadfile` + 重建 VM/环境，
    **不存在 ALC 那种"协作式卸载 + 静默失败"**。Flax 为了"卸载单个程序集"要 fork Mono，
    Godot 至今不能恢复热重载状态 —— 这些成本在 Lua 里根本不存在。
  - **部署**：Lua 是 ~200 KB 的 C 源码，直接 static link 进你的 exe，**零运行时依赖、零安装器**。
  - **确定性**：无 JIT、无 GC 线程争抢（Lua 的 GC 是你自己调 `lua_gc` 的，可控）。
  - **先例充分**：LÖVE、Defold 等都是"C++ 2D 引擎 + Lua"的成熟组合
    （[love2d.org](https://love2d.org/)）。
- **代价（要诚实说）**：失去 C# 的静态类型、IDE 体验、整个 NuGet 生态、LINQ/async 等；
  你自己要写绑定层（用 LuaBind/Sol2 之类的 header-only 库可以显著省事）；性能上 Lua 比 C# 慢（但脚本逻辑通常不是瓶颈）。
- **什么情况下"先做 Lua"是明显更优的工程决策**：如果这个引擎的目的是**做出并发布一个 2D 游戏**，
  那么是的 —— **先把 Lua 做出来**。理由是纯工程风险：你要在"单人 + 小引擎"的预算里，
  同时承担「ALC 生命周期正确性」+「.NET 部署」+「GC 帧时间」三块复杂度，而这三块**都不是你的游戏本身**。
  你可以先用 Lua 把游戏做出来，等真有需求再考虑 C#。
- **什么情况下不选它**：如果"引擎支持 C# 作为一等脚本语言"**本身就是项目目标**（做工具/做平台/学习/简历），
  那就该直接上第 1 名，Lua 只是拖延。

#### 🥉 第 3 名：**NativeAOT native library**
- **它的定位是"发布形态"，不是"迭代形态"。**
- 优点：真·零依赖单文件、启动最快、无 JIT 抖动、反编译难度高。
- **主要风险**：
  1. **没有热重载，而且是永久性的**（官方明说不支持卸载）。开发循环 = 改 → publish → 重启引擎。
  2. 一旦你要用反射/动态类型/`Linq.Expressions`/任何 AOT-unclean 的库，就会在 publish 时才炸（`IL2xxx/IL3xxx` 警告）。
  3. 必须保证脚本层 API 表面**只有 blittable 类型**，且所有导出都在被 publish 的那个程序集里。
  4. 跨 x64 的平台工具链要求（MSVC C++ 工作负载）。
- **理性的折中（我推荐的落地形态）**：
  **同一个脚本工程，两种宿主模式，共享同一份"纯 blittable 的 HostApi + 导出签名约定"**：
  - **开发态**：hostfxr + collectible ALC，热重载可用；
  - **发布态**：同一份 C# 源码 `dotnet publish -p:PublishAot=true -p:NativeLib=Shared`，`LoadLibrary` 加载。
  这条路能走通的前提是：**脚本层从一开始就不碰反射、不用 `Type.GetType`、不动态生成代码、
  数据全部走 blittable 结构体/`UnmanagedCallersOnly` 导出**。
  代价是你要维护两套加载代码（C++ 侧约 150–250 行）并让 AOT 分析始终干净。
  ⚠️ 这套"双模"设计是我基于文档的综合建议，**没有官方样例**，属于需要你自己验证的架构选择。

#### ❌ 不推荐：Edit-and-Continue / metadata delta 补丁
AOT 下不支持；托管下也要求 Debug 配置 + 调试器/named-pipe agent，且只能做"温和编辑"
（加字段/加类型就会变成 rude edit 要求重启）。对游戏脚本迭代的收益远低于其复杂度。

### 8.3 如果选第 1 名，最小落地清单

1. C++ 侧：`nethost` + `hostfxr` 初始化（见 9.4 代码）。
2. C# 侧写一个**常驻 bootstrap 程序集**（不可回收，走 Default ALC），暴露：
   `int LoadScripts(string path, HostApi* api)` / `int ReloadScripts()` / `void Tick(float dt)` / `void Shutdown()`。
3. bootstrap 内部：collectible ALC + `AssemblyDependencyResolver` + 反射调用。
4. `HostApi` 表**指向 C++ 函数**（永远不是 ALC 内的），ALC 内脚本只通过这张表回宿主。
5. 每次 `ReloadScripts()` 后：`GC.Collect()` + `WaitForPendingFinalizers()` 循环，
   用 `WeakReference` 断言旧 ALC 已回收，**不回收就打日志**（这是你的第一道防线）。
6. 把"连续重载 N 次"加进 CI。

---

## 9. 可直接复制的代码片段

### 9.1 `csproj` —— NativeAOT 原生库

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <OutputType>Library</OutputType>
    <AssemblyName>GameScript</AssemblyName>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
    <Nullable>enable</Nullable>
    <LangVersion>latest</LangVersion>

    <!-- NativeAOT 原生库 -->
    <PublishAot>true</PublishAot>
    <NativeLib>Shared</NativeLib>
    <SelfContained>true</SelfContained>          <!-- .NET 9+ 显式写上，否则 dotnet msbuild -t:publish 会 NETSDK1102 -->
    <RuntimeIdentifier>win-x64</RuntimeIdentifier>

    <!-- 体积/启动优化（可选） -->
    <InvariantGlobalization>true</InvariantGlobalization>
    <OptimizationPreference>Speed</OptimizationPreference>
  </PropertyGroup>
</Project>
```

```powershell
# 构建
dotnet publish -c Release -r win-x64 -p:PublishAot=true -p:NativeLib=Shared -p:SelfContained=true
# 输出: bin\Release\net10.0\win-x64\publish\GameScript.dll
```

### 9.2 C# 导出 + 宿主函数指针表

```csharp
// Exports.cs  —— 必须写在「被 publish 的那个程序集」里，否则不会被导出
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

[assembly: DisableRuntimeMarshalling]   // 关掉运行期 marshalling，规则更简单、更可预测

namespace GameScript;

[StructLayout(LayoutKind.Sequential)]
public struct Vec2 { public float X, Y; }

/// <summary>宿主（C++ 引擎）交给脚本的函数指针表。全部 Cdecl，全部 blittable。</summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct HostApi
{
    public delegate* unmanaged[Cdecl]<int, float, float, int>  CreateEntity;   // (kind, x, y) -> handle
    public delegate* unmanaged[Cdecl]<int, void>               DestroyEntity;  // (handle)
    public delegate* unmanaged[Cdecl]<int, uint, void*>        GetComponent;   // (handle, typeId) -> ptr
    public delegate* unmanaged[Cdecl]<int, int, float>         GetAxis;       // (player, axis) -> value
    public delegate* unmanaged[Cdecl]<byte*, int, void>        Log;           // (utf8, len)
    public delegate* unmanaged[Cdecl]<double, double>          GetTime;       // () -> seconds  (占位)
    public void*                                               UserData;      // 引擎自有上下文
}

public static unsafe class Exports
{
    private static HostApi _host;
    private static float   _t;

    /// <summary>引擎 LoadLibrary 之后第一个调用：把宿主 API 表交进来。</summary>
    [UnmanagedCallersOnly(EntryPoint = "gs_init")]
    public static int Init(HostApi* host, byte* projectRootUtf8)
    {
        if (host == null) return -1;
        _host = *host;                       // 结构体复制；指针表本身归引擎所有，脚本只读
        Log("GameScript initialised.");
        return 0;
    }

    [UnmanagedCallersOnly(EntryPoint = "gs_frame")]
    public static void Frame(float dt)
    {
        _t += dt;

        // 读输入（通过宿主回调）—— 注意：不能从托管代码调自己的 UnmanagedCallersOnly 方法，
        // 但调 _host.* 里的函数指针是完全合法的。
        float moveX = _host.GetAxis(0, 0);

        int e = _host.CreateEntity(1, moveX * 100f, 0f);
        if (e < 0) { Log("CreateEntity failed"); return; }

        // 直接操作引擎组件内存（宿主传回的裸指针，生命周期由引擎保证）
        Vec2* pos = (Vec2*)_host.GetComponent(e, /*typeId*/ 0x504F53);
        if (pos != null) { pos->Y += 1f; }
    }

    [UnmanagedCallersOnly(EntryPoint = "gs_shutdown")]
    public static void Shutdown() => Log("GameScript shutdown.");

    private static void Log(string s)
    {
        if (_host.Log == null) return;
        int n = Encoding.UTF8.GetByteCount(s);
        byte[] buf = new byte[n];                     // 简化示例；实际应用 ArrayPool
        Encoding.UTF8.GetBytes(s.AsSpan(), buf.AsSpan());
        fixed (byte* p = buf) _host.Log(p, n);
    }
}
```

> 说明：`CallConvs = new[]{ typeof(CallConvCdecl) }` 在 **win-x64 上无效果**（该架构只有一种调用约定），
> 可省；显式写只是为了跨平台/可读性。见 [calling-conventions](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/calling-conventions)。

### 9.3 C++ 侧：加载 NativeAOT DLL 并调用导出

```cpp
// nativeaot_host.cpp
#include <windows.h>
#include <cstdint>
#include <cstdio>

struct Vec2 { float x, y; };

// ---- 宿主实现（这些函数会被脚本通过函数指针回调）----
extern "C" {
    static int32_t __cdecl Host_CreateEntity(int32_t kind, float x, float y);
    static void    __cdecl Host_DestroyEntity(int32_t h);
    static void*   __cdecl Host_GetComponent(int32_t h, uint32_t typeId);
    static float   __cdecl Host_GetAxis(int32_t player, int32_t axis);
    static void    __cdecl Host_Log(const char* utf8, int32_t len);
    static double  __cdecl Host_GetTime();
}

struct HostApi {
    int32_t (__cdecl* CreateEntity) (int32_t kind, float x, float y);
    void    (__cdecl* DestroyEntity)(int32_t h);
    void*   (__cdecl* GetComponent) (int32_t h, uint32_t typeId);
    float   (__cdecl* GetAxis)      (int32_t player, int32_t axis);
    void    (__cdecl* Log)          (const char* utf8, int32_t len);
    double  (__cdecl* GetTime)      ();
    void*   UserData;
};

static HostApi g_api{ &Host_CreateEntity, &Host_DestroyEntity, &Host_GetComponent,
                      &Host_GetAxis, &Host_Log, &Host_GetTime, nullptr };

// ---- 脚本导出签名 ----
using gs_init_fn     = int32_t (__cdecl*)(HostApi*, const char*);
using gs_frame_fn    = void    (__cdecl*)(float);
using gs_shutdown_fn = void    (__cdecl*)();

struct ScriptModule {
    HMODULE         h = nullptr;
    gs_init_fn      init = nullptr;
    gs_frame_fn     frame = nullptr;
    gs_shutdown_fn  shutdown = nullptr;
};

bool LoadScriptModule(const wchar_t* dllPath, ScriptModule& out)
{
    out.h = ::LoadLibraryW(dllPath);
    if (!out.h) { std::printf("LoadLibrary failed: %lu\n", ::GetLastError()); return false; }

    out.init     = reinterpret_cast<gs_init_fn>    (::GetProcAddress(out.h, "gs_init"));
    out.frame    = reinterpret_cast<gs_frame_fn>   (::GetProcAddress(out.h, "gs_frame"));
    out.shutdown = reinterpret_cast<gs_shutdown_fn>(::GetProcAddress(out.h, "gs_shutdown"));

    if (!out.init || !out.frame || !out.shutdown) {
        std::printf("missing export(s) — did you set [UnmanagedCallersOnly(EntryPoint=...)]?\n");
        return false;   // 注意：不能 FreeLibrary —— NativeAOT 运行时不支持卸载
    }
    return out.init(&g_api, "D:/game/scripts") == 0;
}

void RunOneFrame(ScriptModule& m, float dt) { if (m.frame) m.frame(dt); }

// ⚠️ 没有 UnloadScriptModule()。NativeAOT 原生库一旦 LoadLibrary 就无法卸载
//    （dlclose/FreeLibrary 不支持）。改脚本 = 重启进程。
```

### 9.4 C++ 侧：hostfxr / nethost 初始化完整序列（推荐路径）

```cpp
// hostfxr_host.cpp  —— 需要 nethost.h / hostfxr.h / coreclr_delegates.h
// 这三个头文件在 .NET SDK 里；也可从 dotnet/runtime 仓库取：
//   src/native/corehost/nethost/nethost.h
//   src/native/corehost/hostfxr.h
//   src/native/corehost/coreclr_delegates.h
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string>

namespace {

hostfxr_initialize_for_runtime_config_fn g_init_fptr   = nullptr;
hostfxr_get_runtime_delegate_fn          g_get_delegate= nullptr;
hostfxr_close_fn                         g_close       = nullptr;

void* LoadLib(const wchar_t* p) { return ::LoadLibraryW(p); }
void* GetExport(void* lib, const char* name) { return reinterpret_cast<void*>(::GetProcAddress((HMODULE)lib, name)); }

} // namespace

// 步骤 1：用 nethost 找到 hostfxr，并取出三个导出
bool LoadHostFxr()
{
    wchar_t buffer[MAX_PATH];
    size_t  size = sizeof(buffer) / sizeof(wchar_t);

    int rc = get_hostfxr_path(buffer, &size, nullptr);   // nethost 导出，__stdcall
    if (rc != 0) { std::printf("get_hostfxr_path failed: 0x%x\n", rc); return false; }

    void* lib = LoadLib(buffer);
    if (!lib) return false;

    g_init_fptr    = (hostfxr_initialize_for_runtime_config_fn)GetExport(lib, "hostfxr_initialize_for_runtime_config");
    g_get_delegate = (hostfxr_get_runtime_delegate_fn)         GetExport(lib, "hostfxr_get_runtime_delegate");
    g_close        = (hostfxr_close_fn)                        GetExport(lib, "hostfxr_close");
    return g_init_fptr && g_get_delegate && g_close;
}

// 步骤 2+3：初始化运行时并拿到 load_assembly_and_get_function_pointer 委托
load_assembly_and_get_function_pointer_fn GetLoadAssemblyFn(const wchar_t* runtimeConfigPath)
{
    hostfxr_handle ctx = nullptr;
    // 传 nullptr 作为 parameters：不指定 host_path / dotnet_root，由 hostfxr 自行推断
    int rc = g_init_fptr(runtimeConfigPath, nullptr, &ctx);
    if (rc != 0 || ctx == nullptr) {
        std::printf("hostfxr_initialize_for_runtime_config failed: 0x%x\n", rc);
        if (ctx) g_close(ctx);
        return nullptr;
    }

    void* fn = nullptr;
    rc = g_get_delegate(ctx, hdt_load_assembly_and_get_function_pointer, &fn);
    if (rc != 0 || !fn) std::printf("get_runtime_delegate failed: 0x%x\n", rc);

    g_close(ctx);   // 委托本身已取出，host context 可以关掉
    return (load_assembly_and_get_function_pointer_fn)fn;
}

// 步骤 4：加载托管程序集并取函数指针
using engine_tick_fn = int (__cdecl*)(void* args, int32_t sizeBytes);

engine_tick_fn LoadEntryPoint(load_assembly_and_get_function_pointer_fn loadFn,
                              const wchar_t* assemblyPath)
{
    // 用一个「不可回收」的桥接类型作为入口；热重载逻辑写在它里面。
    // 也可以用 UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1) 指向 [UnmanagedCallersOnly] 方法。
    const wchar_t* typeName   = L"Engine.Bootstrap, Engine.Bootstrap";
    const wchar_t* methodName = L"Tick";
    const wchar_t* delegateType = nullptr;  // nullptr => 默认 ComponentEntryPoint(IntPtr, int)

    void* fn = nullptr;
    int rc = loadFn(assemblyPath,
                    typeName,
                    methodName,
                    delegateType,
                    nullptr,          // reserved 必须为 nullptr
                    &fn);
    if (rc != 0 || !fn) { std::printf("load_assembly_and_get_function_pointer failed: 0x%x\n", rc); return nullptr; }
    return (engine_tick_fn)fn;
}

// ⚠️ 热重载不要用上面这个委托：它加载进的是「隔离但无法卸载」的 ALC，
//    且返回的函数指针是进程生命周期的、无法释放。
//    正确做法：用一个常驻 bootstrap 方法（hdt_get_function_pointer 拿指针），
//    让它在托管侧自己 new collectible AssemblyLoadContext() 来加载/卸载脚本程序集。
```

### 9.5 C# 侧：collectible ALC 热重载（托管宿主路径的核心）

```csharp
// Bootstrap.cs  —— 放在「不可回收」的桥接程序集里
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;

namespace Engine;

internal sealed class ScriptAlc : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;
    public ScriptAlc(string mainAssemblyPath) : base(isCollectible: true)
        => _resolver = new AssemblyDependencyResolver(mainAssemblyPath);

    protected override Assembly? Load(AssemblyName name)
    {
        var p = _resolver.ResolveAssemblyToPath(name);
        return p != null ? LoadFromAssemblyPath(p) : null;   // null => 依赖走 Default ALC
    }
}

public static class Bootstrap
{
    private static ScriptAlc? _alc;
    private static MethodInfo? _tick, _shutdown;
    private static WeakReference? _lastAlcRef;

    // 入口：引擎通过 hostfxr 拿到这个方法的指针。签名必须与引擎约定一致。
    public static int LoadScripts(string assemblyPath, IntPtr hostApi)
    {
        Unload();
        var alc = new ScriptAlc(assemblyPath);
        var asm = alc.LoadFromAssemblyPath(assemblyPath);
        var t   = asm.GetType("GameScript.Entry", throwOnError: true)!;
        // 注意：bridge 把 hostApi 交给脚本，脚本侧用 Marshal/函数指针表接住
        t.GetMethod("Init")!.Invoke(null, new object[] { hostApi });
        _tick     = t.GetMethod("Tick")!;
        _shutdown = t.GetMethod("Shutdown")!;
        _alc      = alc;
        _lastAlcRef = new WeakReference(alc, trackResurrection: true);
        return 0;
    }

    // ⚠️ 不要跨重载保留任何 MethodInfo / Type / 实例到 static 字段里！
    //    （除了当前的，且必须在 Unload 时清掉）—— 否则 ALC 永远卸不掉。
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void Tick(float dt) => _tick?.Invoke(null, new object[] { dt });

    public static void Unload()
    {
        try { _shutdown?.Invoke(null, null); } catch { /* 容错 */ }

        _tick = null; _shutdown = null;          // 清掉 ALC 内的引用
        _alc?.Unload();
        _alc = null;

        // 验证卸载是否真的完成 —— 这是你的第一道防线
        for (int i = 0; i < 10 && _lastAlcRef is { IsAlive: true }; i++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }
        if (_lastAlcRef is { IsAlive: true })
            Console.Error.WriteLine("[hotreload] WARNING: previous ALC was NOT collected — leak!");
    }
}
```

> 这段代码把 5.2/6.3 提到的所有坑都显式处理了：ALC 子类不持有脚本字段、卸载后清引用、
> 用 `WeakReference` 断言回收。**这就是"collectible ALC 能不能真的卸载"的工程答案 ——
> 能，但必须自己验证，不能假设。**

---

## 10. 未能验证项 / 低置信度清单

| # | 事项 | 状态 |
|---|---|---|
| 1 | **NativeAOT 小型原生库的精确 DLL 体积 / 构建时间 / `LoadLibrary` 初始化开销** | ❌ 未实测（沙箱无 NuGet 网络）。文中所有相关数字均来自第三方，且是 exe 形态而非原生库形态 |
| 2 | #132562（.NET 10 泛型虚派发在反复 ALC 卸载后挂死）的修复**是否已进入 10.0.11 / 10.0.12** | ⚠️ 未验证。10.0.12 release notes 只列 CVE，未提该修复；issue 本身已于 2026-09-03 关闭 |
| 3 | 第三方 `[DllExport]`（UnmanagedExports / DllExport）能否用于 NativeAOT | ⚠️ 未找到任何一手资料；我倾向于认为**不能**（依赖 IL 重写/JIT 侧机制），但这是推断 |
| 4 | 同一进程内并存/加载**多个** NativeAOT 共享库是否被支持 | ⚠️ 未找到官方说明 |
| 5 | `NativeLib` MSBuild 属性的**正式文档页** | ⚠️ 未找到。只有 sample README 与 [issue #69369](https://github.com/dotnet/runtime/issues/69369) |
| 6 | Windows **x86** 的 NativeAOT 是否官方支持 | ⚠️ 未验证。.NET 10.0.12 的包清单里**存在** `Microsoft.NETCore.App.Runtime.NativeAOT.win-x86`，但包存在 ≠ 官方支持（你对 x64 也用不到） |
| 7 | `CallConvSuppressGCTransition` 与 `UnmanagedCallersOnly` 函数指针组合导致崩溃的那篇 StackOverflow | ⚠️ 抓取返回 **403**，未读到正文。只知道标题，不知道确切条件 |
| 8 | "小型 C++ 引擎用 NativeAOT 原生库嵌入 C# 脚本"的完整公开技术写实 | ⚠️ 未找到（可能不存在） |
| 9 | `AssemblyLoadContext` 内带 `UnmanagedCallersOnly` 导出的程序集卸载的**专门**文档/gotcha 说明 | ⚠️ 未找到专门资料。文中结论是从「卸载条件」+「不得从托管调用」两条官方规则推导的 |
| 10 | Godot 的 NativeAOT 导出（`bugnet.io` 那篇文章） | ❌ 来源质量低，**未采信**，不引为证据 |
| 11 | 沙箱环境只能读不能跑构建：本机仅装有 **.NET 9.0.305 SDK**，且 `nuget.org` 不可达，因此**没有做任何真机构建验证** | 事实 |
