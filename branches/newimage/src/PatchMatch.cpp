
// CS448F final project
// Implementation of PatchMatch algorithm and its applications
// Sung Hee Park (shpark7@stanford.edu)


#include "main.h"
#include "File.h"
#include "Geometry.h"
#include "PatchMatch.h"
#include "Arithmetic.h"
#include "Calculus.h"
#include "Statistics.h"
#include "Filter.h"
#include "Paint.h"
#include "Display.h"
#include "header.h"
// PATCHMATCH =============================================================//

void PatchMatch::help() {

    printf("-patchmatch computes approximate nearest neighbor field from the top\n"
           "image on the stack to the second image on the stack, using the\n"
           "algorithm from the PatchMatch SIGGRAPH 2009 paper. This operation\n"
           "requires two input images which may have multiple frames.\n"
           "It returns an image with four channels. First three channels \n"
           "correspond to x, y, t coordinate of closest patch and \n"
           "fourth channels contains the sum of squared differences \n"
           "between patches. \n"
           "\n"
           " arguments [numIter] [patchSize]\n"
           "  - numIter : number of iterations performed. (default: 5)\n"
           "  - patchSize : size of patch. (default: 7, 7x7 square patch)\n"
           " You can omit some arguments from right to use default values.\n"
           "\n"
           "Usage: ImageStack -load target.jpg -load source.jpg -patchmatch -save match.tmp\n\n");
}

void PatchMatch::parse(vector<string> args) {

    int numIter = 5, patchSize = 7;

    assert(args.size() <= 2, "-patchmatch takes two or fewer arguments\n");
    if (args.size() == 2) {
        numIter = readInt(args[0]);
        patchSize = (int) readInt(args[1]);
    } else if (args.size() == 1) {
        numIter = readInt(args[0]);
    }

    Image result;

    result = apply(stack(0), stack(1), numIter, patchSize);

    push(result);
}

Image PatchMatch::apply(Image source, Image target, int iterations, int patchSize) {
    return apply(source, target, Image(), iterations, patchSize);
}

Image PatchMatch::apply(Image source, Image target, Image mask, int iterations, int patchSize) {

    if (mask) {
        assert(target.width == mask.width &&
               target.height == mask.height &&
               target.frames == mask.frames,
               "Mask must have the same dimensions as the target\n");
        assert(mask.channels == 1,
               "Mask must have a single channel\n");
    }
    assert(iterations > 0, "Iterations must be a strictly positive integer\n");
    assert(patchSize >= 3 && (patchSize & 1), "Patch size must be at least 3 and odd\n");

    // convert patch diameter to patch radius
    patchSize /= 2;

    // For each source pixel, output a 3-vector to the best match in
    // the target, with an error as the last channel.
    Image out(source.width, source.height, source.frames, 4);

    // Iterate over source frames, finding a match in the target where
    // the mask is high

    for (int t = 0; t < source.frames; t++) {
        // INITIALIZATION - uniform random assignment
        for (int y = 0; y < source.height; y++) {
            for (int x = 0; x < source.width; x++) {
                int dx = randomInt(patchSize, target.width-patchSize-1);
                int dy = randomInt(patchSize, target.height-patchSize-1);
                int dt = randomInt(0, target.frames-1);
		out(x, y, t, 0) = dx;
		out(x, y, t, 1) = dy;
		out(x, y, t, 2) = dt;
		out(x, y, t, 3) = distance(source, target, mask,
					   t, x, y,
					   dt, dx, dy,
					   patchSize, HUGE_VAL);
            }
        }
    }

    bool forwardSearch = true;

    Image dx = out.channel(0), dy = out.channel(1), dt = out.channel(2), error = out.channel(3);

    for (int i = 0; i < iterations; i++) {

        //printf("Iteration %d\n", i);

        // PROPAGATION
        if (forwardSearch) {
            // Forward propagation - compare left, center and up
            for (int t = 0; t < source.frames; t++) {
                for (int y = 1; y < source.height; y++) {
                    for (int x = 1; x < source.width; x++) {
                        if (error(x, y, t, 0) > 0) {
                            float distLeft = distance(source, target, mask,
                                                      t, x, y,
						      dt(x-1, y, t, 0), 
						      dx(x-1, y, t, 0)+1, 
						      dy(x-1, y, t, 0),
                                                      patchSize, error(x, y, t, 0));

                            if (distLeft < error(x, y, t, 0)) {
				dx(x, y, t, 0) = dx(x-1, y, t, 0)+1;
				dy(x, y, t, 0) = dy(x-1, y, t, 0);
				dt(x, y, t, 0) = dt(x-1, y, t, 0);
				error(x, y, t, 0) = distLeft;
                            }

                            float distUp = distance(source, target, mask,
                                                    t, x, y,
						    dt(x, y-1, t, 0),
						    dx(x, y-1, t, 0),
						    dy(x, y-1, t, 0)+1,
                                                    patchSize, error(x, y, t, 0));
			    
                            if (distUp < error(x, y, t, 0)) {
				dx(x, y, t, 0) = dx(x, y-1, t, 0);
				dy(x, y, t, 0) = dy(x, y-1, t, 0)+1;
				dt(x, y, t, 0) = dt(x, y-1, t, 0);
				error(x, y, t, 0) = distUp;
                            }
                        }

                        // TODO: Consider searching across time as well

                    }
                }
            }

        } else {
            // Backward propagation - compare right, center and down
            for (int t = source.frames-1; t >= 0; t--) {
                for (int y = source.height-2; y >= 0; y--) {
		    for (int x = source.width-2; x >= 0; x--) {
                        if (error(x, y, t, 0) > 0) {
                            float distRight = distance(source, target, mask,
						       t, x, y,
						       dt(x+1, y, t, 0), 
						       dx(x+1, y, t, 0)-1, 
						       dy(x+1, y, t, 0),
						       patchSize, error(x, y, t, 0));

                            if (distRight < error(x, y, t, 0)) {
				dx(x, y, t, 0) = dx(x+1, y, t, 0)-1;
				dy(x, y, t, 0) = dy(x+1, y, t, 0);
				dt(x, y, t, 0) = dt(x+1, y, t, 0);
				error(x, y, t, 0) = distRight;
                            }

                            float distDown = distance(source, target, mask,
						      t, x, y,
						      dt(x, y+1, t, 0),
						      dx(x, y+1, t, 0),
						      dy(x, y+1, t, 0)-1,
						      patchSize, error(x, y, t, 0));
			    
                            if (distDown < error(x, y, t, 0)) {
				dx(x, y, t, 0) = dx(x, y+1, t, 0);
				dy(x, y, t, 0) = dy(x, y+1, t, 0)-1;
				dt(x, y, t, 0) = dt(x, y+1, t, 0);
				error(x, y, t, 0) = distDown;
                            }
                        }
			
                        // TODO: Consider searching across time as well
			
                    }
                }
            }
        }

        forwardSearch = !forwardSearch;

        // RANDOM SEARCH
        for (int t = 0; t < source.frames; t++) {
            for (int y = 0; y < source.height; y++) {
                for (int x = 0; x < source.width; x++) {

		    if (error(x, y, t, 0) > 0) {

                        int radius = target.width > target.height ? target.width : target.height;

                        // search an exponentially smaller window each iteration
                        while (radius > 8) {
                            // Search around current offset vector (distance-weighted)
			    
                            // clamp the search window to the image
                            int minX = (int)dx(x, y, t, 0) - radius;
                            int maxX = (int)dx(x, y, t, 0) + radius + 1;
                            int minY = (int)dy(x, y, t, 0) - radius;
                            int maxY = (int)dy(x, y, t, 0) + radius + 1;
                            if (minX < 0) { minX = 0; }
                            if (maxX > target.width) { maxX = target.width; }
                            if (minY < 0) { minY = 0; }
                            if (maxY > target.height) { maxY = target.height; }

                            int randX = rand() % (maxX - minX) + minX;
                            int randY = rand() % (maxY - minY) + minY;
                            int randT = rand() % target.frames;
                            float dist = distance(source, target, mask,
                                                  t, x, y,
                                                  randT, randX, randY,
                                                  patchSize, error(x, y, t, 0));
                            if (dist < error(x, y, t, 0)) {
                                dx(x, y, t, 0) = randX;
                                dy(x, y, t, 0) = randY;
                                dt(x, y, t, 0) = randT;
                                error(x, y, t, 0) = dist;
                            }

                            radius >>= 1;

                        }
                    }
                }
            }
        }
    }

    return out;
}

float PatchMatch::distance(Image source, Image target, Image mask,
                           int st, int sx, int sy,
                           int tt, int tx, int ty,
                           int patchSize, float prevDist) {

    // Do not use patches on boundaries
    if (tx < patchSize || tx >= target.width-patchSize ||
        ty < patchSize || ty >= target.height-patchSize) {
        return HUGE_VAL;
    }

    // Compute distance between patches
    // Average L2 distance in RGB space
    float dist = 0;
    float weight = 0;

    float threshold = prevDist*target.channels*(2*patchSize+1)*(2*patchSize+1);

    int x1 = max(-patchSize, -sx, -tx);
    int x2 = min(patchSize, -sx+source.width-1, -tx+target.width-1);
    int y1 = max(-patchSize, -sy, -ty);
    int y2 = min(patchSize, -sy+source.height-1, -ty+target.height-1);

    /*
    int x1 = -patchSize, x2 = patchSize;
    int y1 = -patchSize, y2 = patchSize;
    */


    for (int c = 0; c < target.channels; c++) {
	for (int y = y1; y <= y2; y++) {	   	
	    for (int x = x1; x <= x2; x++) {
		float w = mask ? mask(tx+x, ty+y, tt, 0) : 1;
		assert(w >= 0, "Negative w %f\n", w);
		
		float delta = source(sx+x, sy+y, st, c) - target(tx+x, ty+y, tt, c);
		dist += w * delta * delta;
		weight += w;
		
		// Early termination
		if (dist > threshold) {return HUGE_VAL;}
	    }
        }
    }

    assert(dist >= 0, "negative dist\n");
    assert(weight >= 0, "negative weight\n");

    if (!weight) { return HUGE_VAL; }

    return dist / weight;
}


// BIDIRECTIONAL SIMILARITY =====================================================//


void BidirectionalSimilarity::help() {
    pprintf("-bidirectionalsimilarity reconstructs the top image on the stack using"
            " patches from the second image on the stack, by enforcing coherence"
            " (every patch in the output must look like a patch from the input) and"
            " completeness (every patch from the input must be represented somewhere"
            " in the output). The first argument is a number between zero and one,"
            " which trades off between favoring coherence only (at zero), and"
            " completeness only (at one). It defaults to 0.5. The second arguments"
            " specifies the number of iterations that should be performed, and"
            " defaults to five. Bidirectional similarity uses patchmatch as the"
            " underlying nearest-neighbour-field algorithm, and the third argument"
            " specifies how many iterations of patchmatch should be performed each"
            " time it is run. This also defaults to five.\n"
            "\n"
            "This is an implementation of the paper \"Summarizing visual data using"
            " bidirectional similarity\" by Simakov et al. from CVPR 2008.\n"
            "\n"
            "Usage: ImageStack -load source.jpg -load target.jpg -bidirectional 0.5 -display\n");
}

void BidirectionalSimilarity::parse(vector<string> args) {

    float alpha = 0.5;
    int numIter = 5;
    int numIterPM = 5;

    assert(args.size() <= 3, "-bidirectional takes three or fewer arguments\n");
    if (args.size() == 3) {
        alpha = readFloat(args[0]);
        numIter = readFloat(args[1]);
        numIterPM = readFloat(args[2]);
    } else if (args.size() == 2) {
        alpha = readFloat(args[0]);
        numIter = readFloat(args[1]);
    } else if (args.size() == 1) {
        alpha = readFloat(args[0]);
    }

    apply(stack(1), stack(0), Image(), Image(), alpha, numIter, numIterPM);
}


// Reconstruct the portion of the target where the mask is high, using
// the portion of the source where its mask is high. Source and target
// masks are allowed to be null windows.
void BidirectionalSimilarity::apply(Image source, Image target,
                                    Image sourceMask, Image targetMask,
                                    float alpha, int numIter, int numIterPM) {



    // TODO: intelligently crop the input to where the mask is high +
    // patch radius on each side


    // recurse
    if (source.width > 32 && source.height > 32 && target.width > 32 && target.height > 32) {
        Image smallSource = Resample::apply(source, source.width/2, source.height/2, source.frames);
        Image smallTarget = Resample::apply(target, target.width/2, target.height/2, target.frames);

        Image smallSourceMask;
        Image smallTargetMask;
        if (sourceMask) {
            smallSourceMask = Downsample::apply(sourceMask, 2, 2, 1);
        }

        if (targetMask) {
            smallTargetMask = Downsample::apply(targetMask, 2, 2, 1);
        }

        apply(smallSource, smallTarget, smallSourceMask, smallTargetMask, alpha, numIter, numIterPM);

        Image newTarget = Resample::apply(smallTarget, target.width, target.height, target.frames);

        if (targetMask) {
            Composite::apply(target, newTarget, targetMask);
        } else {
	    newTarget = target.copy();
        }
    }

    printf("%dx%d ", target.width, target.height); fflush(stdout);
    for (int i = 0; i < numIter; i++) {
        printf("."); fflush(stdout);

        int patchSize = 5;
        Image completeMatch, coherentMatch;

        // The homogeneous output for this iteration
        Image out(target.width, target.height, target.frames, target.channels+1);

        if (alpha != 0) {

            // COMPLETENESS TERM
            Image completeMatch = PatchMatch::apply(source, target, targetMask, numIterPM, patchSize);

            // For every patch in the source, splat it onto the
            // nearest match in the target, weighted by the source
            // mask and also by the inverse of the patch distance
            for (int t = 0; t < source.frames; t++) {
                for (int y = 0; y < source.height; y++) {
                    for (int x = 0; x < source.width; x++) {

                        if (!sourceMask || sourceMask(x, y, t, 0) > 0) {

                            int dstX = (int)completeMatch(x, y, t, 0);
                            int dstY = (int)completeMatch(x, y, t, 1);
                            int dstT = (int)completeMatch(x, y, t, 2);
                            float weight = 1.0f/(completeMatch(x, y, t, 3) + 1);

                            if (sourceMask) { weight *= sourceMask(x, y, t, 0); }

                            for (int dy = -patchSize/2; dy <= patchSize/2; dy++) {
                                if (y+dy < 0) { continue; }
                                if (y+dy >= source.height) { break; }
                                for (int dx = -patchSize/2; dx <= patchSize/2; dx++) {
                                    if (x+dx < 0) continue;
				    if (x+dx >= source.width) break;

				    for (int c = 0; c < source.channels; c++) {
					out(dstX+dx, dstY+dy, dstT, c) += weight*source(x+dx, y+dy, t, c);
				    }
				    out(dstX+dx, dstY+dy, dstT, source.channels) += weight;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (alpha != 1) {
            // COHERENCE TERM
            Image coherentMatch = PatchMatch::apply(target, source, sourceMask,
						       numIterPM, patchSize);
            // For every patch in the target, pull from the nearest match in the source
            for (int t = 0; t < target.frames; t++) {
                for (int y = 0; y < target.height; y++) {
                    for (int x = 0; x < target.width; x++) {

                        if (!targetMask || targetMask(x, y, t, 0) > 0) {

                            int dstX = (int)coherentMatch(x, y, t, 0);
                            int dstY = (int)coherentMatch(x, y, t, 1);
                            int dstT = (int)coherentMatch(x, y, t, 2);
                            float weight = 1.0f/(coherentMatch(x, y, t, 3)+1);

                            if (targetMask) { weight *= targetMask(x, y, t, 0); }

                            for (int dy = -patchSize/2; dy <= patchSize/2; dy++) {
                                if (y+dy < 0) { continue; }
                                if (y+dy >= out.height) { break; }
                                for (int dx = -patchSize/2; dx <= patchSize/2; dx++) {
                                    if (x+dx < 0) continue;
				    if (x+dx >= out.width) break;
				    for (int c = 0; c < source.channels; c++) {
					out(x+dx, y+dy, t, c) += weight*source(dstX+dx, dstY+dy, dstT, c);
				    }
				    out(x+dx, y+dy, t, source.channels) += weight;
                                }
                            }
                        }
                    }
                }
            }
        }

        // rewrite the target using the homogeneous output
        for (int t = 0; t < out.frames; t++) {
            for (int y = 0; y < out.height; y++) {
                for (int x = 0; x < out.width; x++) {
                    float w = 1.0f/(out(x, y, t, target.channels));
                    float a = 1;
                    if (targetMask) {
                        a = targetMask(x, y, t, 0);
                    }
                    if (a == 1) {
                        for (int c = 0; c < target.channels; c++) {
			    target(x, y, t, c) = w*out(x, y, t, c);
                        }
                    } else if (a > 0) {
                        for (int c = 0; c < target.channels; c++) {
			    target(x, y, t, c) *= 1-a;
			    target(x, y, t, c) += a*w*out(x, y, t, c);
                        }
                    }
                }
            }
        }
    }
    printf("\n");
}

void Heal::help() {
    printf("-heal takes an image and a mask, and reconstructs the portion of"
           " the image where the mask is high using patches from the rest of the"
           " image. It uses the patchmatch algorithm for acceleration. The"
           " arguments include the number of iterations to run per scale, and the"
           " number of iterations of patchmatch to run. Both default to five.\n"
           "\n"
           "Usage: ImageStack -load mask.png -load image.jpg -heal -display\n");
}

void Heal::parse(vector<string> args) {
    int numIter = 5;
    int numIterPM = 5;

    assert(args.size() < 3, "-heal takes zero, one, or two arguments\n");

    Image mask = stack(1);
    Image image = stack(0);

    Image inverseMask = mask.copy();
    inverseMask *= -1;
    inverseMask += 1;

    if (args.size() > 0) { numIter = readInt(args[0]); }
    if (args.size() > 1) { numIterPM = readInt(args[1]); }

    BidirectionalSimilarity::apply(image, image, inverseMask, mask, 0, numIter, numIterPM);
}
#include "footer.h"
