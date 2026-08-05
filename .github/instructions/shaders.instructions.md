---
description: "Authoring GLSL for AeonEngine: Vulkan/OpenGL/Metal branches, descriptor and argument-buffer binding rules, and regenerating game/shaders via the shader-pipelines target."
applyTo: "assets/shadercode/**"
---

# AeonEngine shader authoring

Sources here are the **only** hand-edited shader files. `game/shaders/*.txt` and `*.pln` are
generated artifacts (gitignored, absent on a fresh checkout) — never edit them.

## Workflow

1. Edit or add a `.vert` / `.frag` / `.comp` / `.geom` / `.tesc` / `.tese` here.
2. Register the pipeline in the `PIPELINES` list of
   [assets/shadercode/CMakeLists.txt](../../assets/shadercode/CMakeLists.txt). Entry format:
   `"<name>|<stage>=<file>|…"`, plus optional `topology=<TRIANGLE|LINE>`,
   `<Renderer>:<stage>=<file>` for a per-renderer variant, and `<Renderer>:<stage>` to disable a
   stage for that renderer.
3. Rebuild: `cd mingw64 && make shader-pipelines` on MSYS2, or
  `cmake --build clang64 --target shader-pipelines` on macOS.
4. Verify every built backend: `game -r OpenGL …`, `game -r Vulkan …`, and on Apple Silicon
  `game -r Metal …`. Run Metal-focused tests with `MTL_DEBUG_LAYER=1` and
  `MTL_SHADER_VALIDATION=1`.

## Shared backend source

One GLSL source normally serves all backends. The Metal cooker targets Vulkan SPIR-V, retaining
Vulkan-style descriptor declarations, and additionally defines `METAL` for Metal-specific ABI or
math branches. The plain fallback path is OpenGL.

```glsl
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices { mat4 ProjectionMatrix; mat4 ViewMatrix; };
```

- Vulkan and Metal author resources using `set` + `binding`; OpenGL uses a **flat global binding
  number** per resource class. The OpenGL binding is *not* the Vulkan set index — check the existing
  assignments in [static_mesh.vert](../../assets/shadercode/static_mesh.vert) and
  [clustered_phong.frag](../../assets/shadercode/clustered_phong.frag) before claiming a slot, and
  keep every applicable renderer in sync.
- On Metal, the descriptor set becomes the MSL argument-buffer index. SPIRV-Cross may compact the
  resources inside that set, so generated Metal `ShaderInterface.resource.binding` values are the
  runtime MSL argument IDs and may differ from GLSL `binding`. Never infer Metal argument IDs from
  GLSL or parse generated MSL at runtime.
- A Metal combined image sampler occupies separate texture and sampler argument IDs. Runtime-sized
  texture arrays (the set-2 `global_textures` bindless table) require a device argument buffer;
  regular material records remain in set 7.
- Keep shared pipelines within **8 descriptor sets** (`max(set index) + 1` across every stage).
  Metal's current argument-buffer binder indexes sets 0 through 7, and MoltenVK deployments may
  impose the same practical ceiling even though AeonEngine's native Vulkan layout array supports
  up to 16. Keep set indices dense and low. When a pipeline needs more engine resources, **pack**
  several into one set under different `binding` numbers; the engine identifies a packed set by
  its **binding 0** and binds the whole set at once (see sets 0 and 1 of `clustered_phong.frag`,
  bound by `VulkanWindow::BindShadingPassSets`). Packing also needs a matching aggregate descriptor
  set in `VulkanWindow::InitializePackedShadingSets`, so it is not a shader-only change.
- Backend-specific access goes behind a macro (`MODEL_MATRIX`, `MAT_REC`, `MAT_TEX(i)`) so `main()`
  stays backend-agnostic.
- Instancing: Vulkan folds the base instance into `gl_InstanceIndex`; OpenGL needs
  `GL_ARB_shader_draw_parameters` and `gl_BaseInstanceARB + gl_InstanceID`; generated MSL uses
  Metal's instance/base-instance built-ins.
- Bindless materials: Vulkan uses a `buffer_reference` (BDA) push constant plus
  `nonuniformEXT` indexing into `global_textures[]`; OpenGL uses `GL_ARB_bindless_texture` resident
  handles; Metal uses a fixed-size combined texture/sampler array in set 2 and a regular material
  storage buffer in set 7. Always bounds-check the material index (`MATERIAL_CAPACITY`) before any
  unchecked read — Vulkan can otherwise fault a BDA address and Metal can read past its buffer.

## Gotchas

- A `layout(binding = N) uniform sampler2D` on OpenGL binds **texture unit N**; the C++ reflection
  must store the unit, not the uniform location.
- The OpenGL `Globals` block is an `std430` SSBO on purpose (NVIDIA GLSL front-end mis-fetches
  `std140` UBO members past the first). Vulkan keeps the `std140` UBO. Do not unify them.
- Explicit `location` qualifiers are required on all vertex attributes, varyings and fragment
  outputs; all backends match by location, not by name.
- Metal uses `[0,1]` clip/window depth like Vulkan. For math-only conditionals such as Hi-Z depth
  comparisons, use `#if defined(VULKAN) || defined(METAL)` where appropriate; do not accidentally
  send Metal through OpenGL's `[-1,1]` depth conversion. Resource declaration branches may still
  use `#ifdef VULKAN` because the Metal cooker retains Vulkan-style descriptors in its SPIR-V.
- Metal MSL is generated and compiled at build time by `metal-shader-tool` using
  `glslangValidator`, SPIRV-Cross, and Apple's downloaded Metal compiler. Do not hand-edit the MSL
  embedded in `game/shaders/*.txt`; fix GLSL or the cooker, regenerate, and verify the reflected
  interface metadata.
