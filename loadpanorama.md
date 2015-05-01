
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help loadpanorama


-loadpanorama takes a filename as its first argument. The file must be the
homography text file output from autostitch. It loads and parses this file, 
and places each warped image in a separate frame. The remaining six arguments
specify minimum and maximum theta, then phi, then the desired output resolution.

Usage: ImageStack -loadpanorama pano.txt -0.1 0.1 -0.1 0.1 640 480 -display

```