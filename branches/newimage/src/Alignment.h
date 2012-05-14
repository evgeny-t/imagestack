#ifndef IMAGESTACK_ALIGNMENT_H
#define IMAGESTACK_ALIGNMENT_H

#include "header.h"

class Align : public Operation {
public:
    void help();
    void parse(vector<string> args);

    typedef enum {TRANSLATE = 0, SIMILARITY, AFFINE, PERSPECTIVE, RIGID} Mode;

    static Image apply(Image a, Image b, Mode m);

};

class AlignFrames : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(Image im, Align::Mode m);
};

#include "footer.h"

#endif
