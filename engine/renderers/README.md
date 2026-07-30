# AeonEngine Renderer Backends

Everything under `engine/renderers/` is a **self-contained plugin DLL** implementing the
[`Renderer`](../../include/aeongames/Renderer.hpp) interface. Two backends ship today:

| Directory | CMake target | Registered name | Notes |
| --- | --- | --- | --- |
| [opengl](opengl) | `OpenGLRenderer` | `"OpenGL"` | OpenGL 4.5 core + `GL_ARB_bindless_texture`; entry points loaded from [glFunctions.txt](opengl/glFunctions.txt) |
| [vulkan](vulkan) | `VulkanRenderer` | `"Vulkan"` | Vulkan 1.x; GLSL→SPIR-V through glslang ([SPIR-V/](vulkan/SPIR-V)), reflection through SPIRV-Reflect, MoltenVK on macOS |

For a frame-by-frame walkthrough of the rendering path — clustered forward shading, shadow passes,
HDR + tone map, image-based lighting, compute skinning — read
[vulkan/RENDER_PIPELINE.md](vulkan/RENDER_PIPELINE.md). The passes it describes are shared by both
backends.

> A feature landed in one backend almost always needs its counterpart (and the shader `#ifdef VULKAN`
> branch) **in the same change**. See [AGENTS.md](../../AGENTS.md).

## 1. What the engine owns vs. what a backend owns

`Renderer::RenderScene` is **non-virtual** and lives in [../core/Renderer.cpp](../core/Renderer.cpp).
It owns the per-frame protocol so the sequence is written exactly once; a backend implements only the
individual step primitives.

Also owned by the base class — do not reimplement per backend:

- the step order below;
- the debug-geometry step (ground grid, node AABBs, octree cells, camera frustums), built on the
  public draw API and gated by `SetDebugRendering`;
- `FilterLightsByType`, which every backend's `SetLights` must call before uploading;
- the per-caster point-shadow cache (a cube map is re-rendered only when the light or the scene's
  shadow geometry changed);
- the opt-in GPU benchmark driven by `AEON_BENCH_FRAMES` / `AEON_BENCH_WARMUP`.

One `RenderScene` frame:

1. return immediately when `IsDeviceLost()`;
2. `BeginRender(window, lightingPipeline)` — dispatches compute stage 0 (cluster build) and opens the
   color pass;
3. for each spot, point and directional caster: `Scene::BuildRenderQueue(lightFrustum)` →
   `Begin*ShadowPass` → `SubmitRenderQueue(ShadowPass)` → `End*ShadowPass`;
4. `Scene::BuildRenderQueue(GetFrustum(window))` for the camera;
5. `SubmitRenderQueue(DepthPrePass)` → `EndDepthPrePass` — mark active clusters, barrier, light-cull
   compute, reopen the color pass;
6. `SubmitRenderQueue(Shading)`;
7. `SubmitDebugGeometry` when debug rendering is enabled;
8. `RenderOverlay` when a GUI overlay was passed;
9. `EndRender`.

Everything that precedes the call — `BeginFrame`, `SetViewMatrix` / `SetProjectionMatrix`,
`SetLights`, `SetGlobals`, `SetEnvironmentMap` and the `Skin` compute pre-pass — stays in the
application loop, because compute work must be recorded before the first render pass opens.

## 2. Plugin anatomy

A backend is a `SHARED` library exporting a single `PluginModuleInterface PMI` (see
[Plugin.hpp](../../include/aeongames/Plugin.hpp)); `StartUp` registers the constructors and
`Shutdown` unregisters them:

```cpp
extern "C"
{
    bool MyStartUp()
    {
        const bool legacy = AeonGames::RegisterRendererConstructor ( "MyAPI",
            [] ( void* aWindow ) { return std::make_unique<MyRenderer> ( aWindow ); } );
        const bool configurable = AeonGames::RegisterRendererConstructorWithSettings ( "MyAPI",
            [] ( void* aWindow, AeonGames::RendererSettings aSettings )
        { return std::make_unique<MyRenderer> ( aWindow, aSettings ); } );
        return legacy && configurable;
    }

    void MyShutdown()
    {
        AeonGames::UnregisterRendererConstructor ( "MyAPI" );
        AeonGames::UnregisterRendererConstructorWithSettings ( "MyAPI" );
    }

    PLUGIN PluginModuleInterface PMI
    {
        "MyAPI Renderer", "Implements a MyAPI Renderer", MyStartUp, MyShutdown
    };
}
```

- Registration is keyed by `StringId` (CRC32), and `Renderer::GetName()` **must return the same
  string** — it is what selects per-renderer shader variants at pipeline load.
- Register **both** constructors: the settings-less overload is the legacy path, the engine prefers
  the one taking `RendererSettings`.
- CMake: `add_library(<Target> SHARED …)`, then
  `set_property(GLOBAL APPEND PROPERTY PLUGINS <Target>)`, plus `PREFIX ""` under MinGW/MSYS so the
  DLL name matches the config entry. Gate the `add_subdirectory` behind a `BUILD_<API>_RENDERER`
  option in [../CMakeLists.txt](../CMakeLists.txt).
- The plugin is loaded at runtime from [game/config](../../game/config) through a
  `Plugin: "<TargetName>"` line and selected with `game.exe -r <RegisteredName>`.
- Keep each plugin DLL self-contained: it may link third-party compilers (glslang) **statically**,
  but must not depend on the other renderer plugin.

## 3. The interface to implement

Required (pure virtual in [Renderer.hpp](../../include/aeongames/Renderer.hpp)):

| Group | Members |
| --- | --- |
| Identity | `GetName`, `GetSettings` |
| Resources | `LoadMesh`/`UnloadMesh`, `LoadPipeline`/`UnloadPipeline`, `LoadMaterial`/`UnloadMaterial`, `LoadTexture`/`UnloadTexture` |
| Surfaces | `AttachWindow`/`DetachWindow`, `ResizeViewport`, `SetClearColor`, `SetViewMatrix`, `SetProjectionMatrix`, `SetLights` |
| Frame protocol | `BeginFrame`, `BeginRenderPass`, `BeginRender`, `EndDepthPrePass`, `EndRender`, `Finish`, `BeginShadowPass`/`EndShadowPass` |
| Submission | `Render`, `Dispatch`, `Skin`, `Barrier`, `RenderOverlay`, and the protected `SubmitRenderQueue` |
| Per-frame memory | `AllocateSingleFrameUniformMemory`, `AllocateSingleFrameStorageMemory` |
| Introspection | `GetFrustum`, `GetProjectionMatrix`, `GetFrameLightGrid`, `GetFrameClusterActive`, and the protected `IsValidWindow` |

Optional overrides — the defaults are no-ops, so a new backend boots before it has them:

| Override | Default behaviour |
| --- | --- |
| `SetGlobals` | ignored; the `Globals` block keeps its initial value (constant ambient) |
| `SetEnvironmentMap` | ignored; no skybox and no image-based lighting |
| `SetSpotShadowParams`, `Begin`/`EndSpotShadowPass` | ignored; spot lights stay unshadowed |
| `SetPointShadowParams`, `Begin`/`EndPointShadowPass` | ignored; point lights stay unshadowed |
| `RenderInstanced` | loops `Render` once per matrix — correct, just one draw per instance |
| `RecordGpuTimestamp` / `ReadGpuTimestamps` | the benchmark reports nothing |
| `IsDeviceLost` | always `false` |

> `Renderer` deliberately has **no out-of-line non-pure virtual**. Keep the default bodies of optional
> overrides inline in the header: giving the class a key function would move its weak vtable into a
> single object file and break the separately linked plugin DLLs.

Semantics worth knowing before implementing:

- `Render` / `RenderInstanced` take a `RenderPass` ([RenderItem.hpp](../../include/aeongames/RenderItem.hpp)).
  `ShadowPass` and `DepthPrePass` substitute the renderer-owned shadow-depth / cluster-mark pipeline
  for the item's own pipeline; `Shading` uses it as authored.
- `Dispatch` group counts are **workgroups**, not invocations, and on backends with an explicit render
  pass must be recorded between `BeginFrame` and `BeginRenderPass`. Storage blocks arrive as
  `StorageBufferBinding{ crc32(blockName), accessor }`; blocks the pipeline does not declare are
  silently ignored.
- `Skin` is the compute skinning pre-pass. `RenderItem::mSkinnedVertices` only exists afterwards,
  which is why the scene collects draws after skinning.
- Components never call the renderer. They append `RenderItem`s during the read-only collect sweep;
  `SubmitRenderQueue` walks `Scene::ForEachRenderBatch` and issues one draw per batch (instanced when
  the batch holds more than one item).

## 4. Binding model: names, not slots

The engine never uses free-floating uniforms, even on OpenGL where they are legal. Every engine input
is a uniform block, storage block or sampler **matched by the CRC32 of its GLSL name**, so one shader
source serves both a set/binding API and a flat-binding one.

[`Mesh::BindingLocations`](../../include/aeongames/Mesh.hpp) is the registry of reserved names:

| Group | Names |
| --- | --- |
| Per-frame / per-draw | `Matrices`, `Material`, `Samplers`, `Lights`, `Globals`, `Bindless` |
| Clustered lighting | `ClusterParams`, `ClusterAABBs`, `LightGrid`, `LightIndexList`, `LightIndexCounter`, `ClusterActive` |
| Shadows | `ShadowParams`/`ShadowMap`, `SpotShadowParams`/`SpotShadowMap`, `PointShadowParams`/`PointShadowMap` |
| Compute skinning | `SkinningMatrices`, `SourceVertices`, `SkinnedVertices` |
| Instancing / GPU-driven | `InstanceMatrices`, `InstanceMaterials`, `CullInstances`, `DrawCommands`, `DrawCount`, `HiZ` |
| Image-based lighting | `PrefilteredEnvironment` |

Vertex attributes follow the same scheme through `Mesh::AttributeSemantic` (`VertexPosition`,
`VertexNormal`, `VertexUV`, `VertexWeights`, …).

At pipeline load each backend reflects its shaders into a *hash → slot* map:

- **Vulkan** — SPIRV-Reflect yields *hash → descriptor set index*; draw code asks
  `GetDescriptorSetIndex(hash)` and binds at whatever index the shader declared. A pipeline layout may
  declare at most **16** sets (an engine-chosen array in `VulkanPipeline.cpp`); gaps are filled with a
  shared empty layout because `pSetLayouts` is dense.
- **OpenGL** — `glGetProgramInterfaceiv` / `glGetProgramResource*` on the linked program yield
  *hash → uniform-block binding, storage-block binding or texture unit*.

Adding a new engine-wide block means: add the name to `Mesh::BindingLocations`, declare it in the
shader with both `#ifdef VULKAN` branches, and reflect + bind it in **both** backends.

## 5. Shaders and per-renderer variants

Shader sources live in [../../assets/shadercode](../../assets/shadercode) and are packed into
`game/shaders/*.txt` / `*.pln` by the `shader-pipelines` target; those outputs are generated and never
hand-edited. See
[.github/instructions/shaders.instructions.md](../../.github/instructions/shaders.instructions.md).

A backend resolves source per stage with its own name:

```cpp
std::string_view vert = aPipeline.GetShaderCode ( ShaderType::VERT, GetName() );
for ( uint32_t i = 0; i < aPipeline.GetComputeStageCount ( GetName() ); ++i )
{
    std::string_view comp = aPipeline.GetComputeShaderCode ( i, GetName() );
}
```

A stage may carry per-renderer variants: a renderer-specific entry overrides the default and an
**empty** entry disables the stage for that renderer. The `PIPELINES` entry format in
[assets/shadercode/CMakeLists.txt](../../assets/shadercode/CMakeLists.txt) expresses both —
`point_shadow_depth` uses a multiview vertex shader with no geometry stage on Vulkan and a geometry
shader on OpenGL:

```cmake
"point_shadow_depth|vert=point_shadow_depth.vert|geom=point_shadow_depth.geom|frag=point_shadow_depth.frag|Vulkan:vert=point_shadow_depth_mv.vert|Vulkan:geom"
```

Compute stages are **ordered**; index 0 is dispatched first (`lighting.0.comp` builds the clusters,
`lighting.1.comp` culls the lights).

## 6. Runtime settings

`ConstructRenderer` (and the `Window` constructor) pass a `RendererSettings` built by
`GetRendererSettings(name)` from the `Renderer` block of the configuration file
([configuration.proto](../../proto/configuration.proto)). The backend-agnostic fields — bindless
capacities, uniform/storage pool sizes, shadow map resolutions, prefiltered environment size — are
honoured by every backend, and an omitted field keeps the compiled-in default.

Knobs only one backend needs go in the per-plugin property bag rather than the shared struct:

```cpp
size_t capacity = GetSettings().GetPluginProperty ( "UniformPoolInitialCapacity"_crc32, 64 * 1024 );
```

The configuration addresses those generically by plugin name plus property name, so the engine never
hardcodes a backend-specific key. Take the settings in the constructor
(`MyRenderer ( void* aWindow, const RendererSettings& aSettings = {} )`) and expose them through
`GetSettings()`.

## 7. Adding a backend

1. Create `engine/renderers/<api>/` with a `CMakeLists.txt` producing a `SHARED` target, append it to
   the global `PLUGINS` property, and gate it behind `BUILD_<API>_RENDERER`.
2. Write `Plugin.cpp` (§2) and add the target name to [game/config](../../game/config).
3. Implement the required interface (§3). Start with resource upload, `AttachWindow`,
   `BeginFrame`/`BeginRenderPass`/`EndRender` and `Render`; leave every optional override at its
   default so the backend runs without shadows, skybox or IBL.
4. Implement reflection into the *hash → slot* map (§4) before writing any bind code.
5. Add the per-frame linear uniform/storage pools backing `AllocateSingleFrame*Memory`.
6. Layer on compute: `Dispatch`, `Barrier`, `Skin`, then `BeginRender`'s cluster build and
   `EndDepthPrePass`'s light cull.
7. Add shadows, then HDR + tone map, then image-based lighting.
8. Override `RenderInstanced` last — the base loop is already correct.

## 8. Testing and debugging

- Build and test from the MSYS2 MinGW shell: `cd mingw64 && make -j$(nproc) && ctest --output-on-failure`.
  The single gtest binary runs with the **repo root** as its working directory.
- Run from the repo root:
  `PATH="$PWD/mingw64/bin:$PATH" ./mingw64/bin/game.exe -r OpenGL -s scenes/main.txt`
  (`-r Vulkan` for the other backend). Renderer names are exactly `OpenGL` and `Vulkan`.
- Pipeline load logs every reflected uniform/storage block binding and sampler unit at
  `LogLevel::Debug` — the fastest way to verify a new binding.
- Deterministic frame capture, immune to window occlusion:
  `AEON_GL_SCREENSHOT_FRAME=<n> AEON_GL_SCREENSHOT_DIR=<dir>` on OpenGL,
  `VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_screenshot VK_SCREENSHOT_FRAMES=<n>` on Vulkan.
- Per-pass GPU timings: `AEON_BENCH_FRAMES=<n> AEON_BENCH_WARMUP=<n>` (requires
  `RecordGpuTimestamp` / `ReadGpuTimestamps`).
- Cross-check by rendering the same scene on both backends; a divergence is almost always a wrong
  binding or a missing `#ifdef VULKAN` branch.

## 9. Pitfalls that have bitten before

| Pitfall | Detail |
| --- | --- |
| GL sampler binding ≠ uniform location | `OpenGLPipeline::GetSamplerLocation` returns the **texture unit** from `layout(binding = N)`, not a uniform location. Mixing them up silently renders black. |
| Missing GL entry point | A GL function absent from [opengl/glFunctions.txt](opengl/glFunctions.txt) compiles fine and is a null pointer at runtime. Add the name there first. |
| `std140` UBOs on OpenGL/NVIDIA | The driver's GLSL front-end mis-fetches members past the first, so the `Globals` block is an `std430` SSBO on the OpenGL branch while Vulkan keeps the UBO. Do not unify them. A full GL-SPIR-V migration was investigated and **shelved**: glslang rejects `GL_ARB_bindless_texture` when targeting SPIR-V. |
| Vulkan descriptor set count | At most 16 sets per pipeline layout; sparse indices are allowed, but gaps must be filled with the shared empty layout. |
| Unchecked bindless material index | An out-of-range index dereferences a wild GPU address and causes `VK_ERROR_DEVICE_LOST`. Bounds-check against `MATERIAL_CAPACITY` in the shader. |
| Frames in flight | `BeginFrame` does not imply the previous frame finished. Call `Finish` before reading back GPU-written buffers, capturing the framebuffer or tearing resources down. |
| Recursion | Engine-wide rule: no recursive functions. Traverse iteratively with an explicit, preferably fixed-size, stack. |
