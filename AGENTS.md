# AGENTS.md

Bamboo is a Windows-only OpenGL 2D game engine written in C++17 using the ECS pattern (via `entt`). CMake build, MSVC/Visual Studio only — no Linux/macOS support.

## Build (Windows / MSVC)

Configure from `Source/` (see `Source/生成工程命令.txt`):

```
cd Source
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

- Output goes to `Source/build/bin/<Config>/` and `Source/build/lib/`.
- `Source/build/` is untracked/ignored; do not commit it.
- `MSVC` targets compile with `/utf-8`. Source comments and commit messages are written in **Chinese** — match that convention.

## Targets

- `Bamboo` — static library, the engine core (subdir `Source/Bamboo/`). Sources are gathered via `file(GLOB_RECURSE .../*.cpp *.h)`, so new files are picked up automatically on reconfigure; a full re-run of CMake may be needed after adding a file.
- `Sandbox` — executable, the Breakout demo (`BreakoutApp` in `Source/Sandbox/BreakoutDemo/`). Working directory is auto-set to the output `bin/<Config>` dir via `VS_DEBUGGER_WORKING_DIRECTORY`.
- `Editor` — executable, ImGui-based editor (`Source/Editor/`). Note its CMake links `Bamboo`, `imgui`, `spdlog`, `stb`, `glfw` explicitly.

## Asset root gotcha (important)

`Source/Bamboo/Config.in.h` is `configure_file`'d at CMake configure time into `build/generated/Config.h`, baking the **absolute** path `BAMBOO_ASSET_ROOT` → `Source/BambooAssets/`.

- Assets are resolved at runtime by joining `BAMBOO_ASSET_ROOT` with `Shaders/` and `Texture2d/` (see `OpenGLShader.cpp`, `OpenGLTexture.cpp`, `ImageAsset.cpp`).
- If the repo is moved/cloned elsewhere, the stale path remains baked — you **must** re-run CMake (reconfigure) or asset loading will fail silently.

## Conventions / gotchas

- No test framework, no lint/typecheck config in the repo. Verification is just a successful build.
- The engine is ECS: entities hold components (`TransformComponent`, `SpriteRendererComponent`, `CameraComponent`, physics colliders, etc.); logic lives in `ISystem` subclasses (`RendererSystem`, `PhysicsSystem`, ...) registered on `Scene`. `Scene::Update(dt)` runs the four phase buckets in a fixed order ending with the `Render` bucket (see the next entry).
- **`docs/README.md` is the single entry point for project docs** (verified against code on 2026-09-22). It covers: current capability/state, the five architectural root causes, the defect inventory (P0~P5), target architecture + migration path, the game ladder (G1~G5), editor design, C# scripting constraints, test/acceptance criteria, roadmap, and a learning map. Read it before changing architecture.
- The only other doc is `docs/research_csharp_embedding.md` — C# embedding technical research (91 citations, copy-pasteable code, an 11-row "could not verify" table). Read it before touching the scripting layer.
- Defect IDs (`P0-1` … `P5-2`) live in `docs/README.md` §5. Reference them from code comments instead of repeating explanations.
- Systems are no longer run from a `Scene`-owned list: `Scene::Update` delegates to `SystemRegistry`, which executes systems in four phase buckets — `Logic → Transform → Physics → Render`. A new system declares its stage by overriding `ISystem::GetPhase()` (default is `Logic`, so forgetting it silently puts the system in the Logic bucket). `SystemRegistry::Register<T>()` also calls `Init()`.
- Sandbox/Editor apps subclass `Application` and call `app.Run()` from `main` (`Sandbox/main.cpp`, `Editor/Source/main.cpp`).
- Third-party libs are vendored under `Source/ThirdParty/` (`GLFW`, `glad`, `entt`, `spdlog`, `stb`, `imgui`). Do not add new deps without adding them to `ThirdParty/CMakeLists.txt`.

## Comments（注释约定）

Target: comments are **sparse but trustworthy**. A wrong comment is worse than no comment — it makes readers trust a lie.

**Do write** (these are the high-value ones — a reader cannot infer them from the code):

- Non-obvious conventions: coordinate system (world unit = 1 pixel), matrix storage order (row-major for both `Matrix3`/`Matrix4`), angle-vs-radian.
- Invariants and preconditions: "entity must come from `Scene::CreateEntity`", "component pointers are only valid for the current call, never cache across frames".
- Why something is the way it is (a workaround, a deliberate limitation, a performance tradeoff).
- Known defects, **with the defect ID from `docs/README.md` §5**: `// 已知缺陷：… 见 P1-6`. This is what keeps the code honest about its own gaps.
- TODOs that are specific and attributable: `// TODO(相机): 投影矩阵未惰性重算，见 P1-6`. A bare `// todo` with no owner and no concrete next step is noise — either make it specific or delete it.

**Do not write**:

- Restating the code (`GetWidth()` → `// 获取宽度`).
- Commented-out code. If it is an old implementation, delete it — `git log` has it. If it is an experiment, put it on a branch.
- Template placeholders left unfilled (`@author <NAME>`, `<EMAIL>`).
- `@param` docs that contradict the implementation. If the param is ignored, say so.

**Style**: use `///` for public-API doc comments, `//` inline. Write in Chinese, matching existing code. Keep `@brief`/`@param` only where they add information beyond the signature.
