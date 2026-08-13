Cooking assets from the build
=============================

The `.blend` files under `assets/` are cooked into engine assets by opt-in CMake
targets, one per asset:

    make aerin
    make backdrop
    make polesign

Each runs Blender headless through `export_asset.py`, which drives the
`io_model_mdl` exporter and writes a binary model plus its meshes, skeleton,
animations, materials and textures into the matching `game/` directory. Textures
packed inside a `.blend` are written out alongside the ones that live loose on
disk, so materials resolve either way.

CMake picks the newest installed Blender; pass `-DBLENDER_EXECUTABLE=<path>` to
choose a different one. When no Blender is found the configure step only prints
a message and skips these targets.

The exporters need a protobuf runtime that Blender does not bundle. Instead of
modifying the Blender installation, the `blender-python-venv` target builds a
virtual environment out of Blender's own interpreter under `<build>/blender-venv`
and installs the exact runtime version the generated `*_pb2.py` modules were
compiled for. The asset targets depend on it, so there is nothing to install by
hand.

Using the exporters interactively
=================================

1. Build the `generate-python-protobuf-source` and `blender-python-venv` targets.
2. Run Blender and open Preferences -&gt; File Paths -&gt; Script Directories.
3. Add this folder.
4. Save preferences and restart Blender.
5. The exporters are now listed as Import-Export add-ons on the Add-ons tab; check
   the box next to the ones you need, they should report no errors during loading.
6. Export from File -&gt; Export.

Blender's bundled Python still has to be able to import `google.protobuf` for the
interactive path. Add `<build>/blender-venv/Lib/site-packages` (or
`lib/python*/site-packages` outside Windows) to `PYTHONPATH` before launching
Blender, or install protobuf into the Blender installation yourself.

There is only export functionality for the time being, so avoid lossing the original blend file if you want to make changes to your model down the road.
