#ifndef IMAGESTACK_PREDICTION_H
#define IMAGESTACK_PREDICTION_H
#include "header.h"

class Inpaint : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static Image apply(Image im, Image mask);
};

#include "footer.h"
#endif
