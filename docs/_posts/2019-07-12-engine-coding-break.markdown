---
layout: post
title: "Engine Coding Break"
date: 2019-07-12 16:42:28 -0600
categories: engine
comments: true
---

I know it seems like there is lately no progress on the engine, this is because I came to a point where I realized I needed a game graphical user interface (GUI) and after looking around at what is available, and not find anything that matched my expectations, I decided to revisit my own GUI project from a while back.

So, that's what I've been doing, writing code for my long forgotten GUI library alongside contributing some ports to [vcpkg](https://github.com/Microsoft/vcpkg), in particular libcroco and librsvg, which turns out I may not end up using anyway.

The GUI library can be found [here](https://github.com/AeonGames/AeonGUI), my current idea is to create a [Scalable Vector Graphics](https://en.wikipedia.org/wiki/Scalable_Vector_Graphics) (SVG) [user agent](https://en.wikipedia.org/wiki/User_agent) that's easily embeddable into a game engine while being engine agnostic. It would be like a limited browser which doesn't render HTML but SVG documents exclusively and that does all the drawing to memory to be easily moved or copied to a texture, surface, bitmap, or anything else.

So far I am happy with the results, but working on top of my old ideas has turned the code into a mess, I know were what I need is, but others may have problems understanding why there is so much dead code and even some unused CUDA kernels in there.

Anyway, just wanted to let you know that the project is not dead yet, its just that some other part that needs to live as a separate entity is getting some well deserved love.

Until Next Time! Bye.