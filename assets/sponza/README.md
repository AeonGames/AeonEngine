# Sponza Scene - Modified for Real-Time Rendering

This folder contains a modified version of the **Sponza 2022** scene originally created by Frank Meinl and sponsored by Anton Kaplanyan.

## Original Source

The original Sponza scene can be found at:

- **Intel Graphics Research Samples**: <https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html>
- **Direct Download**: <https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-processing-research/samples.html>

## Modifications

This version has been modified for game-compatible real-time rendering:

- **Model Optimization**: The original model was imported into Blender and simplified to reduce polygon count and optimize geometry for real-time rendering performance
- **Texture Reduction**: Surface textures have been downscaled to 512×512 to reduce memory use; the authored Radiance environment remains 4096×2048 for image-based lighting
- **Scene Authoring**: The Blender source includes engine-ready instancing, lights, cameras, and an HDR world environment
- **Format Conversion**: The opt-in Blender cook converts the source scene to AeonEngine scene, model, mesh, material, and texture resources

### Modifications By

**Rodrigo Jose Hernandez Cordoba** - All game-compatible modifications, optimization, and asset conversion (2025-2026)

## Cooking and Running

[sponza.blend](sponza.blend) and the files under `textures/` are source assets.
Generated runtime files under `game/sponza/` and `game/scenes/sponza.scn` should
not be edited by hand.

Cook the scene from a configured build tree:

```bash
cmake --build <build> --target sponza
```

The target is opt-in and stamped under `<build>/blender-assets`. Scene mode
exports one model per unique mesh datablock, preserves duplicate objects as
instances, creates light and camera components, selects the clustered-lighting
pipeline, and copies the world HDR image as the environment map. See the
[Blender asset pipeline](../../tools/blender/README.md) for setup details.

Run the cooked scene from the repository root:

```bash
PATH="$PWD/<build>/bin:$PATH" ./<build>/bin/game -r Vulkan -s scenes/sponza.scn
```

## License

The original **Creative Commons Attribution 4.0 International License** (CC BY 4.0) from the source material remains in effect for this modified version. See [credits_license.txt](credits_license.txt) for the complete original license and attribution requirements.

### Citation

If using this scene in a publication, please include the original citation:

```bibtex
@misc{sponza22,
  Author = {Frank Meinl and Anton Kaplanyan},
  Year = {2022},
  Note = {https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-processing-research/samples.html},
  Title = {Intel Sample Library}
}
```

## Original Credits

- **Scene Commissioned By**: Frank Meinl
- **Sponsored By**: Anton Kaplanyan
- **Sponza Addon Package Crew**: Katica Putica, Cristiano Siqueira, Timothy Heath, Justin Prazen, Sebastian Herholz, Bruce Cherniak, Anton Kaplanyan
- **Reference Photos**: Katica Putica, Princino.photo (Dubrovnik, Croatia)

For complete credits and license details, see [credits_license.txt](credits_license.txt).
