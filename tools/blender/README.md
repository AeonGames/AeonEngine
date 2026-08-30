# Blender Asset Pipeline

## Headless asset cooking

The `.blend` files under `assets/` are cooked by opt-in CMake targets. They are
never part of the default build:

| Target | Mode | Output |
| --- | --- | --- |
| `aerin` | Model | `game/aerin/` |
| `backdrop` | Model | `game/backdrop/` |
| `polesign` | Model | `game/polesign/` |
| `sponza` | Scene | `game/scenes/sponza.scn` plus models and shared resources under `game/sponza/` |

Build one target with the configured generator:

```bash
# Windows/MSYS2
cd mingw64 && make aerin

# macOS/Ninja
cmake --build clang64 --target sponza
```

Each target runs Blender headless through [export_asset.py](export_asset.py).
Model mode drives `io_model_mdl` and writes one model plus its meshes, skeleton,
animations, materials, and textures. Scene mode drives `io_scene_scn`, writes a
scene, and exports one model per unique mesh datablock; objects sharing that
datablock become instances of the same model.

Packed images are exported alongside loose textures. Loose textures named in
the CMake target are copied after export so sources under `assets/` remain
authoritative. Cooking is stamped under `<build>/blender-assets` and reruns when
the `.blend`, a named texture, or an exporter changes.

CMake chooses the newest installed Blender. Pass
`-DBLENDER_EXECUTABLE=<path>` to select one explicitly. When Blender or its
bundled Python cannot be found, configuration succeeds and omits the asset
targets.

## Blender Python environment

The exporters use generated Python protobuf modules, but Blender does not ship
the matching protobuf runtime. Blender's extension manager also needs `cattrs`.
The `blender-python-venv` target creates `<build>/blender-venv` with Blender's
own interpreter and installs both dependencies. Every cook target depends on
this environment and `generate-python-protobuf-source`.

Cook targets expose the venv during Blender startup automatically. For
interactive Blender sessions, expose its site-packages and allow Blender to use
the environment before launch:

```bash
# Windows layout
export PYTHONPATH="<build>/blender-venv/Lib/site-packages"

# macOS/Linux layout
export PYTHONPATH="<build>/blender-venv/lib/pythonX.Y/site-packages"

blender --python-use-system-env
```

## Interactive addons

The primary AeonEngine exporters are:

| Addon | Purpose |
| --- | --- |
| `io_scene_scn` | Export a complete `.scn`, one model per unique mesh, instances, markers, lights, cameras, and the world environment map |
| `io_model_mdl` | Export a `.mdl` and orchestrate mesh, material, skeleton, animation, and texture output |
| `io_mesh_msh` | Export mesh attributes and optional per-material-slot geometry |
| `io_material_mtl` | Convert Principled BSDF inputs to the engine's Phong material and samplers |
| `io_skeleton_skl` | Export an armature as `.skl` |
| `io_animation_anm` | Export Blender actions as `.anm` clips |
| `io_collision_cln` | Export collision geometry as `.cln` |
| `io_images` | Export all images from the current file |

The addon packager also includes the lossless bone-connect utility, the Unreal
PSK/PSA importer, and the duplicate-mesh detector. Build all zip packages with:

```bash
cmake --build <build> --target package_blender_addons
```

Packages are written to `<build>/blender_addons/`. To run directly from the
source tree instead:

1. Build `generate-python-protobuf-source` and `blender-python-venv`.
2. In Blender, open Preferences > File Paths > Script Directories.
3. Add this `tools/blender` directory, save preferences, and restart Blender.
4. Enable the required addons under Preferences > Add-ons.
5. Use File > Export for the AeonEngine formats.

`io_scene_scn` requires `io_model_mdl` and its lower-level exporters. It writes
assets under `<game root>/<scene name>/` and the scene itself under
`<game root>/scenes/`. Hidden objects are skipped; duplicate mesh datablocks are
instanced; Blender sun, spot, and point lights become engine light components;
cameras retain field of view and clipping planes; empties can become marker
components; and a world environment image becomes the scene environment map.

AeonEngine formats are export-only. Keep the original `.blend` as the editable
source for future changes.
