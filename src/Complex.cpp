#include "main.h"
#include "Complex.h"
#include "header.h"

void ComplexMultiply::help() {
    pprintf("-complexmultiply multiplies the top image in the stack by the second"
            " image in the stack, using 2 \"complex\" images as its input - a"
            " \"complex\" image is one where channel 2*n is the real part of the nth"
            " channel and channel 2*n + 1 is the imaginary part of the nth"
            " channel. Using zero arguments results in a straight multiplication"
            " (a + bi) * (c + di), using one argument results in a conjugate"
            " multiplication (a - bi) * (c + di).\n"
            "\n"
            "Usage: ImageStack -load a.tga -load b.tga -complexmultiply -save out.tga.\n");
}

bool ComplexMultiply::test() {
    return false;
}

void ComplexMultiply::parse(vector<string> args) {
    assert(args.size() < 2, "-complexmultiply takes zero or one arguments\n");
    if (stack(0).channels == 2 && stack(1).channels > 2) {
        apply(stack(1), stack(0), (bool)args.size());
        pop();
    } else {
        apply(stack(0), stack(1), (bool)args.size());
        pull(1);
        pop();
    }
}

void ComplexMultiply::apply(Image a, Image b, bool conj = false) {
    assert(a.channels % 2 == 0 && b.channels % 2 == 0,
           "-complexmultiply requires images with an even number of channels (%d %d)\n",
           a.channels, b.channels);

    assert(a.frames == b.frames &&
           a.width == b.width &&
           a.height == b.height,
           "images must be the same size\n");

    if (a.channels == 2 && b.channels == 2) {
        // Scalar times scalar
        Image a_real = a.channel(0), a_imag = a.channel(1);
        Image b_real = b.channel(0), b_imag = b.channel(1);
        if (conj) {
            a.setChannels(a_real*b_real + a_imag*b_imag,
                          a_imag*b_real - a_real*b_imag);
        } else {
            a.setChannels(a_real*b_real - a_imag*b_imag,
                          a_imag*b_real + a_real*b_imag);
        }
    } else if (b.channels == 2) {
        // Vector times scalar
        for (int c = 0; c < a.channels; c += 2) {
            apply(a.selectChannels(c, 2), b, conj);
        }
    } else {
        // Vector times vector (elementwise)
        for (int c = 0; c < a.channels; c += 2) {
            apply(a.selectChannels(c, 2),
                  b.selectChannels(c, 2), conj);
        }
    }
}


void ComplexDivide::help() {
    pprintf("-complexdivide divides the top image in the stack by the second image"
            " in the stack, using 2 \"complex\" images as its input - a \"complex\""
            " image is one where channel 2*n is the real part of the nth channel and"
            " channel 2*n + 1 is the imaginary part of the nth channel. Using zero"
            " arguments results in a straight division (a + bi) / (c + di). Using"
            " one argument results in a conjugate division (a - bi) / (c + di).\n"
            "\n"
            "Usage: ImageStack -load a.tga -load b.tga -complexdivide -save out.tga.\n");
}

bool ComplexDivide::test() {
    return false;
}

void ComplexDivide::parse(vector<string> args) {
    assert(args.size() == 0 || args.size() == 1,
           "-complexdivide takes zero or one arguments\n");
    if (stack(0).channels == 2 && stack(1).channels > 2) {
        apply(stack(1), stack(0), (bool)args.size());
        pop();
    } else {
        apply(stack(0), stack(1), (bool)args.size());
        pull(1);
        pop();
    }
}

void ComplexDivide::apply(Image a, Image b, bool conj = false) {
    assert(a.channels % 2 == 0 && b.channels % 2 == 0,
           "-complexdivide requires images with an even number of channels\n");

    assert(a.frames == b.frames &&
           a.width == b.width &&
           a.height == b.height,
           "images must be the same size\n");

    if (a.channels == 2 && b.channels == 2) {
        // Scalar over scalar
        Image a_real = a.channel(0), a_imag = a.channel(1);
        Image b_real = b.channel(0), b_imag = b.channel(1);
        auto denom = b_real*b_real + b_imag*b_imag;
        if (conj) {
            a.setChannels((a_real * b_real - a_imag * b_imag) / denom,
                          (a_imag * b_real + a_real * b_imag) / denom);
        } else {
            a.setChannels((a_real * b_real + a_imag * b_imag) / denom,
                          (a_imag * b_real - a_real * b_imag) / denom);
        }
    } else if (b.channels == 2) {
        // Vector over scalar
        for (int c = 0; c < a.channels; c += 2) {
            apply(a.selectChannels(c, 2), b, conj);
        }
    } else {
        // Vector over vector (elementwise)
        for (int c = 0; c < a.channels; c += 2) {
            apply(a.selectChannels(c, 2),
                  b.selectChannels(c, 2), conj);
        }
    }    
}


void ComplexReal::help() {
    pprintf("-complexreal takes a \"complex\" image, in which the even channels"
            " represent the real component and the odd channels represent the"
            " imaginary component, and produces an image containing only the real"
            " channels.\n"
            "\n"
            "Usage: ImageStack -load a.png -fftreal -complexreal -display\n");
}

bool ComplexReal::test() {
    return false;
}

void ComplexReal::parse(vector<string> args) {
    assert(args.size() == 0, "-complexreal takes no arguments\n");
    Image im = apply(stack(0));
    pop();
    push(im);
}

Image ComplexReal::apply(Image im) {
    assert(im.channels % 2 == 0,
           "complex images must have an even number of channels\n");

    Image out(im.width, im.height, im.frames, im.channels/2);

    for (int c = 0; c < out.channels; c++) {
        out.channel(c).set(im.channel(2*c));
    }

    return out;
}

void RealComplex::help() {
    pprintf("-realcomplex takes a \"real\" image, and converts it to a \"complex\""
            " image, in which the even channels represent the real component and"
            " the odd channels represent the imaginary component.\n"
            "\n"
            "Usage: ImageStack -load a.png -realcomplex -fft -display\n");
}

bool RealComplex::test() {
    return false;
}

void RealComplex::parse(vector<string> args) {
    assert(args.size() == 0, "-complexreal takes no arguments\n");
    Image im = apply(stack(0));
    pop();
    push(im);
}

Image RealComplex::apply(Image im) {
    Image out(im.width, im.height, im.frames, im.channels*2);
    
    for (int c = 0; c < im.channels; c++) {                    
        out.channel(2*c).set(im.channel(c));
    }

    return out;
}

void ComplexImag::help() {
    pprintf("-compleximag takes a \"complex\" image, in which the even channels"
            " represent the real component and the odd channels represent the"
            " imaginary component, and produces an image containing only the imaginary"
            " channels.\n"
            "\n"
            "Usage: ImageStack -load a.png -fftreal -compleximag -display\n");
}

bool ComplexImag::test() {
    return false;
}

void ComplexImag::parse(vector<string> args) {
    assert(args.size() == 0, "-compleximag takes no arguments\n");
    Image im = apply(stack(0));
    pop();
    push(im);
}

Image ComplexImag::apply(Image im) {
    assert(im.channels % 2 == 0,
           "complex images must have an even number of channels\n");

    Image out(im.width, im.height, im.frames, im.channels/2);

    for (int c = 0; c < out.channels; c++) {
        out.channel(c).set(im.channel(2*c+1));
    }

    return out;
}


void ComplexMagnitude::help() {
    pprintf("-complexmagnitude takes a \"complex\" image, in which the even channels"
            " represent the real component and the odd channels represent the"
            " imaginary component, and produces an image containing the complex"
            " magnitude\n"
            "\n"
            "Usage: ImageStack -load a.png -fftreal -complexmagnitude -display\n");
}

bool ComplexMagnitude::test() {
    return false;
}

void ComplexMagnitude::parse(vector<string> args) {
    assert(args.size() == 0, "-complexmagnitude takes no arguments\n");
    Image im = apply(stack(0));
    pop();
    push(im);
}

Image ComplexMagnitude::apply(Image im) {
    assert(im.channels % 2 == 0,
           "complex images must have an even number of channels\n");

    Image out(im.width, im.height, im.frames, im.channels/2);

    for (int c = 0; c < out.channels; c++) {
        Image real = im.channel(2*c);
        Image imag = im.channel(2*c+1);
        out.channel(c).set(sqrt(real*real + imag*imag));
    }

    return out;
}



void ComplexPhase::help() {
    pprintf("-complexphase takes a \"complex\" image, in which the even channels"
            " represent the real component and the odd channels represent the"
            " imaginary component, and produces an image containing the complex"
            " phase\n"
            "\n"
            "Usage: ImageStack -load a.png -fftreal -complexphase -display\n");
}

bool ComplexPhase::test() {
    return false;
}

void ComplexPhase::parse(vector<string> args) {
    assert(args.size() == 0, "-complexphase takes no arguments\n");
    Image im = apply(stack(0));
    pop();
    push(im);
}

Image ComplexPhase::apply(Image im) {
    assert(im.channels % 2 == 0, "complex images must have an even number of channels\n");

    Image out(im.width, im.height, im.frames, im.channels/2);    

    for (int c = 0; c < out.channels; c++) {
        Image real = im.channel(2*c);
        Image imag = im.channel(2*c+1);
        out.channel(c).set(Lazy::atan2(imag, real));
    }

    return out;
}


void ComplexConjugate::help() {
    pprintf("-complexconjugate takes a \"complex\" image, in which the even channels"
            " represent the real component and the odd channels represent the"
            " imaginary component, and produces an image containing the complex"
            " conjugate\n"
            "\n"
            "Usage: ImageStack -load a.png -fftreal -complexconjugate -display\n");
}

void ComplexConjugate::parse(vector<string> args) {
    assert(args.size() == 0, "-complexconjugate takes no arguments\n");
    apply(stack(0));
}

bool ComplexConjugate::test() {
    return false;
}

void ComplexConjugate::apply(Image im) {
    assert(im.channels % 2 == 0, "complex images must have an even number of channels\n");

    for (int c = 1; c < im.channels; c+=2) {
        im.channel(c).set(-im.channel(c));
    }
}

#include "footer.h"
