#ifndef IMAGESTACK_LOCAL_LAPLACIAN_H
#define IMAGESTACK_LOCAL_LAPLACIAN_H
#include "header.h"

class LocalLaplacian : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, float alpha, float beta);
 private:
    static NewImage pyramidDown(NewImage im);
    static NewImage pyramidUp(NewImage im, int w, int h, int f);
};


#include "footer.h"
#endif
