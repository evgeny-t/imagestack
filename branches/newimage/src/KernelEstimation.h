#ifndef NO_FFTW
#ifndef KERNELESTIMATION_H
#define KERNELESTIMATION_H
#include "header.h"

class KernelEstimation : public Operation {
  public:
    void help();
    void parse(vector<string> args);
    static Image apply(Image im, int kernel_size = 9);

    // Helpers
    static void NormalizeSum(Image im);
    static Image EnlargeKernel(Image im, int w, int h);
    static Image ContractKernel(Image im, int size);
    static Image BilinearResample(Image im, int w, int h);

  private:
    static void ShockFilterIteration(Image im, float dt = 1.f);
    static void BilateralFilterIteration(Image im, float sigma_r);
};

#include "footer.h"
#endif
#endif
