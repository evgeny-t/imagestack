#ifndef IMAGESTACK_PATCHMATCH_H
#define IMAGESTACK_PATCHMATCH_H
#include "header.h"

class PatchMatch : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage source, NewImage target, int iterations, int patchSize);
    static NewImage apply(NewImage source, NewImage target, NewImage mask, int iterations, int patchSize);

private:
    static float distance(NewImage source, NewImage target, NewImage mask,
                          int st, int sx, int sy,
                          int tt, int tx, int ty,
                          int patchSize, float prevDist);

};



class BidirectionalSimilarity : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage source, NewImage target,
                      NewImage sourceMask, NewImage targetMask,
                      float alpha, int numIter, int numIterPM = 5);

};


class Heal : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

#include "footer.h"
#endif
