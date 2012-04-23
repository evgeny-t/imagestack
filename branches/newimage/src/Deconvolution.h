#ifndef NO_FFTW
#ifndef DECONVOLUTION_H
#define DECONVOLUTION_H
#include "header.h"

class Deconvolve : public Operation {
  public:
    void help();
    void parse(vector<string> args);
    static NewImage applyCho2009(NewImage im, NewImage kernel);
    static NewImage applyShan2008(NewImage im, NewImage kernel);
    static NewImage applyLevin2007(NewImage im, NewImage kernel, float weight);
  private:
    static NewImage applyPadding(NewImage im);
};

#include "footer.h"
#endif
#endif
