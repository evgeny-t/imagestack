#include "main.h"
#include "Arithmetic.h"
#include "header.h"
#include "Statistics.h"

void Add::help() {
    printf("\n-add adds the second image in the stack to the top image in the stack.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -add -save out.tga.\n");
}

bool Add::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 3);
    Noise::apply(a, -100, 13);
    Noise::apply(b, 17, 82);
    float before = a(10, 10, 2, 2);
    a += b;
    return a(10, 10, 2, 2) == (before + b(10, 10, 2, 2));
}

void Add::parse(vector<string> args) {
    assert(args.size() == 0, "-add takes no arguments\n");
    stack(0) += stack(1);
    pull(1);
    pop();
}

void Multiply::help() {
    pprintf("-multiply multiplies the top image in the stack by the second image in"
            " the stack. If one or both images are single-channel, then this"
            " performs a scalar-vector multiplication. If both images have multiple"
            " channels, there are three different vector-vector multiplications"
            " possible, selectable with the sole argument. Say the first image has"
            " k channels and the second image has j channels. \"elementwise\""
            " performs element-wise multiplication. If k != j then the lesser is"
            " repeated as necessary. The output has max(k, j) channels. \"inner\""
            " performs an inner, or matrix, product. It treats the image with more"
            " channels as containing matrices in row-major order, and the image"
            " with fewer channels as a column vector, and performs a matrix-vector"
            " multiplication at every pixel. The output has max(j/k, k/j)"
            " channels. \"outer\" performs an outer product at each pixel. The"
            " output image has j*k channels. If no argument is given, the"
            " multiplication will be \"elementwise\".\n\n"
            "Usage: ImageStack -load a.tga -load b.tga -multiply -save out.tga.\n");
}

void Multiply::parse(vector<string> args) {
    assert(args.size() < 2, "-multiply takes zero or one arguments\n");

    Mode m = Elementwise;
    if (args.size() == 0) { m = Elementwise; }
    else if (args[0] == "inner") { m = Inner; }
    else if (args[0] == "outer") { m = Outer; }
    else if (args[0] == "elementwise") { m = Elementwise; }
    else {
        panic("Unknown vector-vector multiplication: %s\n", args[0].c_str());
    }

    bool swapped = false;

    Image a = stack(1);
    Image b = stack(0);
    if (a.channels < b.channels) {
	std::swap(a, b);
        swapped = true;
    }

    if (m == Elementwise || b.channels == 1) {
	a *= b;
        if (!swapped) pop();
        else {
          Image im = stack(0);
          pop();
          pop();
          push(im);
        }
    } else {
        Image im = apply(a, b, m);
        pop();
        pop();
        push(im);
    }
}

bool Multiply::test() {
    Image a(101, 128, 1, 3), b(101, 128, 1, 3);
    Image matrix(101, 128, 1, 9);
    Noise::apply(a, -100, 13);
    Noise::apply(b, 17, 82);
    Noise::apply(matrix, -1, 1);

    printf("Testing outer product\n");
    Image c_outer = apply(a, b, Outer);
    if (c_outer(10, 10, 0, 7) != 
	a(10, 10, 0, 2) * b(10, 10, 0, 1)) return false;

    printf("Testing inner product\n");
    Image c_inner = apply(a, b, Inner);
    float result = c_inner(10, 10, 0, 0);
    float correct = (a(10, 10, 0, 0) * b(10, 10, 0, 0) + 
		     a(10, 10, 0, 1) * b(10, 10, 0, 1) + 
		     a(10, 10, 0, 2) * b(10, 10, 0, 2));

    if (fabs(correct - result) > 0.001) return false;

    printf("Testing elementwise product\n");
    Image c_elementwise = apply(a, b, Elementwise);
    if (c_elementwise(10, 10, 0, 1) != a(10, 10, 0, 1) * b(10, 10, 0, 1)) return false;

    printf("Testing matrix product\n");
    Image c_matrix = apply(matrix, a, Inner);
    if (c_matrix(10, 10, 0, 1) != 
	(matrix(10, 10, 0, 3) * a(10, 10, 0, 0) +
	 matrix(10, 10, 0, 4) * a(10, 10, 0, 1) + 
	 matrix(10, 10, 0, 5) * a(10, 10, 0, 2))) return false;

    printf("Testing scalar product\n");
    Image c_scalar = apply(a, b.channel(0), Outer);
    if (c_scalar(10, 10, 0, 1) != a(10, 10, 0, 1) * b(10, 10, 0, 0)) return false;

    return true;
}

Image Multiply::apply(const Image a, const Image b, Mode m) {
    if (a.channels < b.channels) { return apply(b, a, m); }

    assert(a.width == b.width &&
           a.height == b.height &&
           a.frames == b.frames,
           "Cannot multiply images of different sizes\n");

    assert(a.channels % b.channels == 0,
           "One input must have a number of channels which is a multiple of the other's\n");

    Image out;

    // This code is written on the assumption that this op will be
    // memory-bandwidth limited so too much optimization is foolish

    if (b.channels == 1) { // scalar-vector case
        out = Image(a.width, a.height, a.frames, a.channels);

	for (int c = 0; c < a.channels; c++) {
	    for (int t = 0; t < a.frames; t++) {
		for (int y = 0; y < a.height; y++) {
		    for (int x = 0; x < a.width; x++) {
			out(x, y, t, c) = a(x, y, t, c) * b(x, y, t, 0);
                    }
                }
            }
        }

    } else if (m == Elementwise) {
        out = Image(a.width, a.height, a.frames, a.channels);
        if (a.channels == b.channels) {
	    for (int c = 0; c < a.channels; c++) {		
		for (int t = 0; t < a.frames; t++) {
		    for (int y = 0; y < a.height; y++) {
			for (int x = 0; x < a.width; x++) {
                            out(x, y, t, c) = a(x, y, t, c) * b(x, y, t, c);
                        }
                    }
                }
            }
        } else {
            int factor = a.channels / b.channels;
	    int oc = 0;
	    for (int c = 0; c < factor; c++) {
		for (int bc = 0; bc < b.channels; bc++) {		    
		    for (int t = 0; t < a.frames; t++) {
			for (int y = 0; y < a.height; y++) {
			    for (int x = 0; x < a.width; x++) {
                                out(x, y, t, oc) = a(x, y, t, oc) * b(x, y, t, bc);
                            }
                        }
                    }
		    oc++;				
                }
            }

        }
    } else if (m == Inner) {
        out = Image(a.width, a.height, a.frames, a.channels/b.channels);
        if (a.channels == b.channels) {
	    for (int c = 0; c < a.channels; c++) {
		for (int t = 0; t < a.frames; t++) {
		    for (int y = 0; y < a.height; y++) {
			for (int x = 0; x < a.width; x++) {
			    out(x, y, t, 0) += a(x, y, t, c) * b(x, y, t, c);
			}
                    }
                }
            }
        } else {
            int factor = a.channels / b.channels;
	    int ac = 0;
	    for (int oc = 0; oc < factor; oc++) {
		for (int bc = 0; bc < b.channels; bc++) {
		    for (int t = 0; t < a.frames; t++) {
			for (int y = 0; y < a.height; y++) {
			    for (int x = 0; x < a.width; x++) {
                                out(x, y, t, oc) += a(x, y, t, ac) * b(x, y, t, bc);
                            }
                        }
                    }
		    ac++;
                }
            }
        }
    } else if (m == Outer) {
        out = Image(a.width, a.height, a.frames, a.channels*b.channels);
	int oc = 0;
	for (int ac = 0; ac < a.channels; ac++) {
	    for (int bc = 0; bc < b.channels; bc++) {
		for (int t = 0; t < a.frames; t++) {
		    for (int y = 0; y < a.height; y++) {
			for (int x = 0; x < a.width; x++) {
                            out(x, y, t, oc) = a(x, y, t, ac) * b(x, y, t, bc);
                        }
                    }
                }
		oc++;
            }
        }
    } else {
        panic("Unknown multiplication type: %d\n", m);
    }

    return out;
}


void Subtract::help() {
    printf("\n-subtract subtracts the second image in the stack from the top image in the stack.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -subtract -save out.tga.\n");
}

bool Subtract::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 3);
    Noise::apply(a, -100, 13);
    Noise::apply(b, 17, 82);
    float before = a(10, 10, 2, 2);
    a -= b;
    return a(10, 10, 2, 2) == (before - b(10, 10, 2, 2));
}

void Subtract::parse(vector<string> args) {
    assert(args.size() == 0, "-subtract takes no arguments\n");
    stack(0) -= stack(1);
    pull(1);
    pop();
}

void Divide::help() {
    printf("\n-divide divides the top image in the stack by the second image in the stack.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -divide -save out.tga.\n");
}

bool Divide::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 1);
    Noise::apply(a, -100, 13);
    Noise::apply(b, 17, 82);
    float before = a(10, 10, 2, 2);
    a /= b;
    return a(10, 10, 2, 2) == (before / b(10, 10, 2, 0));    
}

void Divide::parse(vector<string> args) {
    assert(args.size() == 0, "-divide takes no arguments\n");
    stack(0) /= stack(1);
    pull(1);
    pop();
}

void Maximum::help() {
    printf("\n-max replaces the top image in the stack with the max of the top two images.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -max -save out.tga.\n");
}

bool Maximum::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 3);    
    Noise::apply(a, -100, 130);
    Noise::apply(b, 17, 82);
    float before = a(10, 10, 2, 2);
    Maximum::apply(a, b);
    float after = a(10, 10, 2, 2);
    float correct = max(before, b(10, 10, 2, 2));
    return after == correct;
}

void Maximum::parse(vector<string> args) {
    assert(args.size() == 0, "-max takes no arguments\n");
    apply(stack(0), stack(1));
    pull(1);
    pop();
}

void Maximum::apply(Image a, const Image b) {
    assert(a.width == b.width &&
           a.height == b.height &&
           a.frames == b.frames &&
           a.channels == b.channels,
           "Cannot compare images of different sizes or channel numbers\n");

    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    float aVal = a(x, y, t, c);
                    float bVal = b(x, y, t, c);
                    a(x, y, t, c) = max(aVal, bVal);
                }
            }
        }
    }
}


void Minimum::help() {
    printf("\n-min replaces the top image in the stack with the min of the top two images.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -min -save out.tga.\n");
}

bool Minimum::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 3);    
    Noise::apply(a, -100, 130);
    Noise::apply(b, 17, 82);
    float before = a(10, 10, 2, 2);
    Minimum::apply(a, b);
    return a(10, 10, 2, 2) == (min(before, b(10, 10, 2, 2)));
}

void Minimum::parse(vector<string> args) {
    assert(args.size() == 0, "-min takes no arguments\n");
    apply(stack(0), stack(1));
    pull(1);
    pop();
}

void Minimum::apply(Image a, const Image b) {
    assert(a.width == b.width &&
           a.height == b.height &&
           a.frames == b.frames &&
           a.channels == b.channels,
           "Cannot compare images of different sizes or channel numbers\n");

    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    float aVal = a(x, y, t, c);
                    float bVal = b(x, y, t, c);
                    a(x, y, t, c) = min(aVal, bVal);
                }
            }
        }
    }
}


void Log::help() {
    printf("\n-log takes the natural log of the current image.\n\n"
           "Usage: ImageStack -load a.tga -log -load b.tga -log -add -exp -save product.tga.\n");
}

bool Log::test() {
    Image a(101, 128, 4, 3), b(101, 128, 4, 3);    
    Noise::apply(a, 100, 130);
    Noise::apply(b, 17, 82);        
    float correct = a(10, 10, 2, 2) * b(10, 10, 2, 2);
    Log::apply(a);
    Log::apply(b);
    a += b;
    Exp::apply(a);
    float delta = a(10, 10, 2, 2) - correct;
    return fabs(delta) < 0.01;
}

void Log::parse(vector<string> args) {
    assert(args.size() == 0, "-log takes no arguments\n");
    apply(stack(0));
}

void Log::apply(Image a) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    a(x, y, t, c) = logf(a(x, y, t, c));
                }
            }
        }
    }
}

void Exp::help() {
    printf("\nWith no arguments -exp calculates e to the current image. With one argument\n"
           "it calculates that argument to the power of the current image.\n\n"
           "Usage: ImageStack -load a.tga -log -load b.tga -log -add -exp -save product.tga.\n");
}

void Exp::parse(vector<string> args) {
    if (args.size() == 0) { apply(stack(0)); }
    else if (args.size() == 1) { apply(stack(0), readFloat(args[1])); }
    else { panic("-exp takes zero or one arguments\n"); }
}

void Exp::apply(Image a, float base) {
    for (int t = 0; t < a.frames; t++) {
        for (int y = 0; y < a.height; y++) {
            for (int x = 0; x < a.width; x++) {
                for (int c = 0; c < a.channels; c++) {
                    a(x, y, t, c) = powf(base, a(x, y, t, c));
                }
            }
        }
    }
}

bool Exp::test() {
    // Already tested in log
    return true;
}

void Abs::help() {
    printf("\n-abs takes the absolute value of the current image.\n\n"
           "Usage: ImageStack -load a.tga -load b.tga -subtract -abs -save diff.tga\n\n");
}

bool Abs::test() {
    Image a(101, 128, 4, 3);
    Noise::apply(a, -10, -1);        
    float before = a(10, 2, 1, 2);
    Abs::apply(a);
    return a(10, 2, 1, 2) == -before;
}

void Abs::parse(vector<string> args) {
    assert(args.size() == 0, "-abs takes no arguments\n");
    apply(stack(0));
}

void Abs::apply(Image a) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    a(x, y, t, c) = fabs(a(x, y, t, c));
                }
            }
        }
    }
}

void Offset::help() {
    printf("\n-offset adds to the current image. It can either be called with a single\n"
           "argument, or with one argument per image channel.\n"
           "Usage: ImageStack -load a.tga -offset 0.5 34 2 -save b.tga\n\n");
}

bool Offset::test() {
    Image a(101, 128, 4, 3);
    vector<float> offset(3);
    offset[0] = 1;
    offset[1] = 2;
    offset[2] = 3;
    Noise::apply(a, -1, 1);
    float before = a(10, 2, 1, 2);
    a += offset;
    if (a(10, 2, 1, 2) != before + offset[2]) return false;
    before = a(10, 2, 1, 2);
    a += 17.0f;
    if (a(10, 2, 1, 2) != before + 17.0f) return false;    
    return true;
}

void Offset::parse(vector<string> args) {
    vector<float> fargs;
    for (size_t i = 0; i < args.size(); i++) {
        fargs.push_back(readFloat(args[i]));
    }

    stack(0) += fargs;
}

void Scale::help() {
    printf("\n-scale scales the current image. It can either be called with a single\n"
           "argument, or with one argument per image channel.\n"
           "Usage: ImageStack -load a.tga -scale 0.5 34 2 -save b.tga\n\n");
}

bool Scale::test() {
    Image a(101, 128, 4, 3);
    vector<float> offset(3);
    offset[0] = 1;
    offset[1] = 2;
    offset[2] = 3;
    Noise::apply(a, -1, 1);
    float before = a(10, 2, 1, 2);
    a *= offset;
    if (a(10, 2, 1, 2) != before * offset[2]) return false;
    before = a(10, 2, 1, 2);
    a *= 17.0f;
    if (a(10, 2, 1, 2) != before * 17.0f) return false;    
    return true;
}

void Scale::parse(vector<string> args) {
    vector<float> fargs;
    for (size_t i = 0; i < args.size(); i++) {
        fargs.push_back(readFloat(args[i]));
    }

    stack(0) *= fargs;
}

void Gamma::help() {
    printf("\n-gamma raise the current image to a power. It can either be called with a single\n"
           "argument, or with one argument per image channel.\n"
           "Usage: ImageStack -load a.tga -gamma 0.5 34 2 -save b.tga\n\n");
}

bool Gamma::test() {
    Image a(101, 128, 4, 3);
    vector<float> vec(3);
    vec[0] = 1;
    vec[1] = 2;
    vec[2] = 3;
    Noise::apply(a, 1, 10);
    vector<float> before(3);
    before[0] = a(10, 2, 1, 0);
    before[1] = a(10, 2, 1, 1);
    before[2] = a(10, 2, 1, 2);
    Gamma::apply(a, vec);
    if (!nearly_equal(a(10, 2, 1, 0), powf(before[0], 1))) return false;
    if (!nearly_equal(a(10, 2, 1, 1), powf(before[1], 2))) return false;
    if (!nearly_equal(a(10, 2, 1, 2), powf(before[2], 3))) return false;
    return true;
}

void Gamma::parse(vector<string> args) {
    vector<float> fargs;
    for (size_t i = 0; i < args.size(); i++) {
        fargs.push_back(readFloat(args[i]));
    }

    apply(stack(0), fargs);
}

void Gamma::apply(Image a, float gamma) {
    for (int c = 0; c < a.channels; c++) {	
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
		    float val = a(x, y, t, c);
                    if (val > 0) {
                        val = powf(val, gamma);
                    } else {
                        val = -powf(-val, gamma);
                    }
		    a(x, y, t, c) = val;
                }
            }
        }
    }
}

void Gamma::apply(Image a, vector<float> args) {
    assert(args.size() == (size_t)a.channels, 
	   "-gamma takes either 1 argument, or 1 argument per channel\n");
    for (int c = 0; c < a.channels; c++) {
	apply(a.channel(c), args[c]);
    }
}

void Mod::help() {
    printf("\n-mod takes the floating point modulus of the current image. It can either be\n"
           "called with a single argument, or with one argument per image channel.\n"
           "Usage: ImageStack -load a.tga -mod 0.5 34 2 -save b.tga\n\n");
}

bool Mod::test() {
    Image a(101, 128, 4, 3);
    Noise::apply(a, -10, 10);    
    float before = a(10, 2, 1, 2);
    apply(a, 0.5);    
    float after = a(10, 2, 1, 2);
    while (before > 0.5) before -= 0.5;
    while (before < 0) before += 0.5;
    return before == after;    
}

void Mod::parse(vector<string> args) {
    vector<float> fargs;
    for (size_t i = 0; i < args.size(); i++) {
        fargs.push_back(readFloat(args[i]));
    }

    apply(stack(0), fargs);
}

void Mod::apply(Image a, float mod) {
    for (int c = 0; c < a.channels; c++) {		    
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
		    float val = fmod(a(x, y, t, c), mod);
		    if (val < 0) val += mod;
                    a(x, y, t, c) = val;
                }
            }
        }
    }
}

void Mod::apply(Image a, vector<float> args) {
    assert(args.size() == (size_t)a.channels, "-mod takes either 1 argument, or 1 argument per channel\n");
    for (int c = 0; c < a.channels; c++) {
	apply(a.channel(c), args[c]);
    }
}

void Clamp::help() {
    printf("\n-clamp restricts the image to be between the given minimum and maximum\n"
           "by saturating values outside that range. If given no arguments it defaults\n"
           "to clamping between zero and one.\n\n"
           "Usage: ImageStack -load a.exr -clamp 0 1 -save a.tga\n\n");
}

bool Clamp::test() {
    Image a(101, 128, 4, 3);
    Noise::apply(a, -1, 1);    
    float before = a(10, 2, 1, 2);
    Clamp::apply(a, -0.5, 0.5);
    float after = a(10, 2, 1, 2);
    if (before < -0.5) return after == -0.5;
    if (before > 0.5) return after == 0.5;
    return before == after;
}

void Clamp::parse(vector<string> args) {
    if (args.size() == 0) {
        apply(stack(0), 0, 1);
    } else if (args.size() == 2) {
        apply(stack(0), readFloat(args[0]), readFloat(args[1]));
    } else {
        panic("-clamp takes zero or two arguments\n");
    }
}

void Clamp::apply(Image a, float lower, float upper) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
		    float val = a(x, y, t, c);
                    val = max(lower, val);
		    val = min(upper, val);
		    a(x, y, t, c) = val;
                }
            }
        }
    }
}

void DeNaN::help() {
    printf("\n-denan replaces all NaN values in the current image with its argument, which\n"
           "defaults to zero.\n\n"
           "Usage: ImageStack -load in.jpg -eval \"1/val\" -denan -save out.jpg\n\n");
}

bool DeNaN::test() {
    Image a(101, 128, 4, 3);
    Image b(101, 128, 4, 3);
    Noise::apply(a, -1, 1);
    Noise::apply(b, -1, 1);
    Threshold::apply(b, 0);
    a *= b;
    a /= b;
    float before = a(10, 2, 1, 2);
    apply(a, 17.0f);
    float after = a(10, 2, 1, 2);
    if (b(10, 2, 1, 2)) return before == after;
    else return after == 17.0f;
}

void DeNaN::parse(vector<string> args) {
    if (args.size() == 0) {
        apply(stack(0), 0);
    } else if (args.size() == 1) {
        apply(stack(0), readFloat(args[0]));
    } else {
        panic("-denan takes zero or one arguments\n");
    }
}

void DeNaN::apply(Image a, float replacement) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
		    float val = a(x, y, t, c);
		    if (isnan(val)) {
			a(x, y, t, c) = replacement; 
		    }
                }
            }
        }
    }
}

void Threshold::help() {
    printf("\n-threshold sets the image to zero where it is less than the argument, and\n"
           "sets it to one where it is greater than or equal to the argument.\n\n"
           "Usage: ImageStack -load a.exr -threshold 0.5 -save monochrome.tga\n\n");
}

bool Threshold::test() {
    // Tested by DeNaN
    return true;
}

void Threshold::parse(vector<string> args) {
    assert(args.size() == 1, "-threshold takes exactly one argument\n");
    apply(stack(0), readFloat(args[0]));
}


void Threshold::apply(Image a, float threshold) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    a(x, y, t, c) = a(x, y, t, c) > threshold ? 1.0f : 0.0f;
                }
            }
        }
    }
}

void Normalize::help() {
    printf("\n-normalize restricts the image to be between 0 and 1\n"
           "by rescaling and shifting it.\n\n"
           "Usage: ImageStack -load a.exr -normalize -save a.tga\n\n");
}

bool Normalize::test() {
    Image a(101, 128, 4, 3);
    Noise::apply(a, -3, 1);
    a(0, 0, 0, 0) = 1;
    a(0, 0, 0, 1) = -3;
    float before = a(20, 10, 2, 2);
    apply(a);
    float after = a(20, 10, 2, 2);
    before -= -3.0f;
    before *= 0.25f;
    return after == before;    
}

void Normalize::parse(vector<string> args) {
    assert(args.size() == 0, "-normalize takes no arguments\n");
    apply(stack(0));
}


void Normalize::apply(Image a) {
    float minValue = a(0, 0);
    float maxValue = a(0, 0);

    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {		    
                    minValue = min(a(x, y, t, c), minValue);
                    maxValue = max(a(x, y, t, c), maxValue);
                }
            }
        }
    }    

    float invDelta = 1.0f/(maxValue - minValue);
    for (int c = 0; c < a.channels; c++) {		    
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
                    a(x, y, t, c) -= minValue;
                    a(x, y, t, c) *= invDelta;
                }
            }
        }
    }
}


void Quantize::help() {
    printf("\n-quantize rounds all values down to the nearest multiple of the sole\n"
           "argument. If no argument is given, quantize rounds down to the nearest\n"
           "integer.\n\n"
           "Usage: ImageStack -load test.jpg -quantize 1/128 -save test2.jpg\n\n");
}

bool Quantize::test() {
    Image a(101, 128, 4, 3);
    Noise::apply(a, -1, 1);    
    float before = a(2, 14, 3, 1);
    apply(a, 0.5f);
    float after = a(2, 14, 3, 1);    
    if (before >= 1) return after == 1.0f;
    if (before >= 0.5) return after == 0.5f;
    if (before >= 0.0) return after == 0.0f;
    if (before >= -0.5) return after == -0.5f;
    if (before >= -1.0) return after == -1.0f;
    return false;
}

void Quantize::parse(vector<string> args) {
    assert(args.size() <= 1, "-quantize takes zero or one arguments\n");

    if (args.size()) { apply(stack(0), readFloat(args[0])); }
    else { apply(stack(0), 1); }

}

void Quantize::apply(Image a, float increment) {
    for (int c = 0; c < a.channels; c++) {
	for (int t = 0; t < a.frames; t++) {
	    for (int y = 0; y < a.height; y++) {
		for (int x = 0; x < a.width; x++) {
		    float val = fmodf(a(x, y, t, c), increment);
		    if (val < 0) val += increment;
                    a(x, y, t, c) -= val;
                }
            }
        }
    }
}

#include "footer.h"
