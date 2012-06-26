#include "main.h"
#include "Prediction.h"
#include "Filter.h"
#include "Paint.h"
#include "File.h"
#include "Calculus.h"
#include "header.h"

void Inpaint::help() {
    printf("\n-inpaint takes the image on the top of the stack, and a one channel mask of the\n"
           "same size second on the stack, and diffuses areas of the image where the mask is\n"
           "high into areas of the image where the mask is low. Image pixels with mask of 1\n"
           "are unchanged.\n\n"
           "Usage: ImageStack -push 1 640 480 1 -eval \"(X > 0.5)*(X < 0.6)\" -load in.jpg\n"
           "                  -inpaint -save out.jpg\n\n");
}

void Inpaint::parse(vector<string> args) {
    assert(args.size() == 0, "-inpaint takes no arguments\n");
    Image im = apply(stack(0), stack(1));
    pop();
    push(im);
}

Image Inpaint::apply(Image im, Image mask) {
    assert(im.width == mask.width &&
           im.height == mask.height &&
           im.frames == mask.frames,
           "mask must be the same size as the image\n");
    assert(mask.channels == 1,
           "mask must have one channel\n");

    const int J = 15;
    Image blurred[J], blurredMask[J];

    blurredMask[0] = mask.copy();
    FastBlur::apply(blurredMask[0], 1, 1, 1);
    blurredMask[0].set(max(mask - blurredMask[0], 0));

    blurred[0] = im.copy();
    for (int c = 0; c < im.channels; c++) {
        blurred[0].channel(c) *= blurredMask[0];
    }

    for (int i = 1; i < J; i++) {
        blurred[i] = blurred[i-1].copy();
        blurredMask[i] = blurredMask[i-1].copy();
        float alpha = powf(1.5, i)/4;
        FastBlur::apply(blurred[i], alpha, alpha, alpha);
        FastBlur::apply(blurredMask[i], alpha, alpha, alpha);
        blurred[i] *= 1.5;
        blurredMask[i] *= 1.5;
    }

    for (int i = 0; i < J; i++) {
        for (int c = 0; c < blurred[i].channels; c++) {
            blurred[i].channel(c) /= (blurredMask[i] + 1e-10);
        }
    }
    
    for (int i = J-2; i >= 0; i--) {
        blurredMask[i].set(min(blurredMask[i] * 3, 1));
        Composite::apply(blurred[J-1], blurred[i], blurredMask[i]);
    }

    Composite::apply(blurred[J-1], im, mask);


    return blurred[J-1];
}


#include "footer.h"
