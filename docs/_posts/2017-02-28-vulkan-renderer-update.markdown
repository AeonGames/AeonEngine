---
layout: post
title:  "Vulkan Renderer Update"
date:   2017-02-28 14:24:18 -0600
categories: implementation
comments: true
---

I've been working on the Vulkan Renderer, which can use some refactoring since I've been mostly just pouring code into it.
The Initialize function is really long and I still dont understand some of it, but I am really itching to see something on screen,
so my current goal is that, see something on screen.

Even though I have a digital copy of [The Vulkan Programming Guide](https://www.amazon.com/gp/product/0134464540/ref=as_li_tl?ie=UTF8&camp=1789&creative=9325&creativeASIN=0134464540&linkCode=as2&tag=aeon01-20&linkId=ef63b2e2e0954240ed203eadf4d7dd52), I find that following [Nikko Kauppi's Vulkan tutorials](https://www.youtube.com/playlist?list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P) makes it much easier for me to comprehend the API, and for that, I thank him.

Anyway, this commit should include code for the render pass.