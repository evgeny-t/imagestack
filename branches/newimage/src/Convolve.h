#ifndef IMAGESTACK_CONVOLVE_H
#define IMAGESTACK_CONVOLVE_H

#include "Arithmetic.h"

#include "header.h"

class Convolve : public Operation {
public:
    void help();
    void parse(vector<string> args);

    enum BoundaryCondition {Zero = 0, Homogeneous, Clamp, Wrap};

    static NewImage apply(NewImage im, NewImage filter, BoundaryCondition b = Zero,
                       Multiply::Mode m = Multiply::Outer);

};

#include "footer.h"
#endif
