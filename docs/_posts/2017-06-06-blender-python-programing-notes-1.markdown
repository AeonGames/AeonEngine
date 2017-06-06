---
layout: post
title:  "Blender's Python API Programming Notes Part 1"
date:   2017-06-06 11:21:18 -0600
categories: programming, blender, python, import, 3d
comments: true
---

This is a collection of notes to keep in mind for the next time I work with Blender's Python API.

### The coordinate system in Blender is Right Handed.

Further, there is a slight difference between world coordinates and view coordinates.
View coordinates are the standard OpenGL coordinates, +X is Right, +Y is Up and -Z is Forward,
however, looking at the world axes you'll notice that +X is Right, +Y is Forward and +Z is Up.
The system remains the same, but notice that the world axes are rotated -90 degrees on X.

In general, to convert a matrix from one coordinate system to the other, you have to pre *and* post
multiply it by an identity matrix with one of the 1 scalar elements negated, in effect,
if you negate the first column's 1 (or row, see below) you'll be flipping the X axis,
if you negate the second column you'll be flipping the Y axis 
and finally if you negate the third column you'll be flipping the Z axis.
The choise of which axis to flip depends on where are you exporting to or importing from,
For UDK, I found out that you need to flip Z *and* rotate 180 degrees on Z because in UDK +X is forward.
Note however that you must either negate just one of the columns or all three of them, if you randomly negate
two, you'll be doing a 180 degree rotation on a random X,Y,or Z axis, and will *not* be flipping an axis which
is what a coordinate system change implies.

This information may not seem too relevant and may even seem obvious,
but keep it in mind for upcoming bulletpoints.

### Matrices (mathutils.Matrix) are row mayor.

I am used to think of matrices in column mayor terms because that's the natural way matrices are layed out in memory
in OpenGL. I dont think this has much of an impact on the actual code as transforms are done in Scale-Rotation-Translation order anyway.

### The matrix in Blender's edit bones is calculated from the head and tail position

Now we can directly set the matrix in an EditBone, and from that, the head and tail will be computed (except for the length of the bone),
but what really matters internally is the head and tail. For this reason, when importing it is important to try not to change the tail,
if you do, animations will be harder to import, and exporting will produce a very different file than the one first imported.
Yes, I know, it is nice to have tails match children or at least point to their general area, but you'll avoid animation import nightmares
if you can just live with weirdly angled bones.
By the way, the "identity" bone would be head at (0,0,0) tail at (0,1,0).

