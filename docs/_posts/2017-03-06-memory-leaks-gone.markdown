---
layout: post
title:  "Memory leaks are gone"
date:   2017-03-06 12:56:18 -0600
categories: debugging
---

The memory leak issues I was seeing have magickally banished.

Well, not extrictly magickally, but I was not seeing them in a fresh installed computer with Qt 5.8.0,
so I updated the one computer where I was seeing the issues from Qt 5.6.0 to Qt 5.8.0 and initally
the issues remained, but today, after a couple of reboots, they are gone.

This pretty much pinpoints the culprit to be Qt, I was seeing allocations being made when the mouse moved, so that actually makes a lot of sence. This also means that the issues are gone, so I see no point in continuing with the debug log code experiment any longer than I have already. I still think the functions I was creating would become very useful in the future, but I won't immediatelly continue on that path as I see no point now on wasting time on something that does not address a current pressing issue.

So, back to Vulkan it is!.