#ifndef IMAGESTACK_IMAGE_H
#define IMAGESTACK_IMAGE_H

#include "tables.h"
#include "header.h"

class NewImage {
  public:
    NewImage() {
        init(0, 0, 0, 0);
    }

    NewImage(int w, int h) {
        init(w, h, 1, 1);
    }

    NewImage(int w, int h, int c) {
        init(w, h, 1, c);
    }

    NewImage(int w, int h, int f, int c) {
        init(w, h, f, c);
    }

    float &operator()(int x, int y) {
        return base[x + y*ystride];
    }

    float &operator()(int x, int y, int c) {
        return base[x + y*ystride + c*cstride];
    }

    float &operator()(int x, int y, int t, int c) {
        return base[x + y*ystride + t*tstride + c*cstride];
    }

    const float &operator()(int x, int y) const {
        return base[x + y*ystride];
    }

    const float &operator()(int x, int y, int c) const {
        return base[x + y*ystride + c*cstride];
    }

    const float &operator()(int x, int y, int t, int c) const {
        return base[x + y*ystride + t*tstride + c*cstride];
    }

    float *baseAddress() {
        return base;
    }

    NewImage copy() {
        NewImage m(width, height, frames, channels);
	m.copyFrom(*this);
        return m;
    }

    void copyFrom(NewImage other) {
	assert(other.width == width &&
	       other.height == height &&
	       other.frames == frames &&
	       other.channels == channels,
	       "Can only copy from images with matching dimensions\n");	
	for (int c = 0; c < channels; c++) {
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) = other(x, y, t, c);
		    }
		}
	    }
	}
    }

    void copyTo(NewImage other) {
	other.copyFrom(*this);
    }

    NewImage region(int x, int y, int t, int c,
                    int width, int height, int frames, int channels) {
        NewImage im;
        im.data = data;
        im.base = &((*this)(x, y, t, c));
        im.width = width;
        im.height = height;
        im.frames = frames;
        im.channels = channels;
        im.cstride = cstride;
        im.tstride = tstride;
        im.ystride = ystride;
        return im;
    }

    NewImage column(int x) {
        return region(x, 0, 0, 0, 1, height, frames, channels);
    }

    NewImage row(int y) {
        return region(0, y, 0, 0, width, 1, frames, channels);
    }

    NewImage frame(int t) {
        return region(0, 0, t, 0, width, height, 1, channels);
    }
    
    NewImage channel(int c) {
        return region(0, 0, 0, c, width, height, frames, 1);
    }

    bool dense() {
        return (cstride == width*height*frames && tstride == width*height && ystride == width);
    }

    int frames, width, height, channels;
    int ystride, tstride, cstride;
    
    operator bool() {
        return (base != NULL);
    }


    void operator+=(float f) {
	for (int c = 0; c < channels; c++) {
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) += f;
		    }
		}
	    }
	}
    }

    void operator*=(float f) {
	for (int c = 0; c < channels; c++) {
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) *= f;
		    }
		}
	    }
	}
    }

    void operator-=(float f) {
	(*this) += -f;
    }

    void operator/=(float f) {
	(*this) *= 1.0f/f;
    }

    void operator=(float f) {
	for (int c = 0; c < channels; c++) {
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) = f;
		    }
		}
	    }
	}
    }

    void operator+=(vector<float> f) {	
	for (int c = 0; c < channels; c++) {
	    channel(c) += f[c % f.size()];
	}
    }

    void operator*=(vector<float> f) {
	for (int c = 0; c < channels; c++) {
	    channel(c) *= f[c % f.size()];
	}
    }

    void operator-=(vector<float> f) {	
	for (int c = 0; c < channels; c++) {
	    channel(c) -= f[c % f.size()];
	}
    }

    void operator/=(vector<float> f) {
	for (int c = 0; c < channels; c++) {
	    channel(c) /= f[c % f.size()];
	}
    }

    void operator=(vector<float> f) {
	for (int c = 0; c < channels; c++) {
	    channel(c) = f[c % f.size()];
	}
    }

    void operator+=(NewImage other) {
	assert(other.width == width &&
	       other.height == height &&
	       other.frames == frames,
	       "Can only add images with matching dimensions\n");
	assert(other.channels == channels || other.channels == 1, 
	       "Can only add image with matching channel count, or single channel\n");	
	for (int c = 0; c < channels; c++) {
	    int co = c % other.channels;
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) += other(x, y, t, co);
		    }
		}
	    }
	}
    }

    void operator*=(NewImage other) {
	assert(other.width == width &&
	       other.height == height &&
	       other.frames == frames,
	       "Can only multiply images with matching dimensions\n");
	assert(other.channels == channels || other.channels == 1, 
	       "Can only multiply image with matching channel count, or single channel\n");	
	for (int c = 0; c < channels; c++) {
	    int co = c % other.channels;
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) *= other(x, y, t, co);
		    }
		}
	    }
	}
    }

    void operator-=(NewImage other) {
	assert(other.width == width &&
	       other.height == height &&
	       other.frames == frames,
	       "Can only subtract images with matching dimensions\n");
	assert(other.channels == channels || other.channels == 1, 
	       "Can only subtract image with matching channel count, or single channel\n");	
	for (int c = 0; c < channels; c++) {
	    int co = c % other.channels;
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) -= other(x, y, t, co);
		    }
		}
	    }
	}
    }

    void operator/=(NewImage other) {
	assert(other.width == width &&
	       other.height == height &&
	       other.frames == frames,
	       "Can only divide images with matching dimensions\n");
	assert(other.channels == channels || other.channels == 1, 
	       "Can only divide image with matching channel count, or single channel\n");	
	for (int c = 0; c < channels; c++) {
	    int co = c % other.channels;
	    for (int t = 0; t < frames; t++) {
		for (int y = 0; y < height; y++) {
		    for (int x = 0; x < width; x++) {
			(*this)(x, y, t, c) /= other(x, y, t, co);
		    }
		}
	    }
	}
    }

    typedef enum {ZERO = 0, NEUMANN} BoundaryCondition;

    void sample2D(float fx, float fy, int t, vector<float> &result, BoundaryCondition boundary = ZERO) {
        int ix = (int)fx;
        int iy = (int)fy;
        const int LEFT = -2;
        const int RIGHT = 3;
        const int WIDTH = 6;
        int minX = ix + LEFT;
        int maxX = ix + RIGHT;
        int minY = iy + LEFT;
        int maxY = iy + RIGHT;

        float weightX[WIDTH];
        float weightY[WIDTH];
        float totalXWeight = 0, totalYWeight = 0;
        for (int x = 0; x < WIDTH; x++) {
            float diff = (fx - (x + ix + LEFT)); // ranges between +/- RIGHT
            float val = lanczos_3(diff);
            weightX[x] = val;
            totalXWeight += val;
        }

        for (int y = 0; y < WIDTH; y++) {
            float diff = (fy - (y + iy + LEFT)); // ranges between +/- RIGHT
            float val = lanczos_3(diff);
            weightY[y] = val;
            totalYWeight += val;
        }

        totalXWeight = 1.0f/totalXWeight;
        totalYWeight = 1.0f/totalYWeight;

        for (int i = 0; i < WIDTH; i++) {
            weightX[i] *= totalXWeight;
            weightY[i] *= totalYWeight;
        }

        for (int c = 0; c < channels; c++) {
            result[c] = 0;
        }

        if (boundary == NEUMANN) {

            float *yWeightPtr = weightY;
            for (int y = minY; y <= maxY; y++) {
                float *xWeightPtr = weightX;
                int sampleY = clamp(0, y, height-1);
                for (int x = minX; x <= maxX; x++) {
                    int sampleX = clamp(0, x, width-1);
                    float yxWeight = (*yWeightPtr) * (*xWeightPtr);
                    for (int c = 0; c < channels; c++) {
			result[c] += (*this)(sampleX, sampleY, t, c) * yxWeight;
                    }
                    xWeightPtr++;
                }
                yWeightPtr++;
            }
        } else {
            float *weightYBase = weightY;
            float *weightXBase = weightX;
            if (minY < 0) {
                weightYBase -= minY;
                minY = 0;
            }
            if (minX < 0) {
                weightXBase -= minX;
                minX = 0;
            }
            if (maxX > width-1) { maxX = width-1; }
            if (maxY > height-1) { maxY = height-1; }
            float *yWeightPtr = weightYBase;
            for (int y = minY; y <= maxY; y++) {
                float *xWeightPtr = weightXBase;
                for (int x = minX; x <= maxX; x++) {
                    float yxWeight = (*yWeightPtr) * (*xWeightPtr);
                    for (int c = 0; c < channels; c++) {
                        result[c] += (*this)(x, y, t, c) * yxWeight;
                    }
                    xWeightPtr++;
                }
                yWeightPtr++;
            }

        }
    }

    void sample2D(float fx, float fy, vector<float> &result) {
        sample2D(fx, fy, 0, result);
    }


    void sample2DLinear(float fx, float fy, vector<float> &result) {
        sample2DLinear(fx, fy, 0, result);
    }

    void sample2DLinear(float fx, float fy, int t, vector<float> &result) {
        int ix = (int)fx;
        int iy = (int)fy;
        fx -= ix;
        fy -= iy;

        for (int c = 0; c < channels; c++) {
            float s1 = (1-fx) * (*this)(ix, iy, t, c) + fx * (*this)(ix+1, iy, t, c);
            float s2 = (1-fx) * (*this)(ix, iy+1, t, c) + fx * (*this)(ix+1, iy+1, t, c);
            result[c] = (1-fy) * s1 + fy * s2;
        }

    }

    void sample3DLinear(float fx, float fy, float ft, vector<float> &result) {
        int ix = (int)fx;
        int iy = (int)fy;
        int it = (int)ft;
        fx -= ix;
        fy -= iy;
        ft -= it;

        for (int c = 0; c < channels; c++) {
            float s11 = (1-fx) * (*this)(ix, iy, it, c) + fx * (*this)(ix+1, iy, it, c);
	    float s12 = (1-fx) * (*this)(ix, iy+1, it, c) + fx * (*this)(ix+1, iy+1, it, c);
            float s1 = (1-fy) * s11 + fy * s12;

            float s21 = (1-fx) * (*this)(ix, iy, it+1, c) + fx * (*this)(ix+1, iy, it+1, c);
            float s22 = (1-fx) * (*this)(ix, iy+1, it+1, c) + fx * (*this)(ix+1, iy+1, it+1, c);
            float s2 = (1-fy) * s21 + fy * s22;

            result[c] = (1-ft) * s1 + ft * s2;
        }

    }

    void sample3D(float fx, float fy, float ft, vector<float> &result, BoundaryCondition boundary = ZERO) {
        int ix = (int)fx;
        int iy = (int)fy;
        int it = (int)ft;
        const int LEFT = -2;
        const int RIGHT = 3;
        const int WIDTH = 6;
        int minX = ix + LEFT;
        int maxX = ix + RIGHT;
        int minY = iy + LEFT;
        int maxY = iy + RIGHT;
        int minT = it + LEFT;
        int maxT = it + RIGHT;
        float weightX[WIDTH];
        float weightY[WIDTH];
        float weightT[WIDTH];

        float totalXWeight = 0, totalYWeight = 0, totalTWeight = 0;

        for (int x = 0; x < WIDTH; x++) {
            float diff = (fx - (x + ix + LEFT)); // ranges between +/- RIGHT
            float val = lanczos_3(diff);
            weightX[x] = val;
            totalXWeight += val;
        }

        for (int y = 0; y < WIDTH; y++) {
            float diff = (fy - (y + iy + LEFT)); // ranges between +/- RIGHT
            float val = lanczos_3(diff);
            weightY[y] = val;
            totalYWeight += val;
        }

        for (int t = 0; t < WIDTH; t++) {
            float diff = (ft - (t + it + LEFT)); // ranges between +/- RIGHT
            float val = lanczos_3(diff);
            weightT[t] = val;
            totalTWeight += val;
        }

        totalXWeight = 1.0f/totalXWeight;
        totalYWeight = 1.0f/totalYWeight;
        totalTWeight = 1.0f/totalTWeight;

        for (int i = 0; i < WIDTH; i++) {
            weightX[i] *= totalXWeight;
            weightY[i] *= totalYWeight;
            weightT[i] *= totalTWeight;
        }

        for (int c = 0; c < channels; c++) {
            result[c] = 0;
        }

        if (boundary == NEUMANN) {

            float *tWeightPtr = weightT;
            for (int t = minT; t <= maxT; t++) {
                int sampleT = clamp(t, 0, frames-1);
                float *yWeightPtr = weightY;
                for (int y = minY; y <= maxY; y++) {
                    int sampleY = clamp(y, 0, height-1);
                    float tyWeight = (*yWeightPtr) * (*tWeightPtr);
                    float *xWeightPtr = weightX;
                    for (int x = minX; x <= maxX; x++) {
                        int sampleX = clamp(x, 0, width-1);
                        float tyxWeight = tyWeight * (*xWeightPtr);
                        for (int c = 0; c < channels; c++) {
                            result[c] += (*this)(sampleX, sampleY, sampleT, c) * tyxWeight;
                        }
                        xWeightPtr++;
                    }
                    yWeightPtr++;
                }
                tWeightPtr++;
            }

        } else {

            float *weightTBase = weightT;
            float *weightYBase = weightY;
            float *weightXBase = weightX;

            if (minY < 0) {
                weightYBase -= minY;
                minY = 0;
            }
            if (minX < 0) {
                weightXBase -= minX;
                minX = 0;
            }
            if (minT < 0) {
                weightTBase -= minT;
                minT = 0;
            }
            if (maxX > width-1) { maxX = width-1; }
            if (maxY > height-1) { maxY = height-1; }
            if (maxT > frames-1) { maxT = frames-1; }

            float *tWeightPtr = weightTBase;
            for (int t = minT; t <= maxT; t++) {
                float *yWeightPtr = weightYBase;
                for (int y = minY; y <= maxY; y++) {
                    float *xWeightPtr = weightXBase;
                    for (int x = minX; x <= maxX; x++) {
                        float yxWeight = (*yWeightPtr) * (*xWeightPtr);
                        for (int c = 0; c < channels; c++) {
                            result[c] += (*this)(x, y, t, c) * yxWeight;
                        }
                        xWeightPtr++;
                    }
                    yWeightPtr++;
                }
                tWeightPtr++;
            }
        }

    }


  private:

    void init(int w, int h, int f, int c) {
        width = w;
        height = h;
        frames = f;
        channels = c;

        cstride = width*height*frames;
        tstride = width*height;
        ystride = width;

        if (w*h*f*c) {
            data.reset(new vector<float>(w*h*f*c+3));
            base = &((*data)[0]);
            while (((size_t)base) & 0xf) base++;
        } else {
            base = NULL;
        }
    }

    std::shared_ptr<std::vector<float> > data;
    float *base;
};


#include "footer.h"
#endif
