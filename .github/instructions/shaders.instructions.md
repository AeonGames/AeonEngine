---
description: "Authoring GLSL for AeonEngine: dual Vulkan/OpenGL #ifdef branches, set/binding layout rules, and regenerating game/shaders via the shader-pipelines target."
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
3. Rebuild: `cd mingw64 && make shader-pipelines` (packed by `aeontool pipeline`).
4. Verify on **both** backends — `game.exe -r OpenGL …` and `game.exe -r Vulkan …`.

## Dual-backend source

One source file serves both backends; `VULKAN` is defined only for the Vulkan variant.

```glsl
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices { mat4 ProjectionMatrix; mat4 ViewMatrix; };
```

- Vulkan addresses resources by `set` + `binding`; OpenGL by a **flat global binding number** per
  resource class. The OpenGL binding is *not* the Vulkan set index — check the existing
  assignments in [static_mesh.vert](../../assets/shadercode/static_mesh.vert) and
  [clustered_phong.frag](../../assets/shadercode/clustered_phong.frag) before claiming a slot, and
  keep the C++ side in both renderers in sync.
- **A pipeline may declare at most 8 descriptor sets** — MoltenVK's hard limit, and not
  configurable (Metal argument buffers do not raise it). The count is `max(set index) + 1` across
  *all* stages of the pipeline, since gaps are filled with empty layouts, so keep set indices dense
  and low. When a pipeline needs more engine resources than that, **pack** several into one set
  under different `binding` numbers; the engine identifies a packed set by its **binding 0** and
  binds the whole set at once (see sets 0 and 1 of `clustered_phong.frag`, bound by
  `VulkanWindow::BindShadingPassSets`). Packing also needs a matching aggregate descriptor set in
  `VulkanWindow::InitializePackedShadingSets`, so it is not a shader-only change.
- Backend-specific access goes behind a macro (`MODEL_MATRIX`, `MAT_REC`, `MAT_TEX(i)`) so `main()`
  stays backend-agnostic.
- Instancing: Vulkan folds the base instance into `gl_InstanceIndex`; OpenGL needs
  `GL_ARB_shader_draw_parameters` and `gl_BaseInstanceARB + gl_InstanceID`.
- Bindless materials: Vulkan uses a `buffer_reference` (BDA) push constant plus
  `nonuniformEXT` indexing into `global_textures[]`; OpenGL uses `GL_ARB_bindless_texture` resident
  handles. Always bounds-check the material index (`MATERIAL_CAPACITY`) before the BDA read — an
  out-of-range index dereferences a wild GPU address and causes `VK_ERROR_DEVICE_LOST`.

## Gotchas

- A `layout(binding = N) uniform sampler2D` on OpenGL binds **texture unit N**; the C++ reflection
  must store the unit, not the uniform location.
- The OpenGL `Globals` block is an `std430` SSBO on purpose (NVIDIA GLSL front-end mis-fetches
  `std140` UBO members past the first). Vulkan keeps the `std140` UBO. Do not unify them.
- Explicit `location` qualifiers are required on all vertex attributes, varyings and fragment
  outputs; the two backends match by location, not by name.
