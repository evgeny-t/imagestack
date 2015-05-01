
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help loop


-loop takes an integer and a sequence of commands, and loops that sequence
the specified number of times. The commands that form the argument must be
prefixed with an extra dash. It is possible to nest this operation using more
dashes. If given no integer argument, loop will loop forever.

Usage: ImageStack -load a.tga -loop 36 --rotate 10 --loop 10 ---downsample
                  ---upsample -save b.tga

```