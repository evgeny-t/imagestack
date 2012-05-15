#ifndef IMAGESTACK_LOCAL_LAPLACIAN_H
#define IMAGESTACK_LOCAL_LAPLACIAN_H
#include "header.h"

class LocalLaplacian : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static Image apply(const Image im, float alpha, float beta);
 private:
    static Image pyramidDown(const Image im);
    static Image pyramidUp(const Image im, int w, int h, int f);
};


#include "footer.h"
#endif
