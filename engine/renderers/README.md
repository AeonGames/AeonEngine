# AeonEngine Renderer Backends

Everything under `engine/renderers/` is a **self-contained shared-library plugin** implementing the
[`Renderer`](../../include/aeongames/Renderer.hpp) interface. Three backends ship today:

| Directory | CMake target | Registered name | Notes |
| --- | --- | --- | --- |
| [opengl](opengl) | `OpenGLRenderer` | `"OpenGL"` | OpenGL 4.5 core + `GL_ARB_bindless_texture`; entry points loaded from [glFunctions.txt](opengl/glFunctions.txt) |
| [vulkan](vulkan) | `VulkanRenderer` | `"Vulkan"` | Vulkan 1.x; GLSL→SPIR-V through glslang ([SPIR-V/](vulkan/SPIR-V)), reflection through SPIRV-Reflect, MoltenVK on macOS |
| [metal](metal) | `MetalRenderer` | `"Metal"` | Native Metal 3 on Apple Silicon/macOS 13+; argument-buffer tier 2; generated MSL and interface metadata from `metal-shader-tool` |

For a frame-by-frame walkthrough of the rendering path — clustered forward shading, shadow passes,
HDR + tone map, image-based lighting, compute skinning — read
[vulkan/RENDER_PIPELINE.md](vulkan/RENDER_PIPELINE.md). The passes it describes are shared by both
the OpenGL and Metal backends, while resource ownership and command recording remain API-specific.

> A backend-neutral feature almost always needs Vulkan, OpenGL, and Metal implementations plus any
> matching shader branches or renderer-scoped variants **in the same change**. See
> [AGENTS.md](../../AGENTS.md).

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
5. `SubmitRenderQueue(DepthPrePass)` → `EndDepthPrePass` — retain depth for early-Z/Hi-Z,
  light-cull every cluster, then reopen the color pass;
6. `SubmitRenderQueue(Shading)`;
7. `SubmitDebugGeometry` when debug rendering is enabled;
8. `RenderOverlay` when a GUI overlay was passed;
9. `EndRender`.

Everything that precedes the call — `BeginFrame`, `SetViewMatrix` / `SetProjectionMatrix`,
`SetLights`, `SetGlobals`, `SetEnvironmentMap` and the `Skin` compute pre-pass — stays in the
application loop, because compute work must be recorded before the first render pass opens.

## 2. Plugin anatomy

A backend is a `SHARED` library exporting a single `PluginModuleInterface PMI` (see
[Plugin.hpp](../../include/aeongames/Plugin.hpp)). Follow the concrete
[Vulkan](vulkan/Plugin.cpp), [OpenGL](opengl/Plugin.cpp), or [Metal](metal/Plugin.cpp) entry point:
`StartUp` returns the result of registration, and `Shutdown` unregisters the same name.

```cpp
AeonGames::RegisterRendererConstructor ( "MyAPI", constructor );
AeonGames::UnregisterRendererConstructor ( "MyAPI" );
```

- Registration is keyed by `StringId` (CRC32), and `Renderer::GetName()` **must return the same
  string** — it is what selects per-renderer shader variants at pipeline load.
- Register one settings-aware constructor with the callback signature
  `std::unique_ptr<Renderer> ( void*, const RendererSettings& )`. Every `ConstructRenderer` call
  forwards its caller-supplied settings to that callback.
- CMake: `add_library(<Target> SHARED …)`, then
  `set_property(GLOBAL APPEND PROPERTY PLUGINS <Target>)`, plus `PREFIX ""` under MinGW/MSYS so the
  DLL name matches the config entry. Gate the `add_subdirectory` behind a `BUILD_<API>_RENDERER`
  option in [../CMakeLists.txt](../CMakeLists.txt).
- CMake regenerates [game/config](../../game/config) from the enabled targets in the global `PLUGINS`
  property. The plugin is loaded from its `Plugin: "<TargetName>"` entry and selected with
  `game.exe -r <RegisteredName>` (or `game -r <RegisteredName>` on macOS).
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
- OpenGL GPU culling writes commands to stable input-order slots by default. Atomic compaction makes
  coincident Sponza surfaces change draw order under `LEQUAL`, producing visible flicker; enable it
  only for profiling with `AEON_GL_COMPACT_DRAWS=1`.
- NVIDIA OpenGL defaults Hi-Z occlusion off because its current footprint query can falsely reject
  visible material-split draws at foreground depth discontinuities. Other OpenGL vendors default it
  on; `AEON_HIZ_OCCLUSION=0/1` overrides either policy. GPU frustum culling remains enabled, and
  Vulkan and Metal keep Hi-Z enabled by default.

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
- **Metal** — each Vulkan-style descriptor set becomes an MSL argument-buffer index. The shader
  cooker records SPIRV-Cross's compacted argument IDs in Metal-only `ShaderInterface` metadata;
  runtime code resolves those IDs by resource name/hash instead of assuming the original GLSL
  binding. Vertex data uses Metal buffer index 8.

Adding a new engine-wide block means: add the name to `Mesh::BindingLocations`, declare it in the
shared shader ABI, and reflect + bind it in **all three** backends.

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
`point_shadow_depth` uses a multiview vertex shader with no geometry stage on Vulkan/Metal and a
geometry shader on OpenGL:

```cmake
"point_shadow_depth|vert=point_shadow_depth.vert|geom=point_shadow_depth.geom|frag=point_shadow_depth.frag|Vulkan,Metal:vert=point_shadow_depth_mv.vert|Vulkan,Metal:geom"
```

Compute stages are **ordered**; index 0 is dispatched first (`lighting.0.comp` builds the clusters,
`lighting.1.comp` culls the lights).

## 6. Runtime settings

`ConstructRenderer` requires a platform-native window handle and explicit `RendererSettings`. The
registered plugin must already be loaded; an unknown renderer name returns `nullptr`. To use the
compiled defaults, value-initialize the settings:

```cpp
RendererSettings settings{};
std::unique_ptr<Renderer> renderer = ConstructRenderer ( renderer_name, native_window, settings );
```

To apply the `Renderer` block from the configuration file
([configuration.proto](../../proto/configuration.proto)), build the settings explicitly and pass
them into the same factory:

```cpp
RendererSettings settings = GetRendererSettings ( renderer_name );
std::unique_ptr<Renderer> renderer = ConstructRenderer ( renderer_name, native_window, settings );
```

The application follows the latter path: it passes `GetRendererSettings(name)` to the `Window`
constructor, which forwards the settings to `ConstructRenderer`. The backend-agnostic fields —
bindless capacities, uniform/storage pool sizes, shadow map resolutions, prefiltered environment
size — are honoured by every backend, and a field omitted from the configuration keeps its
compiled-in default.

Knobs only one backend needs go in the per-plugin property bag rather than the shared struct:

```cpp
size_t capacity = GetSettings().GetPluginProperty ( "UniformPoolInitialCapacity"_crc32, 64 * 1024 );
```

The configuration addresses those generically by plugin name plus property name, so the engine never
hardcodes a backend-specific key. Take the settings in the constructor
(`MyRenderer ( void* aWindow, const RendererSettings& aSettings )`) and expose them through
`GetSettings()`.

## 7. Native Metal architecture

Metal follows the same public `Renderer` contract and `RenderScene` sequence, but its ownership and
synchronization deliberately follow Metal rather than imitating Vulkan:

- `MetalRenderer` owns the device, command queue, resource caches, bindless tables and
  renderer-owned pipelines. Each `MetalWindow` owns its `CAMetalLayer`, triple-buffered frame pools,
  attachments, shadow maps, command buffers/encoders, Hi-Z pyramid and environment cubes.
- Descriptor set number maps to MSL argument-buffer index. Bindless texture/sampler pairs live in
  set 2, material records in set 7, and clustered per-frame blocks are packed into set 0. Consume
  generated `ShaderInterface` entries by resource hash: SPIRV-Cross may compact the argument IDs,
  and combined samplers consume both texture and sampler IDs.
- Render and compute work share one command buffer. Ending an encoder before compute and opening the
  next encoder establishes ordering, so `MetalRenderer::Barrier` is intentionally a no-op.
- GUI uploads are premultiplied `BGRA8Unorm` and blend with source factor `ONE`. Treating the overlay
  as straight RGBA obscures or discolours the scene beneath a transparent canvas.

The Metal shader path is enabled by `BUILD_METAL_RENDERER`: `aeontool` first packs the shared GLSL,
then `metal-shader-tool` invokes glslang/SPIRV-Cross and Apple's Metal compiler to append MSL plus
validated interface metadata to each generated pipeline.

## 8. Adding a backend

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

## 9. Testing and debugging

- Build and test from the MSYS2 MinGW shell: `cd mingw64 && make -j$(nproc) && ctest --output-on-failure`.
  The single gtest binary runs with the **repo root** as its working directory.
- Run from the repo root:
  `PATH="$PWD/mingw64/bin:$PATH" ./mingw64/bin/game.exe -r OpenGL -s scenes/main.txt`
  (`-r Vulkan` for Vulkan). On Apple Silicon:
  `PATH="$PWD/clang64/bin:$PATH" ./clang64/bin/game -r Metal -s scenes/main.txt`.
  Renderer names are exactly `OpenGL`, `Vulkan`, and `Metal` where those plugins are built.
- Pipeline load logs every reflected uniform/storage block binding and sampler unit at
  `LogLevel::Debug` — the fastest way to verify a new binding.
- Deterministic frame capture, immune to window occlusion:
  `AEON_GL_SCREENSHOT_FRAME=<n> AEON_GL_SCREENSHOT_DIR=<dir>` on OpenGL,
  `VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_screenshot VK_SCREENSHOT_FRAMES=<n>` on Vulkan.
- Validate Metal work with
  `MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1 MTL_SHADER_VALIDATION_REPORT_TO_STDERR=1`.
- Per-pass GPU timings: `AEON_BENCH_FRAMES=<n> AEON_BENCH_WARMUP=<n>` (requires
  `RecordGpuTimestamp` / `ReadGpuTimestamps`).
- Cross-check by rendering the same scene on every built backend; a divergence is commonly a wrong
  binding, stale generated interface metadata, or a missing renderer-scoped shader branch.

## 10. Pitfalls that have bitten before

| Pitfall | Detail |
| --- | --- |
| GL sampler binding ≠ uniform location | `OpenGLPipeline::GetSamplerLocation` returns the **texture unit** from `layout(binding = N)`, not a uniform location. Mixing them up silently renders black. |
| Missing GL entry point | A GL function absent from [opengl/glFunctions.txt](opengl/glFunctions.txt) compiles fine and is a null pointer at runtime. Add the name there first. |
| `std140` UBOs on OpenGL/NVIDIA | The driver's GLSL front-end mis-fetches members past the first, so the `Globals` block is an `std430` SSBO on the OpenGL branch while Vulkan keeps the UBO. Do not unify them. A full GL-SPIR-V migration was investigated and **shelved**: glslang rejects `GL_ARB_bindless_texture` when targeting SPIR-V. |
| Vulkan descriptor set count | At most 16 sets per pipeline layout; sparse indices are allowed, but gaps must be filled with the shared empty layout. |
| Unchecked bindless material index | An out-of-range index dereferences a wild GPU address and causes `VK_ERROR_DEVICE_LOST`. Bounds-check against `MATERIAL_CAPACITY` in the shader. |
| Metal argument IDs | SPIRV-Cross compacts resources within an argument buffer. Use generated interface metadata; raw GLSL `binding` values are not necessarily runtime argument IDs. |
| Metal runtime SSBO sizes | Runtime-sized buffers can add hidden `spvBufferSizeConstants`; populate them from Metal reflection or shader reads can run out of bounds. |
| Metal drawable/overlay formats | The drawable and final tone-map target are linear `BGRA8Unorm` because the shader performs sRGB transfer. GUI pixels are premultiplied BGRA8 and require source factor `ONE`. |
| Frames in flight | `BeginFrame` does not imply the previous frame finished. Call `Finish` before reading back GPU-written buffers, capturing the framebuffer or tearing resources down. |
| Recursion | Engine-wide rule: no recursive functions. Traverse iteratively with an explicit, preferably fixed-size, stack. |
