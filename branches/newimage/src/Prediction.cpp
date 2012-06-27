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

bool Inpaint::test() {
    return false;
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

    // Make a mask that just selects the outline
    Image boundaryMask = mask.copy();
    FastBlur::apply(boundaryMask, 1, 1, 1);
    boundaryMask.set(max(mask - boundaryMask, 0));

    // Mask out the input image with it
    Image boundary = im.copy();    
    for (int c = 0; c < im.channels; c++) {
        boundary.channel(c) *= boundaryMask;
    }

    Image blurred = boundary.copy();
    Image blurredMask = boundaryMask.copy();

    const int J = 10;
    for (int i = J; i >= 0; i--) {
        float alpha = powf(i, 2)/4;
        // Smooth
        FastBlur::apply(blurred, alpha, alpha, alpha);
        FastBlur::apply(blurredMask, alpha, alpha, alpha);
        // Reimpose boundary conditions
        Composite::apply(blurred, boundary, boundaryMask);
        Composite::apply(blurredMask, boundaryMask, boundaryMask);
    }

    for (int c = 0; c < im.channels; c++) {
        blurred.channel(c) /= blurredMask;
    }

    // Reattach the rest of the image    
    Composite::apply(blurred, im, mask);
    return blurred;
}



void SeamlessClone::help() {
    pprintf("-seamlessclone composites the top image in the stack over the next image in"
           " the stack, using the last channel in the top image in the stack as alpha."
           " The composite is done in such a way as to avoid hard edges around the"
            " boundaries. If the top image in the stack has only one channel, it"
            " interprets this as a mask, and composites the second image in the"
            " stack over the third image in the stack using that mask.\n"
            "\n"
            "Usage: ImageStack -load a.jpg -load b.jpg -load mask.png -seamlessclone\n"
            "       ImageStack -load a.jpg -load b.jpg -evalchannels [0] [1] [2] \\\n"
            "       \"x>width/2\" -seamlessclone -display\n\n");
}

bool SeamlessClone::test() {
    return false;
}

void SeamlessClone::parse(vector<string> args) {
    assert(args.size() == 0, "-seamlessclone takes no arguments\n");

    if (stack(0).channels == 1) {
        apply(stack(2), stack(1), stack(0));
        pop();
        pop();
    } else {
        apply(stack(1), stack(0));
        pop();
    }
}

void SeamlessClone::apply(Image dst, Image src) {
    assert(src.channels > 1, "Source image needs at least two channels\n");
    assert(src.channels == dst.channels || src.channels == dst.channels + 1,
           "Source image and destination image must either have matching channel"
           " counts (if they both have an alpha channel), or the source image"
           " should have one more channel than the destination.\n");
    assert(dst.frames == src.frames && dst.width == src.width 
           && dst.height == src.height,
           "The source and destination images must be the same size\n");

    if (src.channels > dst.channels) {
        apply(dst, 
              src.region(0, 0, 0, 0, 
			 src.width, src.height, 
			 src.frames, dst.channels),
              src.channel(dst.channels));
              
    } else {
        apply(dst, src, src.channel(dst.channels-1));
    }
}

void SeamlessClone::apply(Image dst, Image src, Image mask) {
    assert(src.channels == dst.channels, "The source and destination images must have the same number of channels\n");

    assert(dst.frames == src.frames && dst.width == src.width && dst.height == src.height,
           "The source and destination images must be the same size\n");
    assert(dst.frames == mask.frames && dst.width == mask.width && dst.height == mask.height,
           "The source and destination images must be the same size as the mask\n");

    assert(mask.channels == 1, "Mask must have one channel\n");

    // Generate a smooth patch to fix discontinuities between source and destination
    Image patch = Inpaint::apply(dst-src, mask);

    for (int c = 0; c < dst.channels; c++) {
	dst.channel(c).set((1-mask)*(src.channel(c) + patch.channel(c)) + mask*dst.channel(c));
    }
}

#include "footer.h"
