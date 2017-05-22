---
layout: post
title:  "Added clang-tidy checks."
date:   2017-03-13 17:49:18 -0600
categories: development
comments: true
---

I've just added optional support for clang-tidy checks. clang-tidy is sort of a static analisys tool which is part of [clang](https://clang.llvm.org/), itself part of [LLVM](https://llvm.org/).

Its use is optional and in fact is disabled by default, as it is only suported by CMake 3.6.0 and up (Ubuntu xenial so far ships with 3.5.1), only on "Unix like environments" and it makes the build process take more time. Nevertheless, the USE\_CLANG\_TIDY option is now available for whoever wants to use it, the idea is to eventualy remove all warnings, but for now, the requirement is lax.

I'll be reworking some custom targets to apply (clang-tidy -fix) the sugested changes soon, so removing the warnings should become easier.
