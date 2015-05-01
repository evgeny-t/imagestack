
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help plugin

-plugin loads a shared object that can add new operations to ImageStack. It
does this by calling the function init_imagestack_plugin, which should exist in
the shared object with C linkage. To this function it passes the ImageStack
operation map, into which the plugin may inject new operations.

Usage: ImageStack -plugin foo.so -foo 1 2 3
```