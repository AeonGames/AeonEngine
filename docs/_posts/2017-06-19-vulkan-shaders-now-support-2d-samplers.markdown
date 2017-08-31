---
layout: post
title: "Vulkan shaders now support 2D samplers"
date: 2017-06-19 14:41:19 -0600
categories: vulkan shaders sampler texture
comments: true
---

It is finally posible to load and use texture images on the Vulkan renderer, this has worked for a while on the OpenGL renderer, and it probably was the last thing I worked on in that renderer that was **NOT** related to Vulkan.

I am still not that comfortable with Vulkan, so the code definitely has some flaws and it is not fully optimized yet. Nevertheless I can now start creating better shaders, and post a screen shot once in a while. Pipelines exist and they work, models and meshes do as well, but materials do not, I've been putting that off for a while out of pure laziness :grin:, so thats probably what comes next. After that I'll move the animation code from the old engine into this new one, and then I'll move the scene and collision code, that should jump start work on an actual game.

I want to rework how shaders are built, I want to create a visual language editor as the current method seems flexible on the surface but it is not, you must write GLSL 420 if you want to use the same shaders on OpenGL or Vulkan and even then, if you write them targeting OpenGL, it will let you get away with things Vulkan won't (like not setting a location on an in or out variable as long as they have the same name). So if I write a visual editor, I can generate valid GLSL across the board, and will be much easier to work with than what we have.

I do need to think on the approach though.