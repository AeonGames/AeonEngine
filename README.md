# AeonEngine
Aeon Games Flagship Game Engine

This is the 3rd iteration of the engine, the first one was started circa 1996 and was lost on a hard drive crash, the second one was started circa 2001 and still exists, but is a mess and a patchwork of collected ideas of 15 years of trying to keep up.

THIS IS A WORK IN PROGRESS.

In No Way Complete TODO List:
=============================

* Find out how to use google::protobuf::TextFormat::Parser so raw mesh bytes can be printed and parsed in a more human readable way.

Unasked Questions Nevertheless Answered (UQNA)
----------------------------------------------

* Why do you use protobuf for your data files?
	Because I've always felt human readability is not worth the price you pay in performace.
* Why do you keep PB plain text files around then?
	They're easier to modify. The idea is that you convert them to binary once you're ready to ship.
* You could do that with <insert favorite human readable format> which is nicer, so why don't you?
	PB's text files are a build in feature, anything else would require a tool to either convert to it, directly to binary protocol buffers or use a proprietary format. That takes time and Google already solved the problem. Do feel free to write your own convertion tool.