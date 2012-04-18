#include "main.h"
#include "Geometry.h"
#include "Stack.h"
#include "Arithmetic.h"
#include "header.h"

void Upsample::help() {
    pprintf("-upsample multiplies the width, height, and frames of the current"
            " image by the given integer arguments. It uses nearest neighbor"
            " interpolation. For a slower, high-quality resampling method, use"
            " -resample instead.\n\n"
            "-upsample x y is interpreted as -upsample x y 1\n"
            "-upsample x is interpreted as -upsample x x 1\n"
            "-upsample is interpreted as -upsample 2 2 1\n\n"
            "Usage: ImageStack -load a.tga -upsample 3 2 -save b.tga\n\n");
}

void Upsample::parse(vector<string> args) {
    int boxWidth = 2, boxHeight = 2, boxFrames = 1;
    assert(args.size() <= 3, "-upsample takes three or fewer arguments\n");
    if (args.size() == 3) {
        boxWidth = readInt(args[0]);
        boxHeight = readInt(args[1]);
        boxFrames = readInt(args[2]);
    } else if (args.size() == 2) {
        boxWidth = readInt(args[0]);
        boxHeight = readInt(args[1]);
    } else if (args.size() == 1) {
        boxWidth = boxHeight = readInt(args[0]);
    }

    NewImage im = apply(stack(0), boxWidth, boxHeight, boxFrames);
    pop();
    push(im);
}

NewImage Upsample::apply(NewImage im, int boxWidth, int boxHeight, int boxFrames) {

    NewImage out(im.width*boxWidth, im.height*boxHeight, im.frames*boxFrames, im.channels);

    for (int c = 0; c < im.channels; c++) {
        for (int t = 0; t < out.frames; t++) {
            int it = t / boxFrames;
            for (int y = 0; y < out.height; y++) {
                int iy = y / boxHeight;
                for (int x = 0; x < out.width; x++) {
                    int ix = x / boxWidth;
                    out(x, y, t, c) = im(ix, iy, it, c);
                }                
            }
        }
    }
    
    return out;

}

void Downsample::help() {
    pprintf("-downsample divides the width, height, and frames of the current image"
            " by the given integer arguments. It averages rectangles to get the new"
            " values.\n\n"
            "-downsample x y is interpreted as -downsample x y 1\n"
            "-downsample x is interpreted as -downsample x x 1\n"
            "-downsample is interpreted as -downsample 2 2 1\n\n"
            "Usage: ImageStack -load a.tga -downsample 3 2 -save b.tga\n\n");
}

void Downsample::parse(vector<string> args) {
    int boxWidth = 2, boxHeight = 2, boxFrames = 1;
    assert(args.size() <= 3, "-downsample takes three or fewer arguments\n");
    if (args.size() == 3) {
        boxWidth = readInt(args[0]);
        boxHeight = readInt(args[1]);
        boxFrames = readInt(args[2]);
    } else if (args.size() == 2) {
        boxWidth = readInt(args[0]);
        boxHeight = readInt(args[1]);
    } else if (args.size() == 1) {
        boxWidth = boxHeight = readInt(args[0]);
    }

    NewImage im = apply(stack(0), boxWidth, boxHeight, boxFrames);
    pop();
    push(im);
}

NewImage Downsample::apply(NewImage im, int boxWidth, int boxHeight, int boxFrames) {

    if (!((im.width % boxWidth == 0) && (im.height % boxHeight == 0) && (im.frames % boxFrames == 0))) {
        printf("Warning: Image dimensions are not a multiple of the downsample size. Ignoring some pixels.\n");        
    }

    int newWidth = im.width / boxWidth;
    int newHeight = im.height / boxHeight;
    int newFrames = im.frames / boxFrames;
    float scale = 1.0f / (boxWidth * boxHeight * boxFrames);

    NewImage out(newWidth, newHeight, newFrames, im.channels);

    for (int c = 0; c < out.channels; c++) {
        for (int t = 0; t < out.frames; t++) {
            for (int y = 0; y < out.height; y++) {
                for (int x = 0; x < out.width; x++) {
                    float val = 0;
                    for (int dt = 0; dt < boxFrames; dt++) {
                        for (int dy = 0; dy < boxHeight; dy++) {
                            for (int dx = 0; dx < boxWidth; dx++) {
                                val += im(x*boxWidth+dx, y*boxHeight+dy, t*boxFrames+dt, c);
                            }
                        }
                    }
                    out(x, y, t, c) = val * scale;
                }
            }
        }
    }

    return out;
}

void Resample::help() {
    printf("-resample resamples the input using a 3-lobed Lanczos filter. When"
           " given three arguments, it produces a new volume of the given width,"
           " height, and frames. When given two arguments, it produces a new volume"
           " of the given width and height, with the same number of frames.\n\n"
           "Usage: ImageStack -loadframes f*.tga -resample 20 50 50 -saveframes f%%03d.tga\n\n");
}

void Resample::parse(vector<string> args) {

    if (args.size() == 2) {
        NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]));
        pop();
        push(im);
    } else if (args.size() == 3) {
        NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]), readInt(args[2]));
        pop();
        push(im);
    } else {
        panic("-resample takes two or three arguments\n");
    }

}

NewImage Resample::apply(NewImage im, int width, int height) {
    if (height != im.height && width != im.width) {
        NewImage tmp = resampleY(im, height);
        return resampleX(tmp, width);
    } else if (width != im.width) {
        return resampleX(im, width);
    } else if (height != im.height) {
        return resampleY(im, height);
    }
    return im;
}

NewImage Resample::apply(NewImage im, int width, int height, int frames) {
    if (frames != im.frames) {
        NewImage tmp = resampleT(im, frames);
        return apply(tmp, width, height);
    } else {
        return apply(im, width, height);
    }
}

void Resample::computeWeights(int oldSize, int newSize, vector<vector<pair<int, float> > > &matrix) {
    assert(newSize > 0, "Can only resample to positive sizes");

    float filterWidth = max(1.0f, (float)oldSize / newSize);

    matrix.resize(newSize);

    for (int x = 0; x < newSize; x++) {
        // This x in the output corresponds to which x in the input?
        float inX = (x + 0.5f) / newSize * oldSize - 0.5f;                

        // Now compute a filter surrounding said x in the input
        int minX = ceilf(inX - filterWidth*3);
        int maxX = floorf(inX + filterWidth*3);
        minX = clamp(minX, 0, oldSize-1);
        maxX = clamp(maxX, 0, oldSize-1); 
        
        assert(minX < maxX, "Wha?");

        // Compute a row of the sparse matrix
        matrix[x].resize(maxX - minX + 1);
        float totalWeight = 0;
        for (int i = minX; i <= maxX; i++) {
            float delta = i - inX;
            float w = lanczos_3(delta/filterWidth);
            matrix[x][i - minX] = make_pair(i, w);
            totalWeight += w;
        }
        for (int i = 0; i <= maxX - minX; i++) {
            matrix[x][i].second /= totalWeight;
        }
    }
}

NewImage Resample::resampleX(NewImage im, int width) {

    vector<vector<pair<int, float> > > matrix;
    computeWeights(im.width, width, matrix);

    NewImage out(width, im.height, im.frames, im.channels);

    for (int c = 0; c < out.channels; c++) {
        for (int t = 0; t < out.frames; t++) {
            for (int y = 0; y < out.height; y++) {
                for (int x = 0; x < out.width; x++) {
                    float val = 0;
                    for (size_t i = 0; i < matrix[x].size(); i++) {
                        val += matrix[x][i].second * im(matrix[x][i].first, y, t, c); 
                    }
                    out(x, y, t, c) = val;
                }
            }
        }
    }

    return out;
}

NewImage Resample::resampleY(NewImage im, int height) {
    vector<vector<pair<int, float> > > matrix;
    computeWeights(im.height, height, matrix);

    NewImage out(im.width, height, im.frames, im.channels);

    for (int c = 0; c < out.channels; c++) {
        for (int t = 0; t < out.frames; t++) {
            for (int y = 0; y < out.height; y++) {
                for (int x = 0; x < out.width; x++) {
                    float val = 0;
                    for (size_t i = 0; i < matrix[y].size(); i++) {
                        val += matrix[y][i].second * im(x, matrix[y][i].first, t, c); 
                    }
                    out(x, y, t, c) = val;
                }
            }
        }
    }

    return out;
}

NewImage Resample::resampleT(NewImage im, int frames) {
    vector<vector<pair<int, float> > > matrix;
    computeWeights(im.frames, frames, matrix);

    NewImage out(im.width, im.height, frames, im.channels);

    for (int c = 0; c < out.channels; c++) {
        for (int t = 0; t < out.frames; t++) {
            for (int y = 0; y < out.height; y++) {
                for (int x = 0; x < out.width; x++) {
                    float val = 0;
                    for (size_t i = 0; i < matrix[t].size(); i++) {
                        val += matrix[t][i].second * im(x, y, matrix[t][i].first, c); 
                    }
                    out(x, y, t, c) = val;
                }
            }
        }
    }

    return out;
}



void Interleave::help() {
    pprintf("-interleave divides the image into n equally sized volumes and interleaves"
            " them. When given two arguments it operates on columns and rows. When"
            " given three arguments, it operates on columns, rows, and frames.\n\n"
            "Usage: ImageStack -load deck.exr -interleave 2 -save shuffled.exr\n\n");
}

void Interleave::parse(vector<string> args) {
    if (args.size() == 2) {
        apply(stack(0), readInt(args[0]), readInt(args[1]));
    } else if (args.size() == 3) {
        apply(stack(0), readInt(args[0]), readInt(args[1]), readInt(args[2]));
    } else {
        panic("-interleave takes one, two, or three arguments\n");
    }
}

void Interleave::apply(NewImage im, int rx, int ry, int rt) {
    assert(rt >= 1 && rx >= 1 && ry >= 1, "arguments to interleave must be strictly positive integers\n");

    // interleave in t
    if (rt != 1) {
        vector<float> tmp(im.frames);
        for (int c = 0; c < im.channels; c++) {
            for (int y = 0; y < im.height; y++) {
                for (int x = 0; x < im.width; x++) {
                    // copy out this chunk
                    for (int t = 0; t < im.frames; t++) {                        
                        tmp[t] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldT = 0;
                    for (int t = 0; t < im.frames; t++) {
                        im(x, y, oldT, c) = tmp[t];
                        oldT += rt;
                        if (oldT >= im.frames) oldT = (oldT % rt) + 1; 
                    }
                }
            }
        }
    }

    // interleave in x
    if (rx != 1) {
        vector<float> tmp(im.width);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        tmp[x] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldX = 0;
                    for (int x = 0; x < im.width; x++) {
                        im(oldX, y, t, c) = tmp[x];
                    }
                    oldX += rx;
                    if (oldX >= im.width) oldX = (oldX % rx) + 1;
                }
            }
        }
    }

    // interleave in y
    if (ry != 1) {
        vector<float> tmp(im.height);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int x = 0; x < im.width; x++) {
                    // copy out this chunk
                    for (int y = 0; y < im.height; y++) {
                        tmp[y] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldY = 0;
                    for (int y = 0; y < im.height; y++) {
                        im(x, oldY, t, c) = tmp[y];
                    }
                    oldY += ry;
                    if (oldY >= im.height) oldY = (oldY % ry) + 1;
                }
            }
        }
    }
}

void Deinterleave::help() {
    pprintf("-deinterleave collects every nth frame, column, and/or row of the image"
            " and tiles the resulting collections. When given two arguments it"
            " operates on columns and rows. When given three arguments, it operates"
            " on all columns, rows, and frames.\n\n"
            "Usage: ImageStack -load lf.exr -deinterleave 16 16 -save lftranspose.exr\n\n");
}

void Deinterleave::parse(vector<string> args) {
    if (args.size() == 2) {
        apply(stack(0), readInt(args[0]), readInt(args[1]));
    } else if (args.size() == 3) {
        apply(stack(0), readInt(args[0]), readInt(args[1]), readInt(args[2]));
    } else {
        panic("-deinterleave takes two or three arguments\n");
    }
}

void Deinterleave::apply(NewImage im, int rx, int ry, int rt) {
    assert(rt >= 1 && rx >= 1 && ry >= 1, "arguments to deinterleave must be strictly positive integers\n");

    // interleave in t
    if (rt != 1) {
        vector<float> tmp(im.frames);
        for (int c = 0; c < im.channels; c++) {
            for (int y = 0; y < im.height; y++) {
                for (int x = 0; x < im.width; x++) {
                    // copy out this chunk
                    for (int t = 0; t < im.frames; t++) {                        
                        tmp[t] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldT = 0;
                    for (int t = 0; t < im.frames; t++) {
                        im(x, y, t, c) = tmp[oldT];
                        oldT += rt;
                        if (oldT >= im.frames) oldT = (oldT % rt) + 1; 
                    }
                }
            }
        }
    }

    // interleave in x
    if (rx != 1) {
        vector<float> tmp(im.width);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        tmp[x] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldX = 0;
                    for (int x = 0; x < im.width; x++) {
                        im(x, y, t, c) = tmp[oldX];
                    }
                    oldX += rx;
                    if (oldX >= im.width) oldX = (oldX % rx) + 1;
                }
            }
        }
    }

    // interleave in y
    if (ry != 1) {
        vector<float> tmp(im.height);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int x = 0; x < im.width; x++) {
                    // copy out this chunk
                    for (int y = 0; y < im.height; y++) {
                        tmp[y] = im(x, y, t, c);
                    }

                    // paste this chunk back in in bizarro order
                    int oldY = 0;
                    for (int y = 0; y < im.height; y++) {
                        im(x, y, t, c) = tmp[oldY];
                    }
                    oldY += ry;
                    if (oldY >= im.height) oldY = (oldY % ry) + 1;
                }
            }
        }
    }
    
}


void Rotate::help() {
    printf("\n-rotate takes a number of degrees, and rotates every frame of the current image\n"
           "clockwise by that angle. The rotation preserves the image size, filling empty\n"
           " areas with zeros, and throwing away data which will not fit in the bounds.\n\n"
           "Usage: ImageStack -load a.tga -rotate 45 -save b.tga\n\n");
}


void Rotate::parse(vector<string> args) {
    assert(args.size() == 1, "-rotate takes one argument\n");
    NewImage im = apply(stack(0), readFloat(args[0]));
    pop();
    push(im);
}


NewImage Rotate::apply(NewImage im, float degrees) {

    // figure out the rotation matrix
    float radians = degrees * M_PI / 180;
    float cosine = cosf(radians);
    float sine = sinf(radians);
    float m00 = cosine;
    float m01 = sine;
    float m10 = -sine;
    float m11 = cosine;

    // locate the origin
    float xorigin = (im.width-1) * 0.5;
    float yorigin = (im.height-1) * 0.5;

    NewImage out(im.width, im.height, im.frames, im.channels);

    vector<float> sample(im.channels);

    for (int t = 0; t < im.frames; t++) {
        for (int y = 0; y < im.height; y++) {
            for (int x = 0; x < im.width; x++) {
                // figure out the sample location
                float fx = m00 * (x - xorigin) + m01 * (y - yorigin) + xorigin;
                float fy = m10 * (x - xorigin) + m11 * (y - yorigin) + yorigin;
                // don't sample outside the image
                if (fx < 0 || fx > im.width || fy < 0 || fy > im.height) {
                    for (int c = 0; c < im.channels; c++) 
                        out(x, y, t, c) = 0;
                } else {
                    im.sample2D(fx, fy, t, sample);
                    for (int c = 0; c < im.channels; c++) 
                        out(x, y, t, c) = sample[c];
                }
            }
        }
    }

    return out;

}


void AffineWarp::help() {
    printf("\n-affinewarp takes a 2x3 matrix in row major order, and performs that affine warp\n"
           "on the image.\n\n"
           "Usage: ImageStack -load a.jpg -affinewarp 0.9 0.1 0 0.1 0.9 0 -save out.jpg\n\n");
}

void AffineWarp::parse(vector<string> args) {
    assert(args.size() == 6, "-affinewarp takes six arguments\n");
    vector<double> matrix(6);
    for (int i = 0; i < 6; i++) { matrix[i] = readFloat(args[i]); }
    NewImage im = apply(stack(0), matrix);
    pop();
    push(im);
}

NewImage AffineWarp::apply(NewImage im, vector<double> matrix) {

    NewImage out(im.width, im.height, im.frames, im.channels);

    vector<float> sample(im.channels);
    for (int t = 0; t < im.frames; t++) {
        for (int y = 0; y < im.height; y++) {
            for (int x = 0; x < im.width; x++) {
                // figure out the sample location
                float fx = matrix[0] * x + matrix[1] * y + matrix[2] * im.width;
                float fy = matrix[3] * x + matrix[4] * y + matrix[5] * im.height;
                // don't sample outside the image
                if (fx < 0 || fx > im.width || fy < 0 || fy > im.height) {
                    for (int c = 0; c < im.channels; c++) 
                        out(x, y, t, c) = 0; 
                } else {
                    im.sample2D(fx, fy, t, sample);
                    for (int c = 0; c < im.channels; c++) 
                        out(x, y, t, c) = sample[c];
                }
            }
        }
    }

    return out;
}

void Crop::help() {
    pprintf("-crop takes either zero, two, four, or six arguments. The first half"
            " of the arguments are either minimum t, minimum x and y, or all three"
            " in the order x, y, t. The second half of the arguments are"
            " correspondingly either number of frames, width and height, or all"
            " three in the order width, height, frames. You may crop outside the"
            " bounds of the original image. Values there are assumed to be black. If"
            " no argument are given, ImageStack guesses how much to crop by trimming"
            " rows and columns that are all the same color as the top left"
            " pixel.\n\n"
            "Usage: ImageStack -loadframes f*.tga -crop 10 1 -save frame10.tga\n"
            "       ImageStack -load a.tga -crop 100 100 200 200 -save cropped.tga\n"
            "       ImageStack -loadframes f*.tga -crop 100 100 10 200 200 1\n"
            "                  -save frame10cropped.tga\n\n");
}

void Crop::parse(vector<string> args) {

    NewImage im;

    if (args.size() == 0) {
        im = apply(stack(0));
    } else if (args.size() == 2) {
        im = apply(stack(0),
                   0, 0, readInt(args[0]),
                   stack(0).width, stack(0).height, readInt(args[1]));
    } else if (args.size() == 4) {
        im = apply(stack(0),
                   readInt(args[0]), readInt(args[1]),
                   readInt(args[2]), readInt(args[3]));
    } else if (args.size() == 6) {
        im = apply(stack(0),
                   readInt(args[0]), readInt(args[1]), readInt(args[2]),
                   readInt(args[3]), readInt(args[4]), readInt(args[5]));
    } else {
        panic("-crop takes two, four, or six arguments.\n");
    }

    pop();
    push(im);
}

NewImage Crop::apply(NewImage im) {
    int minX, maxX, minY, maxY, minT, maxT;

    // calculate minX
    for (minX = 0; minX < im.width; minX++) {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {                    
                    if (im(minX, y, t, c) != im(0, 0, 0, c)) { goto minXdone; }
                }
            }
        }
    }
minXdone:

    // calculate maxX
    for (maxX = im.width-1; maxX >= 0; maxX--) {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    if (im(maxX, y, t, c) != im(0, 0, 0, c)) { goto maxXdone; }
                }
            }
        }
    }
maxXdone:

    // calculate minY
    for (minY = 0; minY < im.height; minY++) {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int x = 0; x < im.width; x++) {                    
                    if (im(x, minY, t, c) != im(0, 0, 0, c)) { goto minYdone; }
                }
            }
        }
    }
minYdone:

    // calculate maxY
    for (maxY = im.height-1; maxY >= 0; maxY--) {
        for (int t = 0; t < im.frames; t++) {
            for (int x = 0; x < im.width; x++) {
                for (int c = 0; c < im.channels; c++) {
                    if (im(x, maxY, t, c) != im(0, 0, 0, c)) { goto maxYdone; }
                }
            }
        }
    }
maxYdone:

    // calculate minT
    for (minT = 0; minT < im.frames; minT++) {
        for (int y = 0; y < im.height; y++) {
            for (int x = 0; x < im.width; x++) {
                for (int c = 0; c < im.channels; c++) {
                    if (im(x, y, minT, c) != im(0, 0, 0, c)) { goto minTdone; }
                }
            }
        }
    }
minTdone:

    // calculate maxT
    for (maxT = im.frames-1; maxT >= 0; maxT--) {
        for (int y = 0; y < im.height; y++) {
            for (int x = 0; x < im.width; x++) {
                for (int c = 0; c < im.channels; c++) {
                    if (im(x, y, maxT, c) != im(0, 0, 0, c)) { goto maxTdone; }
                }
            }
        }
    }
maxTdone:

    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    int frames = maxT - minT + 1;

    assert(width >= 0 && height >= 0 && frames >= 0, "Can't auto crop a blank image\n");

    return apply(im, minX, minY, minT, width, height, frames);

}

NewImage Crop::apply(NewImage im, int minX, int minY, int width, int height) {
    return apply(im,
                 minX, minY, 0,
                 width, height, im.frames);
}


NewImage Crop::apply(NewImage im, int minX, int minY, int minT,
                  int width, int height, int frames) {
    NewImage out(width, height, frames, im.channels);

    for (int c = 0; c < im.channels; c++) {
        for (int t = max(0, -minT); t < min(frames, im.frames - minT); t++) {
            for (int y = max(0, -minY); y < min(height, im.height - minY); y++) {
                for (int x = max(0, -minX); x < min(width, im.width - minX); x++) {                    
                    out(x, y, t, c) = im(x + minX, y + minY, t + minT, c);
                }
            }
        }
    }

    return out;
}

void Flip::help() {
    printf("-flip takes 'x', 'y', or 't' as the argument and flips the current image along\n"
           "that dimension.\n\n"
           "Usage: ImageStack -load a.tga -flip x -save reversed.tga\n\n");
}

void Flip::parse(vector<string> args) {
    assert(args.size() == 1, "-flip takes exactly one argument\n");
    char dimension = readChar(args[0]);
    apply(stack(0), dimension);
}

void Flip::apply(NewImage im, char dimension) {
    if (dimension == 't') {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames/2; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        swap(im(x, y, t, c), im(x, y, im.frames-t-1, c));
                    }
                }
            }
        }
    } else if (dimension == 'y') {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height/2; y++) {
                    for (int x = 0; x < im.width; x++) {
                        swap(im(x, y, t, c), im(x, im.height-1-y, t, c));
                    }
                }
            }
        }
    } else if (dimension == 'x') {
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width/2; x++) {
                        swap(im(x, y, t, c), im(im.width-1-x, y, t, c));
                    }
                }
            }
        }
    } else {
        panic("-flip only understands dimensions 'x', 'y', and 't'\n");
    }
}


void Adjoin::help() {
    printf("\n-adjoin takes 'x', 'y', 't', or 'c' as the argument, and joins the top two\n"
           "images along that dimension. The images must match in the other dimensions.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -adjoin x -save ab.tga\n\n");
}

void Adjoin::parse(vector<string> args) {
    assert(args.size() == 1, "-adjoin takes exactly one argument\n");
    char dimension = readChar(args[0]);
    NewImage im = apply(stack(1), stack(0), dimension);
    pop();
    pop();
    push(im);
}


NewImage Adjoin::apply(NewImage a, NewImage b, char dimension) {
    int newFrames = a.frames, newWidth = a.width, newHeight = a.height, newChannels = a.channels;
    int tOff = 0, xOff = 0, yOff = 0, cOff = 0;

    if (dimension == 't') {
        assert(a.width    == b.width &&
               a.height   == b.height &&
               a.channels == b.channels,
               "Cannot adjoin images that don't match in other dimensions\n");
        tOff = newFrames;
        newFrames += b.frames;
    } else if (dimension == 'y') {
        assert(a.width    == b.width &&
               a.frames   == b.frames &&
               a.channels == b.channels,
               "Cannot adjoin images that don't match in other dimensions\n");
        yOff = newHeight;
        newHeight += b.height;
    } else if (dimension == 'c') {
        assert(a.frames == b.frames &&
               a.height == b.height &&
               a.width  == b.width,
               "Cannot adjoin images that don't match in other dimensions\n");
        cOff = newChannels;
        newChannels += b.channels;
    } else if (dimension == 'x') {
        assert(a.frames == b.frames &&
               a.height == b.height &&
               a.channels  == b.channels,
               "Cannot adjoin images that don't match in other dimensions\n");
        xOff = newWidth;
        newWidth += b.width;
    } else {
        panic("-adjoin only understands dimensions 'x', 'y', and 't'\n");
    }

    NewImage out(newWidth, newHeight, newFrames, newChannels);
    // paste in the first image
    for (int c = 0; c < a.channels; c++) {
        for (int t = 0; t < a.frames; t++) {
            for (int y = 0; y < a.height; y++) {
                for (int x = 0; x < a.width; x++) {
                    out(x, y, t, c) = a(x, y, t, c);
                }
            }
        }
    }
    // paste in the second image
    for (int c = 0; c < b.channels; c++) {        
        for (int t = 0; t < b.frames; t++) {
            for (int y = 0; y < b.height; y++) {
                for (int x = 0; x < b.width; x++) {
                    out(x + xOff, y + yOff, t + tOff, c + cOff) = b(x, y, t, c);
                }
            }
        }
    }

    return out;
}

void Transpose::help() {
    printf("-transpose takes two dimension of the form 'x', 'y', or 't' and transposes\n"
           "the current image over those dimensions. If given no arguments, it defaults\n"
           "to x and y.\n\n"
           "Usage: ImageStack -load a.tga -transpose x y -flip x -save rotated.tga\n\n");
}

void Transpose::parse(vector<string> args) {
    assert(args.size() == 0 || args.size() == 2, "-transpose takes either zero or two arguments\n");
    if (args.size() == 0) {
        NewImage im = apply(stack(0), 'x', 'y');
        pop();
        push(im);
    } else {
        char arg1 = readChar(args[0]);
        char arg2 = readChar(args[1]);
        NewImage im = apply(stack(0), arg1, arg2);
        pop();
        push(im);
    }
}


NewImage Transpose::apply(NewImage im, char arg1, char arg2) {

    char dim1 = min(arg1, arg2);
    char dim2 = max(arg1, arg2);

    if (dim1 == 'c' && dim2 == 'y') {
        NewImage out(im.width, im.channels, im.frames, im.height);
        for (int c = 0; c < im.channels; c++) {                        
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        out(x, c, t, y) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;
    } else if (dim1 == 'c' && dim2 == 't') {
        NewImage out(im.width, im.height, im.channels, im.frames);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {                        
                        out(x, y, c, t) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;
    } else if (dim1 == 'c' && dim2 == 'x') {
        NewImage out(im.channels, im.height, im.frames, im.width);
        for (int c = 0; c < im.channels; c++) {                        
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        out(c, y, t, x) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;
    } else if (dim1 == 'x' && dim2 == 'y') {

        NewImage out(im.height, im.width, im.frames, im.channels);
        for (int c = 0; c < im.channels; c++) {                        
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        out(y, x, t, c) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;

    } else if (dim1 == 't' && dim2 == 'x') {
        NewImage out(im.frames, im.height, im.width, im.channels);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        out(t, y, x, c) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;
    } else if (dim1 == 't' && dim2 == 'y') {
        NewImage out(im.width, im.frames, im.height, im.channels);
        for (int c = 0; c < im.channels; c++) {
            for (int t = 0; t < im.frames; t++) {
                for (int y = 0; y < im.height; y++) {
                    for (int x = 0; x < im.width; x++) {
                        out(x, t, y, c) = im(x, y, t, c);
                    }
                }
            }
        }
        return out;
    } else {
        panic("-transpose only understands dimensions 'c', 'x', 'y', and 't'\n");
    }

    // keep compiler happy
    return NewImage();

}

void Translate::help() {
    pprintf("\n-translate moves the image data, leaving black borders. It takes two"
            " or three arguments. Two arguments are interpreted as a shift in x and"
            " a shift in y. Three arguments indicates a shift in x, y, and"
            " t. Negative values shift to the top left and positive ones to the"
            " bottom right. Fractional shifts are permitted; Lanczos sampling is"
            " used in this case.\n\n"
            "Usage: ImageStack -load in.jpg -translate -10 -10 -translate 20 20\n"
            "                  -translate -10 -10 -save in_border.jpg\n\n");
}

void Translate::parse(vector<string> args) {
    if (args.size() == 2) {
        NewImage im = apply(stack(0), readFloat(args[0]), readFloat(args[1]), 0);
        pop();
        push(im);
    } else if (args.size() == 3) {
        NewImage im = apply(stack(0), readFloat(args[0]), readFloat(args[1]), readFloat(args[2]));
        pop();
        push(im);
    } else {
        panic("-translate requires two or three arguments\n");
    }
}

NewImage Translate::apply(NewImage im, float xoff, float yoff, float toff) {
    NewImage current = im;
    NewImage out;

    // First do any non-integer translations
    if (xoff != floorf(xoff)) {
        out = applyX(im, xoff);
        current = out;
        xoff = 0;
    }

    if (yoff != floorf(yoff)) {
        out = applyY(current, yoff);
        current = out;
        yoff = 0;
    }

    if (toff != floorf(toff)) {
        out = applyT(current, toff);
        current = out;
        toff = 0;
    }

    // Now take care of the integer ones with a crop
    return Crop::apply(current, -xoff, -yoff, -toff, im.width, im.height, im.frames);
}

NewImage Translate::applyX(NewImage im, float xoff) {
    int ix = floorf(xoff);
    float fx = xoff - ix;
    // compute a 6-tap lanczos kernel
    float kernel[6];
    kernel[0] = lanczos_3(-3 + fx);
    kernel[1] = lanczos_3(-2 + fx);
    kernel[2] = lanczos_3(-1 + fx);
    kernel[3] = lanczos_3(0 + fx);
    kernel[4] = lanczos_3(1 + fx);
    kernel[5] = lanczos_3(2 + fx);

    NewImage out(im.width, im.height, im.frames, im.channels);
    for (int t = 0; t < im.frames; t++) {
        for (int y = 0; y < im.height; y++) {
            for (int x = 0; x < im.width; x++) {
                for (int kx = -3; kx < 3; kx++) {
                    int imx = x - ix + kx;
                    if (imx < 0) { continue; }
                    if (imx >= im.width) { continue; }
                    float w = kernel[kx+3];
                    for (int c = 0; c < im.channels; c++) {
                        out(x, y, t, c) += w*im(imx, y, t, c);
                    }
                }
            }
        }
    }
    return out;
}

NewImage Translate::applyY(NewImage im, float yoff) {
    int iy = floorf(yoff);
    float fy = yoff - iy;
    // compute a 6-tap lanczos kernel
    float kernel[6];
    kernel[0] = lanczos_3(-3 + fy);
    kernel[1] = lanczos_3(-2 + fy);
    kernel[2] = lanczos_3(-1 + fy);
    kernel[3] = lanczos_3(0 + fy);
    kernel[4] = lanczos_3(1 + fy);
    kernel[5] = lanczos_3(2 + fy);

    NewImage out(im.width, im.height, im.frames, im.channels);
    for (int t = 0; t < im.frames; t++) {
        for (int y = 0; y < im.height; y++) {
            for (int ky = -3; ky < 3; ky++) {
                int imy = y - iy + ky;
                if (imy < 0) { continue; }
                if (imy >= im.height) { continue; }
                float w = kernel[ky+3];
                for (int x = 0; x < im.width; x++) {
                    for (int c = 0; c < im.channels; c++) {
                        out(x, y, t, c) += w*im(x, imy, t, c);
                    }
                }
            }
        }
    }
    return out;
}

NewImage Translate::applyT(NewImage im, float toff) {
    int it = floorf(toff);
    float ft = toff - it;
    // compute a 6-tap lanczos kernel
    float kernel[6];
    kernel[0] = lanczos_3(-3 + ft);
    kernel[1] = lanczos_3(-2 + ft);
    kernel[2] = lanczos_3(-1 + ft);
    kernel[3] = lanczos_3(0 + ft);
    kernel[4] = lanczos_3(1 + ft);
    kernel[5] = lanczos_3(2 + ft);

    NewImage out(im.width, im.height, im.frames, im.channels);
    for (int t = 0; t < im.frames; t++) {
        for (int kt = -3; kt < 3; kt++) {
            int imt = t - it + kt;
            if (imt < 0) { continue; }
            if (imt >= im.frames) { continue; }
            float w = kernel[kt+3];
            for (int y = 0; y < im.height; y++) {
                for (int x = 0; x < im.width; x++) {
                    for (int c = 0; c < im.channels; c++) {
                        out(x, y, t, c) += w*im(x, y, imt, c);
                    }
                }
            }
        }
    }
    return out;
}

void Paste::help() {
    printf("-paste places some of the second image in the stack inside the top image, at\n"
           "the specified location. -paste accepts two or three, six, or nine arguments.\n"
           "When given two or three arguments, it interprets these as x and y, or x, y,\n"
           "and t, and pastes the whole of the second image onto that location in the first\n"
           "image. If six or nine arguments are given, the latter four or six arguments\n"
           "specify what portion of the second image is copied. The middle two or three\n"
           "arguments specify the top left, and the last two or three arguments specify\n"
           "the size of the region to paste.\n\n"
           "The format is thus: -paste [desination origin] [source origin] [size]\n\n"
           "Usage: ImageStack -load a.jpg -push 820 820 1 3 -paste 10 10 -save border.jpg\n\n");
}

void Paste::parse(vector<string> args) {
    int xdst = 0, ydst = 0, tdst = 0;
    int xsrc = 0, ysrc = 0, tsrc = 0;
    int width = stack(1).width;
    int height = stack(1).height;
    int frames = stack(1).frames;

    if (args.size() == 2) {
        xdst = readInt(args[0]);
        ydst = readInt(args[1]);
    } else if (args.size() == 3) {
        xdst = readInt(args[0]);
        ydst = readInt(args[1]);
        tdst = readInt(args[2]);
    } else if (args.size() == 6) {
        xdst = readInt(args[0]);
        ydst = readInt(args[1]);
        xsrc = readInt(args[2]);
        ysrc = readInt(args[3]);
        width = readInt(args[4]);
        height = readInt(args[5]);
    } else if (args.size() == 9) {
        xdst = readInt(args[0]);
        ydst = readInt(args[1]);
        tdst = readInt(args[2]);
        xsrc = readInt(args[3]);
        ysrc = readInt(args[4]);
        tsrc = readInt(args[5]);
        width  = readInt(args[6]);
        height = readInt(args[7]);
        frames = readInt(args[8]);
    } else {
        panic("-paste requires two, three, six, or nine arguments\n");
    }

    apply(stack(0), stack(1),
          xdst, ydst, tdst,
          xsrc, ysrc, tsrc,
          width, height, frames);
    pull(1);
    pop();

}


void Paste::apply(NewImage into, NewImage from,
                  int xdst, int ydst,
                  int xsrc, int ysrc,
                  int width, int height) {
    apply(into, from,
          xdst, ydst, 0,
          xsrc, ysrc, 0,
          width, height, from.frames);
}

void Paste::apply(NewImage into, NewImage from,
                  int xdst, int ydst, int tdst) {
    apply(into, from,
          xdst, ydst, tdst,
          0, 0, 0,
          from.width, from.height, from.frames);
}

void Paste::apply(NewImage into, NewImage from,
                  int xdst, int ydst, int tdst,
                  int xsrc, int ysrc, int tsrc,
                  int width, int height, int frames) {
    assert(into.channels == from.channels,
           "Images must have the same number of channels\n");
    assert(tdst >= 0 &&
           ydst >= 0 &&
           xdst >= 0 &&
           tdst + frames <= into.frames &&
           ydst + height <= into.height &&
           xdst + width  <= into.width,
           "Cannot paste outside the target image\n");
    assert(tsrc >= 0 &&
           ysrc >= 0 &&
           xsrc >= 0 &&
           tsrc + frames <= from.frames &&
           ysrc + height <= from.height &&
           xsrc + width  <= from.width,
           "Cannot paste from outside the source image\n");
    for (int c = 0; c < into.channels; c++) {                    
        for (int t = 0; t < frames; t++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    into(x + xdst, y + ydst, t + tdst, c) =
                        from(x + xsrc, y + ysrc, t + tsrc, c);
                }
            }
        }
    }
}

void Tile::help() {
    printf("\n-tile repeats the image along each dimension. It interprets two arguments as\n"
           "repetitions in x and y. Three arguments are interpreted as repetitions in x,\n"
           "y, and t.\n\n"
           "Usage: ImageStack -load a.tga -tile 2 2 -save b.tga\n\n");
}

void Tile::parse(vector<string> args) {
    int tRepeat = 1, xRepeat = 1, yRepeat = 1;
    if (args.size() == 2) {
        xRepeat = readInt(args[0]);
        yRepeat = readInt(args[1]);
    } else if (args.size() == 3) {
        xRepeat = readInt(args[0]);
        yRepeat = readInt(args[1]);
        tRepeat = readInt(args[2]);
    } else {
        panic("-tile takes two or three arguments\n");
    }
    NewImage im = apply(stack(0), xRepeat, yRepeat, tRepeat);
    pop();
    push(im);
}

NewImage Tile::apply(NewImage im, int xRepeat, int yRepeat, int tRepeat) {

    NewImage out(im.width * xRepeat, im.height * yRepeat, im.frames * tRepeat, im.channels);

    for (int c = 0; c < im.channels; c++) {                    
        for (int t = 0; t < im.frames * tRepeat; t++) {
            int imT = t % im.frames;
            for (int y = 0; y < im.height * yRepeat; y++) {
                int imY = y % im.height;
                for (int x = 0; x < im.width * xRepeat; x++) {
                    int imX = x % im.width;
                    out(x, y, t, c) = im(imX, imY, imT, c);
                }
            }
        }
    }

    return out;
}


void Subsample::help() {
    printf("\n-subsample subsamples the current image. Given two integer arguments, a and b,\n"
           "it selects one out of every a frames starting from frame b. Given four arguments,\n"
           "a, b, c, d, it selects one pixel out of every axb sized box, starting from pixel\n"
           "(c, d). Given six arguments, a, b, c, d, e, f, it selects one pixel from every\n"
           "axbxc volume, in the order width, height, frames starting at pixel (d, e, f).\n\n"
           "Usage: ImageStack -load in.jpg -subsample 2 2 0 0 -save smaller.jpg\n\n");
}

void Subsample::parse(vector<string> args) {
    if (args.size() == 2) {
        NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]));
        pop(); push(im);
    } else if (args.size() == 4) {
        NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]),
                         readInt(args[2]), readInt(args[3]));
        pop(); push(im);
    } else if (args.size() == 6) {
        NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]), readInt(args[2]),
                         readInt(args[3]), readInt(args[4]), readInt(args[5]));
        pop(); push(im);
    } else {
        panic("-subsample needs two, four, or six arguments\n");
    }
}

NewImage Subsample::apply(NewImage im, int boxFrames, int offsetT) {
    return apply(im, 1, 1, boxFrames, 0, 0, offsetT);
}

NewImage Subsample::apply(NewImage im, int boxWidth, int boxHeight,
                       int offsetX, int offsetY) {
    return apply(im, boxWidth, boxHeight, 1, offsetX, offsetY, 0);
}

NewImage Subsample::apply(NewImage im, int boxWidth, int boxHeight, int boxFrames,
                       int offsetX, int offsetY, int offsetT) {

    int newFrames = 0, newWidth = 0, newHeight = 0;
    for (int t = offsetT; t < im.frames; t += boxFrames) { newFrames++; }
    for (int x = offsetX; x < im.width;  x += boxWidth) { newWidth++; }
    for (int y = offsetY; y < im.height; y += boxHeight) { newHeight++; }

    NewImage out(newWidth, newHeight, newFrames, im.channels);

    for (int c = 0; c < im.channels; c++) {
        int outT = 0;
        for (int t = offsetT; t < im.frames; t += boxFrames) {
            int outY = 0;
            for (int y = offsetY; y < im.height; y += boxHeight) {
                int outX = 0;
                for (int x = offsetX; x < im.width; x += boxWidth) {
                    out(outX, outY, outT, c) = im(x, y, t, c);
                    outX++;
                }
                outY++;
            }
            outT++;
        }
    }

    return out;
}

void TileFrames::help() {
    printf("\n-tileframes takes a volume and lays down groups of frames in a grid, dividing\n"
           "the number of frames by the product of the arguments. It takes two arguments,\n"
           "the number of old frames across each new frame, and the number of frames down.\n"
           "each new frame. The first batch of frames will appear as the first row of the.\n"
           "first frame of the new volume.\n\n"
           "Usage: ImageStack -loadframes frame*.tif -tileframes 5 5 -saveframes sheet%%d.tif\n\n");
}

void TileFrames::parse(vector<string> args) {
    assert(args.size() == 2, "-tileframes takes two arguments\n");

    NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]));
    pop();
    push(im);
}

NewImage TileFrames::apply(NewImage im, int xTiles, int yTiles) {

    int newWidth = im.width * xTiles;
    int newHeight = im.height * yTiles;
    int newFrames = (int)(ceil((float)im.frames / (xTiles * yTiles)));

    NewImage out(newWidth, newHeight, newFrames, im.channels);

    for (int c = 0; c < im.channels; c++) {
        for (int t = 0; t < newFrames; t++) {
            int outY = 0;
            for (int yt = 0; yt < yTiles; yt++) {
                for (int y = 0; y < im.height; y++) {
                    int outX = 0;
                    for (int xt = 0; xt < xTiles; xt++) {
                        int imT = (t * yTiles + yt) * xTiles + xt;
                        if (imT >= im.frames) { break; }
                        for (int x = 0; x < im.width; x++) {                            
                            out(outX, outY, t, c) = im(x, y, imT, c);
                            outX++;
                        }
                    }
                    outY++;
                }
            }
        }
    }

    return out;
}

void FrameTiles::help() {
    printf("\n-frametiles takes a volume where each frame is a grid and breaks each frame\n"
           "into multiple frames, one per grid element. The two arguments specify the grid\n"
           "size. This operation is the inverse of tileframes.\n\n"
           "Usage: ImageStack -loadframes sheet*.tif -frametiles 5 5 -saveframes frame%%d.tif\n\n");
}

void FrameTiles::parse(vector<string> args) {
    assert(args.size() == 2, "-frametiles takes two arguments\n");

    NewImage im = apply(stack(0), readInt(args[0]), readInt(args[1]));
    pop();
    push(im);
}

NewImage FrameTiles::apply(NewImage im, int xTiles, int yTiles) {

    assert(im.width % xTiles == 0 &&
           im.height % yTiles == 0,
           "The image is not divisible by the given number of tiles\n");

    int newWidth = im.width / xTiles;
    int newHeight = im.height / yTiles;
    int newFrames = im.frames * xTiles * yTiles;

    NewImage out(newWidth, newHeight, newFrames, im.channels);

    for (int c = 0; c < im.channels; c++) {                            
        for (int t = 0; t < im.frames; t++) {
            int imY = 0;
            for (int yt = 0; yt < yTiles; yt++) {
                for (int y = 0; y < newHeight; y++) {
                    int imX = 0;
                    for (int xt = 0; xt < xTiles; xt++) {
                        int outT = (t * yTiles + yt) * xTiles + xt;
                        for (int x = 0; x < newWidth; x++) {
                            out(x, y, outT, c) = im(imX, imY, t, c);
                            imX++;
                        }
                    }
                    imY++;
                }
            }
        }
    }

    return out;
}


void Warp::help() {
    printf("\n-warp treats the top image of the stack as indices (within [0, 1]) into the\n"
           "second image, and samples the second image accordingly. It takes no arguments.\n"
           "The number of channels in the top image is the dimensionality of the warp, and\n"
           "should be three or less.\n\n"
           "Usage: ImageStack -load in.jpg -push -evalchannels \"X+Y\" \"Y\" -warp -save out.jpg\n\n");
}

void Warp::parse(vector<string> args) {
    assert(args.size() == 0, "warp takes no arguments\n");
    NewImage im = apply(stack(0), stack(1));
    pop();
    pop();
    push(im);
}

NewImage Warp::apply(NewImage coords, NewImage source) {

    NewImage out(coords.width, coords.height, coords.frames, source.channels);
    
    vector<float> sample(out.channels);
    if (coords.channels == 3) {
        for (int t = 0; t < coords.frames; t++) {
            for (int y = 0; y < coords.height; y++) {
                for (int x = 0; x < coords.width; x++) {
                    source.sample3D(coords(x, y, t, 0)*source.width,
                                    coords(x, y, t, 1)*source.height,
                                    coords(x, y, t, 2)*source.frames,
                                    sample);
                    for (int c = 0; c < out.channels; c++) 
                        out(x, y, t, c) = sample[c];
                }
            }
        }
    } else if (coords.channels == 2) {
        for (int t = 0; t < coords.frames; t++) {
            for (int y = 0; y < coords.height; y++) {
                for (int x = 0; x < coords.width; x++) {
                    source.sample2D(coords(x, y, t, 0)*source.width,
                                    coords(x, y, t, 1)*source.height,
                                    t, sample);
                    for (int c = 0; c < out.channels; c++) 
                        out(x, y, t, c) = sample[c];
                }
            }
        }
    } else {
        panic("index image must have two or three channels\n");
    }
    return out;
}



void Reshape::help() {
    printf("\n-reshape changes the way the memory of the current image is indexed. The four\n"
           "integer arguments specify a new width, height, frames, and channels.\n\n"
           "Usage: ImageStack -load movie.tmp -reshape width height*frames 1 channels\n"
           "                  -save filmstrip.tmp\n");
}

void Reshape::parse(vector<string> args) {
    assert(args.size() == 4, "-reshape takes four arguments\n");
    NewImage im = apply(stack(0),
                     readInt(args[0]), readInt(args[1]),
                     readInt(args[2]), readInt(args[3]));
    pop();
    push(im);

}

NewImage Reshape::apply(NewImage im, int x, int y, int t, int c) {
    assert(t * x * y * c == im.frames * im.width * im.height * im.channels,
           "New shape uses a different amount of memory that the old shape.\n");
    assert(im.dense(), "Input image is not densely packed in memory");
    NewImage out = im.copy();
    
    out.frames = t;
    out.width = x;
    out.height = y;
    out.channels = c;
    out.cstride = x*y*t;
    out.tstride = x*y;
    out.ystride = x;
    return out;
}



#include "footer.h"