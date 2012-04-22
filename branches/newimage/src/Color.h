#ifndef IMAGESTACK_COLOR_H
#define IMAGESTACK_COLOR_H
#include "header.h"

class ColorMatrix : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, vector<float> matrix);
};

class ColorConvert : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, string from, string to);
    static NewImage rgb2hsv(NewImage im);
    static NewImage hsv2rgb(NewImage im);
    static NewImage rgb2y(NewImage im);
    static NewImage y2rgb(NewImage im);
    static NewImage rgb2yuv(NewImage im);
    static NewImage yuv2rgb(NewImage im);
    static NewImage rgb2xyz(NewImage im);
    static NewImage xyz2rgb(NewImage im);
    static NewImage lab2xyz(NewImage im);
    static NewImage xyz2lab(NewImage im);
    static NewImage rgb2lab(NewImage im);
    static NewImage lab2rgb(NewImage im);

    static NewImage uyvy2yuv(NewImage im);
    static NewImage yuyv2yuv(NewImage im);

    static NewImage uyvy2rgb(NewImage im);
    static NewImage yuyv2rgb(NewImage im);

    static NewImage argb2xyz(NewImage im);
    static NewImage xyz2argb(NewImage im);

    static NewImage argb2rgb(NewImage im);
    static NewImage rgb2argb(NewImage im);
};

class Demosaic : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage win, int xoff, int yoff, bool awb);
};

#include "footer.h"
#endif
