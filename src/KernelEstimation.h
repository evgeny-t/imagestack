#ifndef NO_FFTW
#ifndef KERNELESTIMATION_H
#define KERNELESTIMATION_H
#include "header.h"

class KernelEstimation : public Operation {
  public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int kernel_size = 9);

    // Helpers
    static void NormalizeSum(NewImage im);
    static NewImage EnlargeKernel(NewImage im, int w, int h);
    static NewImage ContractKernel(NewImage im, int size);
    static NewImage BilinearResample(NewImage im, int w, int h);

  private:
    static void ShockFilterIteration(NewImage im, float dt = 1.f);
    static void BilateralFilterIteration(NewImage im, float sigma_r);
    static float DotProduct(NewImage im1, NewImage im2,
                            int channel = 0, int frame = 0);
};

#include "footer.h"
#endif
#endif
