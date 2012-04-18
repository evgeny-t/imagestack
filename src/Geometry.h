#ifndef IMAGESTACK_GEOMETRY_H
#define IMAGESTACK_GEOMETRY_H
#include "header.h"

class Upsample : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int boxWidth, int boxHeight, int boxFrames = 1);
};

class Downsample : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int boxWidth, int boxHeight, int boxFrames = 1);
};

class Subsample : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int boxWidth, int boxHeight, int boxFrames,
                       int offsetX, int offsetY, int offsetT);
    static NewImage apply(NewImage im, int boxWidth, int boxHeight,
                       int offsetX, int offsetY);
    static NewImage apply(NewImage im, int boxFrames, int offsetT);
};

class Interleave : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage im, int rx, int ry, int rt = 1);
};

class Deinterleave : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage im, int ix, int iy, int it = 1);
};

class Resample : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int width, int height);
    static NewImage apply(NewImage im, int width, int height, int frames);
private:
    static void computeWeights(int oldSize, int newSize, vector<vector<pair<int, float> > > &matrix);
    static NewImage resampleT(NewImage im, int frames);
    static NewImage resampleX(NewImage im, int width);
    static NewImage resampleY(NewImage im, int height);
};

class Rotate : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, float degrees);
};

class AffineWarp : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, vector<double> warp);
};

class Crop : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int minX, int minY, int width, int height);
    static NewImage apply(NewImage im, int minX, int minY, int minT, int width, int height, int frames);
    static NewImage apply(NewImage im);
};

class Flip : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage im, char dimension);
};

class Adjoin : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage a, NewImage b, char dimension);
};

class Transpose : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, char arg1, char arg2);
};

class Translate : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, float xoff, float yoff, float toff = 0);
private:
    static NewImage applyX(NewImage im, float xoff);
    static NewImage applyY(NewImage im, float yoff);
    static NewImage applyT(NewImage im, float toff);
};

class Paste : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage into, NewImage from,
                      int xdst, int ydst,
                      int xsrc, int ysrc,
                      int width, int height);

    static void apply(NewImage into, NewImage from,
                      int xdst, int ydst, int tdst = 0);

    static void apply(NewImage into, NewImage from,
                      int xdst, int ydst, int tdst,
                      int xsrc, int ysrc, int tsrc,
                      int width, int height, int frames);
};

class Tile : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int xTiles, int yTiles, int tTiles = 1);
};

class TileFrames : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int xTiles, int yTiles);
};

class FrameTiles : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int xTiles, int yTiles);
};

class Warp : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage coords, NewImage source);
};

class Reshape : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int newWidth, int newHeight, int newFrames, int newChannels);
};

#include "footer.h"
#endif
