#ifndef NO_FFTW
#ifndef DFT_H
#define DFT_H
#include "header.h"

class DCT : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(Image win, bool x = true, bool y = true, bool t = true);
};

class FFT : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(Image im, bool x = true, bool y = true, bool t = true, bool inverse = false);
};

class IFFT : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(Image im, bool x = true, bool y = true, bool t = true);
};

#include "Convolve.h"

class FFTConvolve : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static Image apply(Image im, Image filter, Convolve::BoundaryCondition b, Multiply::Mode m);
};

class FFTPoisson : public Operation {
public:
    void help();
    void parse(vector<string> args);

    // Return an image which has gradients dx and dy, and is somewhat similar to the target
    static Image apply(Image dx, Image dy, Image target, float targetStrength = 0);
};

#include "footer.h"
#endif
#endif
