---
layout: post
title:  "Sidetracked by memory leaks"
date:   2017-03-03 17:45:18 -0600
categories: debugging
comments: true
---

The Vulkan renderer finaly blanks the window, so I started to clean up before moving on to actually render something to it. While tidying up however I noticed that Visual Studio CrtDbg was reporting some leaks on the model viewer. I expected those to be related to Vulkan but they also appear with the OpenGL renderer.

Aparently, they must be related to either [Protocol Buffers](https://developers.google.com/protocol-buffers/) **OR** [Qt](https://www.qt.io/), and sadly while I get a report of leaks from CrtDbg, there is no info on file, function or line for the allocation and the allocations secuential number always change.

To solve this problem I am cooking something up, messing with the stack frame and DLL export table to at least be able to find out which function is doing the allocations. I think this may become a tool on its own, may not, but current results seem positive.

Anyway, right now, what is is WIP, I may post some details later if I actually get somewhere with it.