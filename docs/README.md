# Developer Blog

This is a page to gather thoughs regarding development before they are implemented.

Feel free to contact me via [Twitter](https://twitter.com/AeonKwizatz) if you have any comments in regard to what is posted here.

## Bindless Textures functionality removed
### 2/14/2017

I've removed the use of BTs from the engine. I am not sure wether or not they actually were an improvement on performance. TBH originaly I just used them because they allowed me to stick sampler variables into uniform blocks and not having to use a second code path specific to samplers.
As it was, I already had to take special path for samplers (image loading), so, in the end the change was not too drastic, and now I can probably generate SPIR-V binaries from the generated GLSL...
By the way, I am not too happy about having the "Graphics API" independent code generate GLSL now,
Vulkan doesn't natively compile it, and since in theory you can compile SPIR-V from any language, I can see a new more generic language emerging that better fits SPIR-V, the fact that OpenGL 3.3 [supports SPIR-V](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_gl_spirv.txt) doesn't help, but SPIR-V is varely human readable and I suppose much less human "assemblable".

## Vulkan
### 2/1/2017

I've been trying to get the Vulkan renderer working, however the way I envisioned modularity to be in this engine is very close to what you'd expect when writting OpenGL code only.
I was expecting Vulkan to be somewhat similar to OpenGL, and I seem to have made a mistake there. In some ways it is, and in some others it isn't.
There'll be time to ramble on some generalities, but the issue I am tackling right now is that of shader modules.
Vulkan doesn't have a runtime GLSL compiler and everyone seems to just assume you'll be running glslang validator to turn GLSL into SPIR-V as a preprocessing step.
The AeonEngine up to this point in time does not assume that. Linking against the glslang library for runtime compilation is something I have considered, but that doesn't seem as simple as it sounds, if it was, the net would be full of examples by now.
Anyway, the most urging issue I have right now, if I am to keep renderer modularity, is to sadly remove support for [bindless textures](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_bindless_texture.txt), which at the time I implemented it solved a problem with how I implemented shader properties as UBOs, but Vulkan doesn't support them, so, off they go!.
