#ifndef IMAGESTACK_ALIGNMENT_H
#define IMAGESTACK_ALIGNMENT_H

#include "header.h"

class Align : public Operation {
public:
    void help();
    void parse(vector<string> args);

    typedef enum {TRANSLATE = 0, SIMILARITY, AFFINE, PERSPECTIVE, RIGID} Mode;

    static NewImage apply(NewImage a, NewImage b, Mode m);

};

class AlignFrames : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage im, Align::Mode m);
};

#include "footer.h"

#endif
