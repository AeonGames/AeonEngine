# AeonEngine — Agent Instructions

Cross-platform, plugin-based 3D game engine in C++20. Vulkan (primary) + OpenGL 4.5 (secondary),
Protocol Buffers for every asset, Blender for content authoring.
Read [README.md](README.md) for the feature list, platform matrix and full dependency install steps.
**Work in progress** — expect churn; check [TODO.md](TODO.md) for direction.

## Build, test, run

Use the **MSYS2 MinGW bash** integrated terminal. The `cmd.exe` automation profile does not have
`make` on `PATH` and the default build task will fail there.

| Task | Command |
| --- | --- |
| Incremental build (existing tree) | `cd mingw64 && make -j$(nproc)` — or the VS Code `build` task |
| Configure from scratch | `cmake -G "MSYS Makefiles" -B build -DCMAKE_BUILD_TYPE=Release -DUSE_AEONGUI=ON` |
| Tests | `cd mingw64 && ctest --output-on-failure` |
| Regenerate shaders only | `cd mingw64 && make shader-pipelines` |

- One gtest binary, `mingw64/bin/unit-tests.exe`, registered with CTest with the **repo root as its
  working directory** — tests load assets via relative paths.
- Run the game **from the repo root** (`game/config` is resolved relatively):
  `PATH="$PWD/mingw64/bin:$PATH" ./mingw64/bin/game.exe -r OpenGL -s scenes/main.txt`
  (`-r Vulkan`, `-f` for fullscreen). Renderer names are exactly `OpenGL` and `Vulkan`.
- CI ([.github/workflows](.github/workflows)) builds MSVC, MSYS2 (mingw64/ucrt64/clang64), Linux
  (gcc/clang), macOS — all with `-DUSE_AEONGUI=ON` followed by `ctest`. Keep changes portable.
- Git LFS is required (`*.msh`, `*.b64`). Without `git lfs install` those files are pointer stubs.

## Layout

| Path | Role |
| --- | --- |
| [include/aeongames](include/aeongames) | Public API, `.hpp`, symbols marked with the `DLL` macro from [Platform.hpp](include/aeongames/Platform.hpp) |
| [engine/core](engine/core) | Engine implementation (Scene, Node, Resource, Octree, Package…) |
| [engine/components](engine/components) | Components + their factory registration in [Plugin.cpp](engine/components/Plugin.cpp) |
| [engine/renderers](engine/renderers) | `opengl/` and `vulkan/` backends, each built as a self-contained plugin DLL |
| [engine/include/Factory.h](engine/include/Factory.h) | `FactoryDefinition` / `FactoryImplementation` macros behind every `Construct*`/`Register*Constructor` |
| [proto](proto) | `.proto` schemas; every asset format is a protobuf message |
| [assets/shadercode](assets/shadercode) | GLSL sources (the shipped `game/shaders/*.txt` are generated) |
| [tools/aeontool](tools/aeontool) | CLI: `convert`, `pack`, `base64`, `pipeline` |
| [tools/worldeditor](tools/worldeditor), [tools/blender](tools/blender) | Qt6 scene editor, Blender exporters |

## Hard rules

- **No recursive functions.** Write tree/graph traversals iteratively with an explicit stack;
  prefer a fixed-size `std::array` when depth is bounded (see `Octree::ForEachCell` in
  [engine/core/Octree.cpp](engine/core/Octree.cpp)). No heap allocation for traversal state.
- **Flag architecture divergences before coding.** When a task touches engine architecture, state
  up front where AeonEngine differs from standard practice and present it as part of the plan
  instead of silently adding functions or virtuals.
- **Pass domain types, not loose primitives.** Take `const AABB&`, not `(origin, radii)`; take a
  `Plane`, not `(nx, ny, nz, d)`. If the right type does not exist yet, ask before inventing one.
- **Data-driven first.** A goal of the engine is configuring behaviour through assets/config rather
  than C++ edits. Prefer extending an asset schema or pipeline definition over hardcoding.
- **Keep both renderers in sync.** A feature landed in Vulkan almost always needs the OpenGL
  counterpart (and the shader `#ifdef VULKAN` branch) in the same change.
- **Never commit `docs/*.pdf`** — local, gitignored reference papers.

## Architecture must-knows

- **Plugins**: each backend DLL exports a `PluginModuleInterface PMI` (see
  [Plugin.hpp](include/aeongames/Plugin.hpp)); `StartUp()` calls `RegisterRendererConstructor("Vulkan", …)`.
  Components/resources register the same way. Registration is keyed by `StringId` (CRC32), so
  `"Model"_crc32` and `ConstructComponent("ModelComponent")` reach the same map.
- **Two per-frame scene sweeps**: `Update` is read-**write** (transforms, animation, anything needed
  to be ready to render); the render sweep is **read-only** and only collects. Components must not
  call the `Renderer` directly — they append `RenderItem`s via `Collect`, and a later submit stage
  decides batching/instancing. Skinned vertex buffers only exist after the compute `Skin` pre-pass.
- **Culling** is two-layered: CPU octree frustum cull building the render queue, then a GPU compute
  cull (`cull.comp`) for static pooled indexed items in the shading pass.
- **Ownership**: containers own children through `std::unique_ptr`; returned raw pointers are
  non-owning. Per-frame scratch memory comes from the frame memory pools, reset each frame.

## Generated files — never hand-edit

| Artifact | Regenerated by | Edit instead |
| --- | --- | --- |
| `game/shaders/*.txt`, `*.pln` | `make shader-pipelines` | [assets/shadercode](assets/shadercode) + its `PIPELINES` list |
| `engine/renderers/opengl/glDeclarations.h` / `glDefinitions.h` / `glAssignments.h` / `glProxyFunctions.*` | CMake **configure** time | `engine/renderers/opengl/glFunctions.txt` |
| `*.pb.h` / `*.pb.cc`, Blender python protobuf | protobuf codegen | [proto](proto) |
| `.vscode/*.json`, `Doxyfile`, `.git/hooks/{pre-commit,commit-msg}` | CMake configure | the matching template in [cmake](cmake) |
| `game/**/*.png`, `game/images/*.svg`, `*.msh`/`*.mtl`/`*.skl`/`*.cln` | asset targets / aeontool | source assets under [assets](assets) or the Blender exporters |

Calling a GL entry point that is not listed in `glFunctions.txt` compiles but yields a null pointer
at runtime — add the name there first.

## Pitfalls that have bitten before

- OpenGL: the value returned by `OpenGLPipeline::GetSamplerLocation` is the **texture unit**
  (`layout(binding = N)`), not a uniform location. Mixing them up silently renders black.
- OpenGL/NVIDIA mis-fetches `std140` UBO members past the first, so the `Globals` block is an
  `std430` SSBO on the OpenGL branch while Vulkan keeps the UBO. Do not "simplify" that back.
  A full GL-SPIR-V migration was investigated and **shelved**: glslang rejects
  `GL_ARB_bindless_texture` when targeting SPIR-V.
- Vulkan: a pipeline layout may declare at most 16 descriptor sets (engine-chosen array size in
  `VulkanPipeline.cpp`), gaps filled with a shared empty layout.
- Tools must `create_directories()` and check `std::ofstream` before writing — `game/shaders/` is
  gitignored and absent on a fresh checkout, and an unchecked stream silently exits 0.
