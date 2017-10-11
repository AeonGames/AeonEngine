---
layout: post
title: "Currently refactoring a lot of stuff"
date: 2017-10-11 15:29:14 -0600
categories: development,vulkan
comments: true
---

As it turns out, Vulkan is a harsh mistress, things kind of worked but once I re-enabled validation layers all hell broke lose.
I have made many changes in order to keep them happy, but one thing still remains: the fact that you cannot copy or update buffers inside a render pass.
I mean, you obviously can since disabling validation allows it and things do get rendered as expected, but I rather play by the rules, besides, due to the nature of Vulkan I doubt I was going to be able to get away with how I am doing things.

On the plus side it kind of forces you to do things right the first time rather than doing it wrong and then having to come back and fix it later.

I have some ideas on how to move forward:

1. Shared pointers on renderer resources pointing back to the renderer will go away in some cases, when I had the instances of these classes as external objects it made sense, now, I really don't see it other than in Window classes (which are objects external to the renderer).
2. I need UBOs for model matrices and skeletons so I will be adding some sort of stacking UBOs to the renderer implementations, this will serve as temporary per frame storage for the geometry that needs rendering.
3. I have to begin bridging the scene graph code with the renderer code, I am still not sure on how to go about this, so hopefully while I am working on the previous two bullet points something will come up, feel free to give me suggestions, should nodes have-a or be-a model/light/camera/etc and how?

Anyway, this post is mostly about getting some ideas written, I find that when I start describing the problems I find a solution to them most of the time... thats probably why I stopped posting at [gamedev](http://www.gamedev.net), I would start describing the problem, the solution came to me and I would delete the draft.

Anyway, until next time!... Oh right, I have an account at [gratipay](https://gratipay.com/AeonEngine/) now in case you feel the need to support the project.