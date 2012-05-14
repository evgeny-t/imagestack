#include "main.h"
#include "Wavelet.h"
#include "Geometry.h"
#include "header.h"

void Haar::help() {
    pprintf("-haar performs the standard 2D haar transform of an image. The image"
            " size must be a power of two. If given an integer argument k, it only"
            " recurses k times, and the image size must be a multiple of 2^k.\n"
            "\n"
            "Usage: ImageStack -load in.jpg -haar 1 -save out.jpg\n\n");
}

void Haar::parse(vector<string> args) {
    if (args.size() == 0) {
        apply(stack(0));
    } else if (args.size() == 1) {
        apply(stack(0), readInt(args[0]));
    } else {
        panic("-haar requires zero or one arguments\n");
    }
}

void Haar::apply(Image im, int times) {

    if (times <= 0) {
        assert(im.width == im.height, "to perform a full haar transorm, the image must be square.\n");
        times = 0;
        int w = im.width >> 1;
        while (w) {
            times++;
            w >>= 1;
        }
    }

    int factor = 1 << times;
    assert(im.width % factor == 0, "the image width is not a multiple of 2^%i", times);
    assert(im.height % factor == 0, "the image height is not a multiple of 2^%i", times);

    // transform in x
    Image win = im.region(0, 0, 0, 0, im.width, im.height, im.frames, im.channels);
    for (int i = 0; i < times; i++) {
	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y++) {
		    for (int x = 0; x < win.width; x+=2) {
			float a = win(x, y, t);
			float b = win(x+1, y, t);
                        win(x, y, t, c) = (a+b)/2;
                        win(x+1, y, t, c) = b-a;
                    }
                }
            }
        }
        // separate into averages and differences
        Deinterleave::apply(win, 2, 1, 1);
        // repeat on the averages
        win = win.region(0, 0, 0, 0, win.width/2, win.height, win.frames, win.channels);
    }

    // transform in y
    win = im.region(0, 0, 0, 0, im.width, im.height, im.frames, im.channels);
    for (int i = 0; i < times; i++) {
	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y+=2) {
		    for (int x = 0; x < win.width; x++) {
			float a = win(x, y, t, c);
			float b = win(x, y+1, t, c);
			win(x, y, t, c) = (a+b)/2;
			win(x, y+1, t, c) = b-a;
                    }
                }
            }
        }
        // separate into averages and differences
        Deinterleave::apply(win, 1, 2, 1);
        // repeat on the averages
        win = win.region(0, 0, 0, 0, win.width, win.height/2, win.frames, win.channels);
    }
}


void InverseHaar::help() {
    printf("-inversehaar inverts the haar transformation with the same argument. See\n"
           "-help haar for detail.\n\n");

}

void InverseHaar::parse(vector<string> args) {
    if (args.size() == 0) {
        apply(stack(0));
    } else if (args.size() == 1) {
        apply(stack(0), readInt(args[0]));
    } else {
        panic("-haar requires zero or one arguments\n");
    }
}

void InverseHaar::apply(Image im, int times) {
    if (times <= 0) {
        assert(im.width == im.height, "to perform a full haar transorm, the image must be square.\n");
        times = 0;
        int w = im.width >> 1;
        while (w) {
            times++;
            w >>= 1;
        }
    }

    int factor = 1 << times;
    assert(im.width % factor == 0, "the image width is not a multiple of 2^%i", times);
    assert(im.height % factor == 0, "the image height is not a multiple of 2^%i", times);

    // transform in y
    int h = 2*im.height/factor;
    Image win = im.region(0, 0, 0, 0, im.width, h, im.frames, im.channels);
    while (1) {
        // combine the averages and differences
        Interleave::apply(win, 1, 2, 1);

	for (int c = 0; c < win.frames; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y+=2) {
		    for (int x = 0; x < win.width; x++) {
			float avg = win(x, y, t, c);
			float diff = win(x, y+1, t, c);
			win(x, y, t, c) = avg-diff/2;
			win(x, y+1, t, c) = avg+diff/2;
		    }
		}
	    }
	}
        // repeat
        h *= 2;
        if (h > im.height) { break; }
        win = im.region(0, 0, 0, 0, im.width, h, im.frames, im.channels);
    }

    // transform in x
    int w = 2*im.width/factor;
    win = im.region(0, 0, 0, 0, w, im.height, im.frames, im.channels);
    while (1) {
        // combine the averages and differences
        Interleave::apply(win, 2, 1, 1);

	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y++) {
		    for (int x = 0; x < win.width; x+=2) {
			float avg = win(x, y, t, c);
			float diff = win(x+1, y, t, c);
			win(x, y, t, c) = avg-diff/2;
			win(x+1, y, t, c) = avg+diff/2;
                    }
                }
            }
        }
        // repeat
        w *= 2;
        if (w > im.width) { break; }
        win = im.region(0, 0, 0, 0, w, im.height, im.frames, im.channels);
    }
}




#define DAUB0 0.4829629131445341
#define DAUB1 0.83651630373780772
#define DAUB2 0.22414386804201339
#define DAUB3 -0.12940952255126034

void Daubechies::help() {
    printf("-daubechies performs the standard 2D daubechies 4 wavelet transform of an image. \n"
           "The image size must be a power of two.\n\n"
           "Usage: ImageStack -load in.jpg -daubechies -save out.jpg\n\n");
}

void Daubechies::parse(vector<string> args) {
    assert(args.size() == 0, "-daubechies takes no arguments");
    apply(stack(0));
}

void Daubechies::apply(Image im) {

    int i;
    for (i = 1; i < im.width; i <<= 1) { ; }
    assert(i == im.width, "Image width must be a power of two\n");
    for (i = 1; i < im.height; i <<= 1) { ; }
    assert(i == im.height, "Image height must be a power of two\n");

    // transform in x
    Image win = im;
    while (1) {
	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y++) {
		    float saved1st = win(0, y, t, c);
		    float saved2nd = win(1, y, t, c);

		    for (int x = 0; x < win.width-2; x+=2) {
			float a = win(x, y, t, c);
			float b = win(x+1, y, t, c);
			float c = win(x+2, y, t, c);
			float d = win(x+3, y, t, c);
			win(x, y, t, c) = DAUB0 * a + DAUB1 * b + DAUB2 * c + DAUB3 * d;
			win(x+1, y, t, c) = DAUB3 * a - DAUB2 * b + DAUB1 * c - DAUB0 * d;
		    }
		    // special case the last two elements using rotation
		    float a = win(win.width-2, y, t, c);
		    float b = win(win.width-1, y, t, c);
		    float c = saved1st;
		    float d = saved2nd;
		    win(win.width-2, y, t, c) = DAUB0 * a + DAUB1 * b + DAUB2 * c + DAUB3 * d;
		    win(win.width-1, y, t, c) = DAUB3 * a - DAUB2 * b + DAUB1 * c - DAUB0 * d;
		}
            }
        }
        // separate into averages and differences
        Deinterleave::apply(win, 2, 1, 1);

        if (win.width == 2) { break; }

        // repeat on the averages
        win = win.region(0, 0, 0, 0, win.width/2, win.height, win.frames, win.channels);
    }

    // transform in y
    win = im.region(0, 0, 0, 0, im.width, im.height, im.frames, im.channels);
    while (1) {
	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int x = 0; x < win.width; x++) {
		    float saved1st = win(x, 0, t, c);
		    float saved2nd = win(x, 1, t, c);
		    for (int y = 0; y < win.height-2; y+=2) {
			float a = win(x, y, t, c);
			float b = win(x, y+1, t, c);
			float c = win(x, y+2, t, c);
			float d = win(x, y+3, t, c);
                        win(x, y, t, c) = DAUB0 * a + DAUB1 * b + DAUB2 * c + DAUB3 * d;
                        win(x, y+1, t, c) = DAUB3 * a - DAUB2 * b + DAUB1 * c - DAUB0 * d;
                    }
		    // special case the last two elements using rotation
		    float a = win(x, win.height-2, t, c);
		    float b = win(x, win.height-1, t, c);
		    float c = saved1st;
		    float d = saved2nd;
                    win(x, win.height-2, t, c) = DAUB0 * a + DAUB1 * b + DAUB2 * c + DAUB3 * d;
                    win(x, win.height-1, t, c) = DAUB3 * a - DAUB2 * b + DAUB1 * c - DAUB0 * d;
                }
            }
        }
        // separate into averages and differences
        Deinterleave::apply(win, 1, 2, 1);

        if (win.height == 2) { break; }

        // repeat on the averages
        win = win.region(0, 0, 0, 0, win.width, win.height/2, win.frames, win.channels);
    }
}


void InverseDaubechies::help() {
    printf("-inversedaubechies performs the standard 2D daubechies 4 wavelet transform of an image. \n"
           "The image size must be a power of two.\n\n"
           "Usage: ImageStack -load in.jpg -inversedaubechies -save out.jpg\n\n");
}

void InverseDaubechies::parse(vector<string> args) {
    assert(args.size() == 0, "-inversedaubechies takes no arguments");
    apply(stack(0));
}

void InverseDaubechies::apply(Image im) {

    int i;
    for (i = 1; i < im.width; i <<= 1) { ; }
    assert(i == im.width, "Image width must be a power of two\n");
    for (i = 1; i < im.height; i <<= 1) { ; }
    assert(i == im.height, "Image height must be a power of two\n");


    // transform in x
    Image win = im.region(0, 0, 0, 0, 2, im.height, im.frames, im.channels);
    while (1) {
        // Collect averages and differences
        Interleave::apply(win, 2, 1, 1);

	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int y = 0; y < win.height; y++) {
		    float saved1st = win(win.width-1, y, t, c);
		    float saved2nd = win(win.width-2, y, t, c);

		    for (int x = win.width-4; x >= 0; x-=2) {
			float a = win(x, y, t, c);
			float b = win(x+1, y, t, c);
			float c = win(x+2, y, t, c);
			float d = win(x+3, y, t, c);
		        win(x+2, y, t, c) = DAUB2 * a + DAUB1 * b + DAUB0 * c + DAUB3 * d;
			win(x+3, y, t, c) = DAUB3 * a - DAUB0 * b + DAUB1 * c - DAUB2 * d;
		    }

		    
		    // special case the first two elements using rotation
		    float a = saved2nd;
		    float b = saved1st;
		    float c = win(0, y, t, c);
		    float d = win(1, y, t, c);
                    win(0, y, t, c) = DAUB2 * a + DAUB1 * b + DAUB0 * c + DAUB3 * d;
                    win(1, y, t, c) = DAUB3 * a - DAUB0 * b + DAUB1 * c - DAUB2 * d;
                }
            }
        }

        if (win.width == im.width) { break; }
	
        // repeat on the averages
        win = im.region(0, 0, 0, 0, win.width*2, win.height, win.frames, win.channels);
    }

    // transform in y
    win = im.region(0, 0, 0, 0, im.width, 2, im.frames, im.channels);
    while (1) {
        // Collect averages and differences
        Interleave::apply(win, 1, 2, 1);

	for (int c = 0; c < win.channels; c++) {
	    for (int t = 0; t < win.frames; t++) {
		for (int x = 0; x < win.width; x++) {
		    float saved1st = win(x, win.height-1, t, c);
		    float saved2nd = win(x, win.height-2, t, c);

		    for (int y = win.height-4; y >= 0; y-=2) {
			float a = win(x, y, t, c);
			float b = win(x, y+1, t, c);
			float c = win(x, y+2, t, c);
			float d = win(x, y+3, t, c);
                        win(x, y+2, t, c) = DAUB2 * a + DAUB1 * b + DAUB0 * c + DAUB3 * d;
                        win(x, y+3, t, c) = DAUB3 * a - DAUB0 * b + DAUB1 * c - DAUB2 * d;
                    }

		    // special case the first two elements using rotation
		    float a = saved2nd;
		    float b = saved1st;
		    float c = win(x, 0, t, c);
		    float d = win(x, 1, t, c);
                    win(x, 0, t, c) = DAUB2 * a + DAUB1 * b + DAUB0 * c + DAUB3 * d;
                    win(x, 1, t, c) = DAUB3 * a - DAUB0 * b + DAUB1 * c - DAUB2 * d;
                }
            }
        }

        if (win.height == im.height) { break; }

        // repeat on the averages
        win = im.region(0, 0, 0, 0, win.width, win.height*2, win.frames, win.channels);
    }
}



#include "footer.h"
