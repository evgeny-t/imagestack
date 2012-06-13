#ifndef IMAGESTACK_FUNC_H
#define IMAGESTACK_FUNC_H

#include <immintrin.h>
#include "header.h"

// This file defines a set of image-like function objects. They
// represent pure functions over a 4-d integer domain. Their body is
// completely contained within their type, so that they compile to
// efficient code (with no dynamic dispatch).
// 
// E.g., the function f(x, y, t, c) = x*3 + 4 has type Add<Mul<X, Const>, Const>
//
// They are all tagged with a nested type called Func so that sfinae prevents unwanted constructions

namespace Func {

#ifdef __AVX__
#define VECTORIZE
typedef __v8sf vec_type;
#else
#ifdef __SSE__
 #define VECTORIZE
typedef __v4sf vec_type;
#endif
#endif

struct Unbounded {
    bool bounded() const {return false;}
    int getWidth() const {return 0;}
    int getHeight() const {return 0;}
    int getFrames() const {return 0;}
    int getChannels() const {return 0;}
};

struct Const : public Unbounded {
    typedef Const Func;
    const float val;
    Const(const float val_) : val(val_) {}
    float operator()(int x, int y, int c, int t) const {
	return val;
    }

    // State needed to iterate across a scanline    
    struct Iter {
	Iter (float v) : val(v) {
	    #ifdef __AVX__
	    vec_val = _mm256_set_ps(val, val, val, val, val, val, val, val);
	    #else
            #ifdef __SSE__
	    vec_val = _mm_set_ps(val, val, val, val);
	    #endif
	    #endif
	}
	const float val;
	float operator[](int x) const {return val;}
	#ifdef VECTORIZE
	vec_type vec_val;
	vec_type vec(int x) const {return vec_val;}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return Iter(val);
    }
};

struct X : public Unbounded {
    typedef X Func;
    float operator()(int x, int, int, int) const {return x;}

    // State needed to iterate across a scanline    
    struct Iter {
	float operator[](int x) const {return x;}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    #ifdef __AVX__
	    return _mm256_set_ps(x+7, x+6, x+5, x+4, x+3, x+2, x+1, x+0);	    
	    #else
	    return _mm_set_ps(x+3, x+2, x+1, x+0);  
	    #endif
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return Iter();
    }
};

struct Y : public Unbounded {
    typedef Y Func;
    float operator()(int, int y, int, int) const {return y;}

    typedef Const::Iter Iter;

    Const::Iter scanline(int y, int t, int c) const {
	return Const::Iter(y);
    }
};

struct T : public Unbounded {
    typedef T Func;
    float operator()(int, int, int t, int) const {return t;}

    typedef Const::Iter Iter;

    Const::Iter scanline(int y, int t, int c) const {
	return Const::Iter(t);
    }
};

struct C : public Unbounded {
    typedef C Func;
    float operator()(int, int, int, int c) const {return c;}

    typedef Const::Iter Iter;

    Const::Iter scanline(int y, int t, int c) const {
	return Const::Iter(c);
    }
};

template<typename A, typename B>
struct BinaryOp {
    const A a;
    const B b;

    BinaryOp(const A a_, const B b_) : a(a_), b(b_) {
	if (a.bounded() && b.bounded()) {
	    assert(a.getWidth() == b.getWidth() &&
		   a.getHeight() == b.getHeight() &&
		   a.getFrames() == b.getFrames() &&
		   a.getChannels() == b.getChannels(), 
		   "Can only combine images with matching size\n");	    
	}    
    }
    bool bounded() const {
	return a.bounded() || b.bounded();
    }
    int getWidth() const {
	if (a.bounded()) return a.getWidth();
	return b.getWidth();
    }
    int getHeight() const {
	if (a.bounded()) return a.getHeight();
	return b.getHeight();
    }
    int getFrames() const {
	if (a.bounded()) return a.getFrames();
	return b.getFrames();
    }
    int getChannels() const {
	if (a.bounded()) return a.getChannels();
	return b.getChannels();
    }
};

template<typename A, typename B>
struct Add : public BinaryOp<A, B> {
    typedef Add<typename A::Func, typename B::Func> Func;
    Add(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) + BinaryOp<A, B>::b(x, y, t, c);
    }

    struct Iter {
	typename A::Iter a; 
	typename B::Iter b;
	float operator[](int x) const {return a[x] + b[x];}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    return a.vec(x) + b.vec(x);
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
    }
};

template<typename A, typename B>
struct Sub : public BinaryOp<A, B> {
    typedef Sub<typename A::Func, typename B::Func> Func;
    Sub(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) - BinaryOp<A, B>::b(x, y, t, c);
    }

    struct Iter {
	typename A::Iter a; 
	typename B::Iter b;
	float operator[](int x) const {return a[x] - b[x];}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    return a.vec(x) - b.vec(x);
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
    }
};

template<typename A, typename B>
struct Mul : public BinaryOp<A, B> {
    typedef Mul<typename A::Func, typename B::Func> Func;
    Mul(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) * BinaryOp<A, B>::b(x, y, t, c);
    }

    struct Iter {
	typename A::Iter a; 
	typename B::Iter b;
	float operator[](int x) const {return a[x] * b[x];}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    return a.vec(x) * b.vec(x);
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
    }
};

template<typename A, typename B>
struct Div : public BinaryOp<A, B> {
    typedef Div<typename A::Func, typename B::Func> Func;
    Div(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) / BinaryOp<A, B>::b(x, y, t, c);
    }

    struct Iter {
	typename A::Iter a; 
	typename B::Iter b;
	float operator[](int x) const {return a[x] / b[x];}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    return a.vec(x) / b.vec(x);
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
    }
};


template<typename A, typename B>
struct GT : public BinaryOp<A, B> {
    typedef GT<typename A::Func, typename B::Func> Func;
    GT(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) / BinaryOp<A, B>::b(x, y, t, c);
    }

    struct Iter {
	typename A::Iter a; 
	typename B::Iter b;
	float operator[](int x) const {return a[x] > b[x] ? 1 : 0;}
	#ifdef VECTORIZE
	vec_type vec(int x) const {
	    vec_type va = a.vec(x), vb = b.vec(x);
	    #ifdef __AVX__
	    vec_type one = _mm256_set_ps(1, 1, 1, 1, 1, 1, 1, 1);
	    vec_type mask = _mm256_cmp_ps(va, vb, _CMP_GT_OQ);
	    return _mm256_and_ps(mask, one);
	    #else 
	    vec_type one = _mm_set_ps(1, 1, 1, 1);
	    vec_type mask = _mm_cmpgt_ps(va, vb);
	    return _mm_and_ps(mask, one);
	    #endif
	}
	#endif
    };
    Iter scanline(int y, int t, int c) const {
	return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
    }
};

}


template<typename A, typename B>
Func::Add<typename A::Func, typename B::Func> operator+(const A a, const B b) {
    return Func::Add<A, B>(a, b);
}

template<typename B>
Func::Add<Func::Const, typename B::Func> operator+(const float a, const B b) {
    return Func::Add<Func::Const, B>(Func::Const(a), b);
}

template<typename A>
Func::Add<typename A::Func, Func::Const> operator+(const A a, const float b) {
    return Func::Add<A, Func::Const>(a, Func::Const(b));
}


template<typename A, typename B>
Func::Sub<typename A::Func, typename B::Func> operator-(const A a, const B b) {
    return Func::Sub<A, B>(a, b);
}

template<typename B>
Func::Sub<Func::Const, typename B::Func> operator-(const float a, const B b) {
    return Func::Sub<Func::Const, B>(Func::Const(a), b);
}

template<typename A>
Func::Sub<typename A::Func, Func::Const> operator-(const A a, const float b) {
    return Func::Sub<A, Func::Const>(a, Func::Const(b));
}

template<typename A, typename B>
Func::Mul<typename A::Func, typename B::Func> operator*(const A a, const B b) {
    return Func::Mul<A, B>(a, b);
}

template<typename B>
Func::Mul<Func::Const, typename B::Func> operator*(const float a, const B b) {
    return Func::Mul<Func::Const, B>(Func::Const(a), b);
}

template<typename A>
Func::Mul<typename A::Func, Func::Const> operator*(const A a, const float b) {
    return Func::Mul<A, Func::Const>(a, Func::Const(b));
}

template<typename A, typename B>
Func::Div<typename A::Func, typename B::Func> operator/(const A a, const B b) {
    return Func::Div<A, B>(a, b);
}

template<typename B>
Func::Div<Func::Const, typename B::Func> operator/(const float a, const B b) {
    return Func::Div<Func::Const, B>(Func::Const(a), b);
}

template<typename A>
Func::Mul<typename A::Func, Func::Const> operator/(const A a, const float b) {
    // replace expr / const with expr * (1/const)
    return Func::Mul<A, Func::Const>(a, Func::Const(1.0f/b));
}

template<typename A, typename B>
Func::GT<typename A::Func, typename B::Func> operator>(const A a, const B b) {
    return Func::GT<A, B>(a, b);
}

template<typename A>
Func::GT<typename A::Func, Func::Const> operator>(const A a, const float b) {
    return Func::GT<A, Func::Const>(a, Func::Const(b));
}

template<typename B>
Func::GT<Func::Const, typename B::Func> operator>(const float a, const B b) {
    return Func::GT<Func::Const, B>(Func::Const(a), b);
}



#include "footer.h"

#endif
