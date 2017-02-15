---
layout: post
title:  "Bindless Textures functionality removed"
date:   2017-02-14 17:17:16 -0600
categories: implementation
---

## Bindless Textures functionality removed
### 2/14/2017

I've removed the use of BTs from the engine. I am not sure wether or not they actually were an improvement on performance. TBH originaly I just used them because they allowed me to stick sampler variables into uniform blocks and not having to use a second code path specific to samplers.
As it was, I already had to take special path for samplers (image loading), so, in the end the change was not too drastic, and now I can probably generate SPIR-V binaries from the generated GLSL...
By the way, I am not too happy about having the "Graphics API" independent code generate GLSL now,
Vulkan doesn't natively compile it, and since in theory you can compile SPIR-V from any language, I can see a new more generic language emerging that better fits SPIR-V, the fact that OpenGL 3.3 [supports SPIR-V](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_gl_spirv.txt) doesn't help, but SPIR-V is varely human readable and I suppose much less human "assemblable".
