
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help receive


-receive sets up a TCP server and listens for an image (such as that sent by
-send). The sole optional argument is the port to listen on. It defaults
to 5678.

Usage: ImageStack -receive 5243 -save image.jpg

       ImageStack -loop --receive --scale 2 --send somewhereelse

```