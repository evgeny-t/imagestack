
---

_This page was auto-generated from the ImageStack -help operator. Direct edits to it will be lost. Use the comments below to discuss this operation._

---

```
ImageStack -help eval


-eval takes a simple expression and evaluates it, writing the result to the
current image.

Variables:
  x   	 the x coordinate, measured from 0 to width - 1
  y   	 the y coordinate, measured from 0 to height - 1
  t   	 the t coordinate, measured from 0 to frames - 1
  c   	 the current channel
  val 	 the image value at the current x, y, t, c

Constants:
  e  	 2.71828183
  pi 	 3.14159265

Uniforms:
  frames 	 the number of frames
  width 	 the image width
  height 	 the image height
  channels 	 the number of channels
Unary Operations:
  -  	 unary negation

Binary Operations:
  +  	 addition
  -  	 subtraction
  %  	 modulo
  *  	 multiplication
  /  	 division
  ^  	 exponentiation
  >  	 greater than
  <  	 less than
  >= 	 greater than or equal to
  <= 	 less than or equal to
  == 	 equal to
  != 	 not equal to

Other Operations:
  a ? b : c 	 if a then b else c
  [c]    	 sample the image here on channel c
  [x, y] 	 sample the image with a 3 lobed lanczos filter at X, Y at this channel
  [x, y, t] 	 sample the image with a 3 lobed lanczos filter at X, Y, T at this channel

Functions:
  log(x)      	 the natural log of x
  exp(x)      	 e to the power of x
  sin(x)      	 the sine of x
  cos(x)      	 the cosine of x
  tan(x)      	 the tangent of x
  asin(x)     	 the inverse sine of x
  acos(x)     	 the inverse cosine of x
  atan(x)     	 the inverse tangent of x
  atan2(y, x) 	 the angle of the vector x, y above horizontal
  abs(x)      	 the absolute value of x
  floor(x)    	 the value of x rounded to the nearest smaller integer
  ceil(x)     	 the value of x rounded to the nearest larger integer
  round(x)    	 the value of x rounded to the nearest integer
  mean()      	 the mean value of the image across all channels
  mean(c)     	 the mean value of the image in channel c
  sum()       	 the sum of the image across all channels
  sum(c)      	 the sum of the image in channel c
  max()       	 the maximum of the image across all channels
  max(c)      	 the maximum of the image in channel c
  min()       	 the minimum of the image across all channels
  min(c)      	 the minimum of the image in channel c
  stddev()    	 the standard deviation of the image across all channels
  stddev(c)   	 the standard deviation of the image in channel c
  variance()  	 the variance of the image across all channels
  variance(c) 	 the variance of the image in channel c
  skew()      	 the skew of the image across all channels
  skew(c)     	 the skew of the image in channel c
  kurtosis()  	 the kurtosis of the image across all channels
  kurtosis(c) 	 the kurtosis of the image in channel c
  covariance(c1, c2) 	 the covariance between channels c1 and c2

To add more functionality, see Parser.cpp in the source

Usage: ImageStack -push 128 128 128 1 -eval "(x*y*t)^0.5" -save out.tga

```