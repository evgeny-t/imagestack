#ifndef IMAGESTACK_PATCHMATCH_H
#define IMAGESTACK_PATCHMATCH_H
#include "header.h"

class PatchMatch : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static Image apply(Image source, Image target, int iterations, int patchSize);
    static Image apply(Image source, Image target, Image mask, int iterations, int patchSize);

private:
    static float distance(Image source, Image target, Image mask,
                          int st, int sx, int sy,
                          int tt, int tx, int ty,
                          int patchSize, float prevDist);

};



class BidirectionalSimilarity : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(Image source, Image target,
                      Image sourceMask, Image targetMask,
                      float alpha, int numIter, int numIterPM = 5);

};


class Heal : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

#include "footer.h"
#endif
