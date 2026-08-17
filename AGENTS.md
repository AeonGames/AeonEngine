# AeonEngine — Agent Instructions

Cross-platform, plugin-based 3D game engine in C++20. Vulkan (primary), OpenGL 4.5 (secondary),
and native Metal 3 on Apple Silicon/macOS 13+. Protocol Buffers for every asset, Blender for
content authoring.
Read [README.md](README.md) for the feature list, platform matrix and full dependency install steps.
**Work in progress** — expect churn.

## Build, test, run

On Windows, use the **MSYS2 MinGW bash** integrated terminal. The `cmd.exe` automation profile does
not have `make` on `PATH`. On macOS, use CMake's generator-neutral build command: a stale `Makefile`
may remain in a build tree configured with Ninja, so trust `CMAKE_GENERATOR` in `CMakeCache.txt`.

| Task | Windows/MSYS2 | macOS/Apple Silicon |
| --- | --- | --- |
| Incremental build | `cd mingw64 && make -j$(nproc)` | `cmake --build clang64 --parallel "$(sysctl -n hw.ncpu)"` |
| Configure from scratch | `cmake -G "MSYS Makefiles" -B build -DCMAKE_BUILD_TYPE=Release -DUSE_AEONGUI=ON` | `cmake -G Ninja -B clang64 -DCMAKE_BUILD_TYPE=Debug -DUSE_AEONGUI=ON -DBUILD_METAL_RENDERER=ON` |
| Tests | `cd mingw64 && ctest --output-on-failure` | `ctest --test-dir clang64 --output-on-failure` |
| Regenerate shaders only | `cd mingw64 && make shader-pipelines` | `cmake --build clang64 --target shader-pipelines` |
| Cook a Blender asset | `cd mingw64 && make aerin` | `cmake --build clang64 --target aerin` |

- One gtest binary, `mingw64/bin/unit-tests.exe`, registered with CTest with the **repo root as its
  working directory** — tests load assets via relative paths.
- Run the game **from the repo root** (`game/config` is resolved relatively):
  `PATH="$PWD/mingw64/bin:$PATH" ./mingw64/bin/game.exe -r OpenGL -s scenes/main.txt`
  (`-r Vulkan`, `-f` for fullscreen). On macOS:
  `PATH="$PWD/clang64/bin:$PATH" ./clang64/bin/game -r Metal -s scenes/main.txt`.
  Renderer names are exactly `OpenGL`, `Vulkan`, and `Metal` where those plugins are built.
- Metal builds require `glslangValidator`, `spirv-cross`, the macOS SDK, and Apple's separately
  downloaded Metal compiler. Install the latter with `xcodebuild -downloadComponent MetalToolchain`.
  CMake discovers `glslangValidator`/`spirv-cross` through `PATH`; clear a cached `*-NOTFOUND` value
  or reconfigure after fixing tool discovery.
- CI ([.github/workflows](.github/workflows)) builds MSVC, MSYS2 (mingw64/ucrt64/clang64), Linux
  (gcc/clang), macOS — all with `-DUSE_AEONGUI=ON` followed by `ctest`. Keep changes portable.
- Git LFS is required (`*.msh`, `*.b64`). Without `git lfs install` those files are pointer stubs.
- Assets cooked from `.blend` files (`aerin`, `backdrop`, `polesign`, `sponza`) are **optional,
  opt-in targets**, never part of `ALL`. CMake picks the newest installed Blender; when none is
  found it prints a status message and skips those targets instead of failing. Each is stamped
  under `<build>/blender-assets`, so cooking one lasts until its `.blend`, its textures or an
  exporter changes. `blender-python-venv` builds a venv out of Blender's own interpreter under
  `<build>/blender-venv` and installs the protobuf runtime the generated `*_pb2.py` ask for.
  `scenes/aerin.txt` and the `CharacterLibrary` tests need `make aerin` first, and
  `scenes/sponza` needs `make sponza` (~5 min, 91 models); `scenes/main.txt` deliberately needs
  neither. `add_blender_asset(... MODE SCENE)` cooks a scene `.blend` (whole scene plus one model
  per mesh datablock); the default `MODE MODEL` cooks the `.blend` into a single model.

## Layout

| Path | Role |
| --- | --- |
| [include/aeongames](include/aeongames) | Public API, `.hpp`, symbols marked with the `DLL` macro from [Platform.hpp](include/aeongames/Platform.hpp) |
| [engine/core](engine/core) | Engine implementation (Scene, Node, Resource, Octree, Package…) |
| [engine/components](engine/components) | Components + their factory registration in [Plugin.cpp](engine/components/Plugin.cpp) |
| [engine/renderers](engine/renderers) | `opengl/`, `vulkan/`, and Apple-only `metal/` backends, each built as a self-contained plugin |
| [engine/include/Factory.h](engine/include/Factory.h) | `FactoryDefinition` / `FactoryImplementation` macros behind every `Construct*`/`Register*Constructor` |
| [proto](proto) | `.proto` schemas; every asset format is a protobuf message |
| [assets/shadercode](assets/shadercode) | GLSL sources (the shipped `game/shaders/*.txt` are generated) |
| [tools/aeontool](tools/aeontool) | CLI: `convert`, `pack`, `base64`, `pipeline` |
| [tools/metalshader](tools/metalshader) | macOS shader cooker: GLSL/SPIR-V reflection to validated MSL 3.0 and Metal interface metadata |
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
- **Keep renderer behavior in parity.** A backend-neutral feature generally needs Vulkan, OpenGL,
  and Metal implementations plus matching shader branches. For an explicitly Metal-only task, do
  not change OpenGL/Vulkan merely to share code; preserve their behavior and validate Metal against
  the same public `Renderer` contract.
- **Never commit `docs/*.pdf`** — local, gitignored reference papers.
- **Never commit Synty assets.** The `synty/` SIDEKICK packages, anything extracted from them, and
  anything cooked from them (`.msh`/`.skl`/`.mdl`/`.mtl`, baked palette PNGs) are licensed art that
  may not be redistributed — derived files included. `synty/` is gitignored; keep cooked output
  outside the worktree or under an ignored path, and never stage it into this public repo. It
  belongs in the internal network repo instead. Tooling that *reads* those assets is fine to commit.

## Architecture must-knows

- **Plugins**: each backend DLL exports a `PluginModuleInterface PMI` (see
  [Plugin.hpp](include/aeongames/Plugin.hpp)); `StartUp()` calls `RegisterRendererConstructor("Vulkan", …)`
  (or `"OpenGL"`/`"Metal"`). Metal requires Apple Silicon, macOS 13+, Metal 3, and argument-buffer
  tier 2; it is enabled by default only for Apple arm64 builds.
  Components/resources register the same way. Registration is keyed by `StringId` (CRC32), so
  `"Model"_crc32` and `ConstructComponent("ModelComponent")` reach the same map.
- **Two per-frame scene sweeps**: `Update` is read-**write** (transforms, animation, anything needed
  to be ready to render); the render sweep is **read-only** and only collects. Components must not
  call the `Renderer` directly — they append `RenderItem`s via `Collect`, and a later submit stage
  decides batching/instancing. Skinned vertex buffers only exist after the compute `Skin` pre-pass.
- **Culling** is two-layered: CPU octree frustum cull building the render queue, then a GPU compute
  cull (`cull.comp`) for indexed static items in the shading pass. Vulkan/OpenGL use shared geometry
  pools; Metal retains per-mesh buffers and GPU-compacts same-mesh batches into fixed, zero-padded
  indirect command ranges. The depth pre-pass feeds a max-reduced Hi-Z pyramid before this cull.
- **Metal frame ownership**: `MetalRenderer` owns the device, command queue, resource caches, bindless
  tables, and renderer-owned pipelines. `MetalWindow` owns each `CAMetalLayer`, triple-buffered frame
  pools, attachments, shadow maps, command buffers/encoders, capture state, Hi-Z, and environment
  cubes. End a render encoder before compute and start a new encoder afterward; encoder boundaries
  on one command buffer provide ordering, so `MetalRenderer::Barrier` is intentionally a no-op.
- **Metal frame flow** mirrors the shared `Renderer::RenderScene` protocol: cluster build, shadow
  passes, depth pre-pass, Hi-Z + light cull, shading/indirect submission, debug geometry and GUI,
  skybox, then HDR/deferred-specular tonemap into the drawable. Do not bypass this with a separate
  Metal frame loop.
- **Metal bindings**: descriptor set number maps to the MSL argument-buffer index; vertex data uses
  buffer index 8. Bindless combined texture/sampler pairs live in set 2 (16,384 shader slots), and
  material records live in set 7. Per-frame blocks for clustered shading are packed into set 0.
  Consume generated `ShaderInterface` metadata by resource name/hash rather than hardcoding MSL
  symbol names.
- **Metal overlays** consume premultiplied BGRA8 pixels (the `GuiOverlay`/AeonGUI contract), use a
  `BGRA8Unorm` upload texture, and blend with source factor ONE. Treating that buffer as straight
  RGBA makes a transparent full-window GUI canvas obscure or discolor the 3D scene.
- **Ownership**: containers own children through `std::unique_ptr`; returned raw pointers are
  non-owning. Per-frame scratch memory comes from the frame memory pools, reset each frame.

## Generated files — never hand-edit

| Artifact | Regenerated by | Edit instead |
| --- | --- | --- |
| `game/shaders/*.txt`, `*.pln` | `make shader-pipelines` / `cmake --build clang64 --target shader-pipelines` | [assets/shadercode](assets/shadercode) + its `PIPELINES` list; Metal variants/interface metadata are added by `metal-shader-tool` |
| `engine/renderers/opengl/glDeclarations.h` / `glDefinitions.h` / `glAssignments.h` / `glProxyFunctions.*` | CMake **configure** time | `engine/renderers/opengl/glFunctions.txt` |
| `*.pb.h` / `*.pb.cc`, Blender python protobuf | protobuf codegen | [proto](proto) |
| `.vscode/*.json`, `Doxyfile`, `.git/hooks/{pre-commit,commit-msg}` | CMake configure | the matching template in [cmake](cmake) |
| `game/**/*.png`, `game/images/*.svg`, `*.msh`/`*.mtl`/`*.skl`/`*.cln` | asset targets / aeontool | source assets under [assets](assets) or the Blender exporters |
| `game/aerin/**`, `game/backdrop/**`, `game/polesign/**` (`*.mdl`/`*.msh`/`*.mtl`/`*.skl`/`*.anm`/`*.png`) | `make aerin` / `backdrop` / `polesign` | the matching `.blend` under [assets](assets) |
| `game/sponza/**` and `game/scenes/sponza.scn` | `make sponza` | [assets/sponza/sponza.blend](assets/sponza/sponza.blend) |

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
- Metal: SPIRV-Cross compacts resources within each argument buffer. The Metal shader cooker records
  the resulting MSL argument IDs in Metal-only `ShaderInterface` metadata; raw GLSL `binding`
  values are not necessarily the runtime argument IDs. Combined samplers reserve both texture and
  sampler IDs. Regenerate pipelines after changing resources or bindings.
- Metal: runtime-sized SSBOs can produce hidden `spvBufferSizeConstants`; `MetalPipeline` discovers
  and populates those through Metal reflection. Omitting the size constant causes out-of-bounds GPU
  reads even when the visible resource bindings look correct.
- Metal: `CAMetalLayer` and final tonemap targets use `BGRA8Unorm`, not an sRGB format, because
  `tonemap.frag` performs the sRGB transfer explicitly. Scene color/G-buffer attachments are
  `RGBA16Float`; applying an sRGB drawable would encode twice.
- Metal validation: run focused GPU tests with
  `MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1 MTL_SHADER_VALIDATION_REPORT_TO_STDERR=1`.
  The parity suite includes compute, skinning, bindless materials, shadows, Hi-Z/indirect culling,
  environment/tonemap, GUI, debug geometry, and the shipped main scene. Metal's "redundant set"
  messages are performance warnings; command-buffer errors, invalid loads, or format mismatches are
  correctness failures.
- Tools must `create_directories()` and check `std::ofstream` before writing — `game/shaders/` is
  gitignored and absent on a fresh checkout, and an unchecked stream silently exits 0.
