---
layout: post
title:  "Visual Studio 2017 released, Qt broken."
date:   2017-03-13 12:30:18 -0600
categories: development
comments: true
---

Last week Visual Studio 2017 was released and I upgraded to it. I build Qt from source and I expected the build from VS 2015 to work on 2017, but it didn't work because of some constexpr issue. There is a fix, it apparently was merged back in November last year, but it didn't make it to the Qt 5.8.0 release.

The Qt build is taking too long to complete, and I had to do it on 3 different machines, that's why I have not been uploading too much work lately and it's side effect is that Linux build is broken. The broken build is nothing serious, just that I have had not been able to boot into Linux and make the changes, and I rather not shot in the dark some fixes on Windows and hope that the Linux build is fixed.

Anyway, to find out why Qt 5.8.0 on Visual C++ 2017 is broken, see [this bug report](https://bugreports.qt.io/browse/QTBUG-57086) and to solve the issue on the released 5.8.0 source zip file save [this](https://codereview.qt-project.org/gitweb?p=qt/qtbase.git;a=blob_plain;f=src/corelib/tools/qalgorithms.h;h=7e846956f5fc60e720c1f075cca69ea75e86d80b;hb=a103992f49045323a3aaa4970eb1ee5f65a378dd) file at <source>/qtbase/src/corelib/tools/qalgorithms.h and do your build as usual. I've seen some subdirectories failing to build due to the linker trying to link release compilation units against debug libraries or viceversa, but cd into the directory and cleaning and rebuilding just the failing subdirectory seems to work.