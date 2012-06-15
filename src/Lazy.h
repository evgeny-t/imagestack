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
// They are all tagged with a nested type called Lazy so that sfinae prevents unwanted constructions

namespace Lazy {

#ifdef __AVX__
#define VECTORIZE
    typedef __v8sf vec_type;
    const int vec_width = 8;
    static const vec_type vec_true() {
	vec_type va;
	return _mm256_cmp_ps(va, va, _CMP_EQ_OQ);
    }
    static const vec_type vec_zero() {
	return _mm256_setzero_ps();
    }
    static const vec_type vec_is_non_zero(vec_type v) {
	return _mm256_cmp_ps(v, vec_zero(), _CMP_NEQ_OQ);
    }
#else
#ifdef __SSE__
#define VECTORIZE
    typedef __v4sf vec_type;
    const int vec_width = 4;
    static const vec_type vec_true() {
	vec_type va;
	return _mm_cmpeq_ps(va, va);
    }
    static const vec_type vec_zero() {
	return _mm_setzero_ps();
    }
    static const vec_type vec_is_non_zero(vec_type v) {
	return _mm_cmpneq_ps(v, vec_zero());
    }
#endif    
#endif

    struct Unbounded {
	int getWidth() const {return 0;}
	int getHeight() const {return 0;}
	int getFrames() const {return 0;}
	int getChannels() const {return 0;}
    };

    struct Const : public Unbounded {
	typedef Const Lazy;
	const float val;
	Const(const float val_) : val(val_) {}
	float operator()(int x, int y, int c, int t) const {
	    return val;
	}

	// State needed to iterate across a scanline    
	struct Iter {
	    Iter (float v) : val(v) {
                #ifdef __AVX__
		vec_val = _mm256_set1_ps(val);
                #else
                #ifdef __SSE__
		vec_val = _mm_set1_ps(val);
                #endif
                #endif
	    }
	    const float val;
	    float operator[](int x) const {return val;}
            #ifdef VECTORIZE
	    vec_type vec_val;
	    vec_type vec(int x) const {return vec_val;}
	    vec_type vec_bool(int x) const {
		return val ? vec_true() : vec_zero();
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return Iter(val);
	}
    };

    struct X : public Unbounded {
	typedef X Lazy;
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
	    vec_type vec_bool(int x) const {
		if (x > 0) return vec_true();
		union {
		    uint32_t high; 
		    float f_high;
		} u;
		u.high = 0xffffffff;
		
                #ifdef __AVX__
		return _mm256_set_ps(0, 0, 0, 0, 0, 0, 0, u.f_high);
		#else
		return _mm_set_ps(0, 0, 0, u.f_high);
		#endif
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return Iter();
	}
    };

    struct Y : public Unbounded {
	typedef Y Lazy;
	float operator()(int, int y, int, int) const {return y;}

	typedef Const::Iter Iter;

	Const::Iter scanline(int y, int t, int c) const {
	    return Const::Iter(y);
	}
    };

    struct T : public Unbounded {
	typedef T Lazy;
	float operator()(int, int, int t, int) const {return t;}

	typedef Const::Iter Iter;

	Const::Iter scanline(int y, int t, int c) const {
	    return Const::Iter(t);
	}
    };

    struct C : public Unbounded {
	typedef C Lazy;
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
	    if (a.getWidth() && b.getWidth()) {
		assert(a.getWidth() == b.getWidth(),
		       "Can only combine images with matching width\n");
	    }
	    if (a.getHeight() && b.getHeight()) {
		assert(a.getHeight() == b.getHeight(),
		       "Can only combine images with matching height\n");
	    }
	    if (a.getFrames() && b.getFrames()) {
		assert(a.getFrames() == b.getFrames(),
		       "Can only combine images with matching frames\n");
	    }
	    if (a.getChannels() && b.getChannels()) {
		assert(a.getChannels() == b.getChannels(),
		       "Can only combine images with matching channels\n");
	    }
	}

	int getWidth() const {
	    if (a.getWidth()) return a.getWidth();
	    return b.getWidth();
	}
	int getHeight() const {
	    if (a.getHeight()) return a.getHeight();
	    return b.getHeight();
	}
	int getFrames() const {
	    if (a.getFrames()) return a.getFrames();
	    return b.getFrames();
	}
	int getChannels() const {
	    if (a.getChannels()) return a.getChannels();
	    return b.getChannels();
	}
    };

    template<typename A, typename B>
    struct Add : public BinaryOp<A, B> {
	typedef Add<typename A::Lazy, typename B::Lazy> Lazy;
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
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };

    template<typename A, typename B>
    struct Sub : public BinaryOp<A, B> {
	typedef Sub<typename A::Lazy, typename B::Lazy> Lazy;
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
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };

    template<typename A, typename B>
    struct Mul : public BinaryOp<A, B> {
	typedef Mul<typename A::Lazy, typename B::Lazy> Lazy;
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
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };

    template<typename A, typename B>
    struct Div : public BinaryOp<A, B> {
	typedef Div<typename A::Lazy, typename B::Lazy> Lazy;
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
	    vec_type vec_bool(int x) const {
		// just check the numerator
		return vec_is_non_zero(a.vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };


    template<typename A, typename B>
    struct _Min : public BinaryOp<A, B> {
	typedef _Min<typename A::Lazy, typename B::Lazy> Lazy;
	_Min(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
	float operator()(int x, int y, int t, int c) const {
	    return min(BinaryOp<A, B>::a(x, y, t, c), BinaryOp<A, B>::b(x, y, t, c));
	}

	struct Iter {
	    typename A::Iter a; 
	    typename B::Iter b;
	    float operator[](int x) const {return a[x] / b[x];}
            #ifdef VECTORIZE
	    vec_type vec(int x) const {		
		#ifdef __AVX__
		return _mm256_min_ps(a.vec(x), b.vec(x));
		#else
		return _mm_min_ps(a.vec(x), b.vec(x));
		#endif
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };

    template<typename A, typename B>
    struct _Max : public BinaryOp<A, B> {
	typedef _Max<typename A::Lazy, typename B::Lazy> Lazy;
	_Max(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
	float operator()(int x, int y, int t, int c) const {
	    return max(BinaryOp<A, B>::a(x, y, t, c), BinaryOp<A, B>::b(x, y, t, c));
	}

	struct Iter {
	    typename A::Iter a; 
	    typename B::Iter b;
	    float operator[](int x) const {return a[x] / b[x];}
            #ifdef VECTORIZE
	    vec_type vec(int x) const {		
		#ifdef __AVX__
		return _mm256_max_ps(a.vec(x), b.vec(x));
		#else
		return _mm_max_ps(a.vec(x), b.vec(x));
		#endif
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };

    template<typename A, typename B>
    _Min<typename A::Lazy, typename B::Lazy>
    min(const A a, const B b) {
	return _Min<typename A::Lazy, typename B::Lazy>(a, b);
    }
    template<typename A>
    _Min<typename A::Lazy, Const>
    min(const A a, float b) {
	return _Min<typename A::Lazy, Const>(a, b);
    }
    template<typename B>
    _Min<Const, typename B::Lazy>
    min(float a, const B b) {
	return _Min<Const, typename B::Lazy>(a, b);
    }
    template<typename A, typename B>
    _Max<typename A::Lazy, typename B::Lazy>
    max(const A a, const B b) {
	return _Max<typename A::Lazy, typename B::Lazy>(a, b);
    }
    template<typename A>
    _Max<typename A::Lazy, Const>
    max(const A a, float b) {
	return _Max<typename A::Lazy, Const>(a, b);
    }
    template<typename B>
    _Max<Const, typename B::Lazy>
    max(float a, const B b) {
	return _Max<Const, typename B::Lazy>(a, b);
    }
   
    template<typename A, typename B, typename C>
    _Min<_Max<typename A::Lazy, typename B::Lazy>, typename C::Lazy>  
    clamp(const A a, const B b, const C c) {
	return min(max(a, b), c);
    }
    template<typename B, typename C>
    _Min<_Max<Const, typename B::Lazy>, typename C::Lazy>  
    clamp(float a, const B b, const C c) {
	return min(max(a, b), c);
    }
    template<typename A, typename C>
    _Min<_Max<typename A::Lazy, Const>, typename C::Lazy>  
    clamp(const A a, float b, const C c) {
	return min(max(a, b), c);
    }
    template<typename A, typename B>
    _Min<_Max<typename A::Lazy, typename B::Lazy>, Const>  
    clamp(const A a, const B b, float c) {
	return min(max(a, b), c);
    }
    template<typename A>
    _Min<_Max<typename A::Lazy, Const>, Const>  
    clamp(const A a, float b, float c) {
	return min(max(a, b), c);
    }

    struct GT {
	static bool cmp(float a, float b) {return a > b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_GT_OQ);	    
            #else
	    return _mm_cmpgt_ps(a, b);
            #endif
	}	
	#endif
    };
    struct LT {
	static bool cmp(float a, float b) {return a < b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_LT_OQ);	    
            #else
	    return _mm_cmplt_ps(a, b);
            #endif
	}	
	#endif
    };
    struct GE {
	static bool cmp(float a, float b) {return a >= b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_GE_OQ);	    
            #else
	    return _mm_cmpge_ps(a, b);
            #endif
	}	
	#endif
    };
    struct LE {
	static bool cmp(float a, float b) {return a <= b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_LE_OQ);	    
            #else
	    return _mm_cmple_ps(a, b);
            #endif
	}	
	#endif
    };
    struct EQ {
	static bool cmp(float a, float b) {return a == b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);	    
            #else
	    return _mm_cmpeq_ps(a, b);
            #endif
	}	
	#endif
    };
    struct NEQ {
	static bool cmp(float a, float b) {return a != b;}
	#ifdef VECTORIZE
	static vec_type vec_cmp(vec_type a, vec_type b) {
            #ifdef __AVX__
	    return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);	    
            #else
	    return _mm_cmpneq_ps(a, b);
            #endif
	}	
	#endif
    };

    

    template<typename A, typename B, typename C>
    struct Cmp : public BinaryOp<A, B> {
	typedef Cmp<typename A::Lazy, typename B::Lazy, C> Lazy;
	Cmp(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
	float operator()(int x, int y, int t, int c) const {
	    return C::cmp(BinaryOp<A, B>::a(x, y, t, c),
			  BinaryOp<A, B>::b(x, y, t, c)) ? 1 : 0;
	}
	
	struct Iter {
	    typename A::Iter a; 
	    typename B::Iter b;
	    float operator[](int x) const {
		return C::cmp(a[x], b[x]) ? 1 : 0;
	    }
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		vec_type mask = vec_bool(x);
                #ifdef __AVX__
		vec_type one = _mm256_set1_ps(1);
		return _mm256_and_ps(mask, one);
                #else 
	 	vec_type one = _mm_set1_ps(1);
		return _mm_and_ps(mask, one);
                #endif
	    }
	    vec_type vec_bool(int x) const {
		return C::vec_cmp(a.vec(x), b.vec(x));
	    }
	    #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c), BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };
    
    // Lift a unary function over floats to the same function over an image (e.g. cosf)
    template<float (*fn)(float), typename B>
    struct Lift {
	typedef Lift<fn, typename B::Lazy> Lazy;
	const B b;
	Lift(const B b_) : b(b_) {}
	float operator()(int x, int y, int t, int c) const {
	    return (*fn)(b(x, y, t, c));
	}

	int getWidth() const {return b.getWidth();}
	int getHeight() const {return b.getHeight();}
	int getFrames() const {return b.getFrames();}
	int getChannels() const {return b.getChannels();}

	struct Iter {
	    typename B::Iter b;
	    float operator[](int x) const {return (*fn)(b[x]);}
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		union {		   
		    float f[vec_width];
		    vec_type v;
		} vb;
		vb.v = b.vec(x);
		vb.f[0] = (*fn)(vb.f[0]);
		vb.f[1] = (*fn)(vb.f[1]);
		vb.f[2] = (*fn)(vb.f[2]);
		vb.f[3] = (*fn)(vb.f[3]);
		#ifdef __AVX__
		vb.f[4] = (*fn)(vb.f[4]);
		vb.f[5] = (*fn)(vb.f[5]);
		vb.f[6] = (*fn)(vb.f[6]);
		vb.f[7] = (*fn)(vb.f[7]);
		#endif
		return vb.v;
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {b.scanline(y, t, c)};
	}
    };

    // Lift a binary function over floats to the same function over an image (e.g. cosf)
    template<float (*fn)(float, float), typename A, typename B>
    struct Lift2 : public BinaryOp<A, B> {
	typedef Lift2<fn, typename A::Lazy, typename B::Lazy> Lazy;
	Lift2(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
	float operator()(int x, int y, int t, int c) const {
	    return (*fn)(BinaryOp<A, B>::a(x, y, t, c),
			 BinaryOp<A, B>::b(x, y, t, c));
	}

	struct Iter {
	    typename A::Iter a;
	    typename B::Iter b;
	    float operator[](int x) const {return (*fn)(a[x], b[x]);}
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		union {		   
		    float f[vec_width];
		    vec_type v;
		} va, vb;
		va.v = a.vec(x);
		vb.v = b.vec(x);
		vb.f[0] = (*fn)(va.f[0], vb.f[0]);
		vb.f[1] = (*fn)(va.f[1], vb.f[1]);
		vb.f[2] = (*fn)(va.f[2], vb.f[2]);
		vb.f[3] = (*fn)(va.f[3], vb.f[3]);
		#ifdef __AVX__
		vb.f[4] = (*fn)(va.f[4], vb.f[4]);
		vb.f[5] = (*fn)(va.f[5], vb.f[5]);
		vb.f[6] = (*fn)(va.f[6], vb.f[6]);
		vb.f[7] = (*fn)(va.f[7], vb.f[7]);
		#endif
		return vb.v;
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	};
	Iter scanline(int y, int t, int c) const {
	    return {BinaryOp<A, B>::a.scanline(y, t, c),
		    BinaryOp<A, B>::b.scanline(y, t, c)};
	}
    };


    template<typename A>
    Lift<logf, typename A::Lazy> log(const A a) {
	return Lift<logf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<expf, typename A::Lazy> exp(const A a) {
	return Lift<expf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<cosf, typename A::Lazy> cos(const A a) {
	return Lift<cosf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<sinf, typename A::Lazy> sin(const A a) {
	return Lift<sinf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<tanf, typename A::Lazy> tan(const A a) {
	return Lift<tanf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<acosf, typename A::Lazy> acos(const A a) {
	return Lift<acosf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<asinf, typename A::Lazy> asin(const A a) {
	return Lift<asinf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<atanf, typename A::Lazy> atan(const A a) {
	return Lift<atanf, typename A::Lazy>(a);
    }

    template<typename A>
    Lift<fabsf, typename A::Lazy> abs(const A a) {
	return Lift<fabsf, typename A::Lazy>(a);
    }

    template<typename A, typename B>
    Lift2<powf, typename A::Lazy, typename B::Lazy> pow(const A a, const B b) {
	return Lift2<powf, typename A::Lazy, typename B::Lazy>(a, b);
    }
    template<typename A>
    Lift2<powf, typename A::Lazy, Const> pow(const A a, float b) {
	return Lift2<powf, typename A::Lazy, Const>(a, b);
    }
    template<typename B>
    Lift2<powf, Const, typename B::Lazy> pow(float a, const B b) {
	return Lift2<powf, Const, typename B::Lazy>(a, b);
    }

    template<typename A, typename B>
    Lift2<fmodf, typename A::Lazy, typename B::Lazy> fmod(const A a, const B b) {
	return Lift2<fmodf, typename A::Lazy, typename B::Lazy>(a, b);
    }
    template<typename A>
    Lift2<fmodf, typename A::Lazy, Const> fmod(const A a, float b) {
	return Lift2<fmodf, typename A::Lazy, Const>(a, b);
    }
    template<typename B>
    Lift2<fmodf, Const, typename B::Lazy> fmod(float a, const B b) {
	return Lift2<fmodf, Const, typename B::Lazy>(a, b);
    }
    
    template<typename A>
    struct _RepeatC {
	typedef _RepeatC<typename A::Lazy> Lazy;
	const A a;
	_RepeatC(const A a_) : a(a_) {}
	float operator()(int x, int y, int t, int c) const {
	    return a(x, y, t, 0);
	}

	int getWidth() const {return a.getWidth();}
	int getHeight() const {return a.getHeight();}
	int getFrames() const {return a.getFrames();}
	int getChannels() const {return 0;}

	typedef typename A::Iter Iter;
	Iter scanline(int y, int t, int c) const {
	    return a.scanline(y, t, 0);
	}
    };
    
    template<typename A>
    _RepeatC<typename A::Lazy> RepeatC(const A a) {
	return _RepeatC<typename A::Lazy>(a);
    }

    template<typename A>
    struct _RepeatT {
	typedef _RepeatT<typename A::Lazy> Lazy;
	const A a;
	_RepeatT(const A a_) : a(a_) {}
	float operator()(int x, int y, int t, int c) const {
	    return a(x, y, 0, c);
	}

	int getWidth() const {return a.getWidth();}
	int getHeight() const {return a.getHeight();}
	int getFrames() const {return 0;}
	int getChannels() const {return a.getChannels();}

	typedef typename A::Iter Iter;
	Iter scanline(int y, int t, int c) const {
	    return a.scanline(y, 0, c);
	}
    };
    
    template<typename A>
    _RepeatT<typename A::Lazy> RepeatT(const A a) {
	return _RepeatT<typename A::Lazy>(a);
    }

    template<typename A>
    struct _RepeatY {
	typedef _RepeatY<typename A::Lazy> Lazy;
	const A a;
	_RepeatY(const A a_) : a(a_) {}
	float operator()(int x, int y, int t, int c) const {
	    return a(x, 0, t, c);
	}

	int getWidth() const {return a.getWidth();}
	int getHeight() const {return 0;}
	int getFrames() const {return a.getFrames();}
	int getChannels() const {return a.getChannels();}

	typedef typename A::Iter Iter;
	Iter scanline(int y, int t, int c) const {
	    return a.scanline(0, t, c);
	}
    };
    
    template<typename A>
    _RepeatY<typename A::Lazy> RepeatY(const A a) {
	return _RepeatY<typename A::Lazy>(a);
    }

    template<typename A>
    struct _RepeatX {
	typedef _RepeatX<typename A::Lazy> Lazy;
	const A a;
	_RepeatX(const A a_) : a(a_) {}
	float operator()(int x, int y, int t, int c) const {
	    return a(0, y, t, c);
	}

	int getWidth() const {return 0;}
	int getHeight() const {return a.getHeight();}
	int getFrames() const {return a.getFrames();}
	int getChannels() const {return a.getChannels();}

	typedef typename Const::Iter Iter;
	Iter scanline(int y, int t, int c) const {
	    return Const::Iter(a(0, y, t, c));
	}
    };
    
    template<typename A>
    _RepeatX<typename A::Lazy> RepeatX(const A a) {
	return _RepeatX<typename A::Lazy>(a);
    }

    template<typename A, typename B, typename C>
    struct TernaryOp {
	const A a;
	const B b;
	const C c;

	TernaryOp(const A a_, const B b_, const C c_) : a(a_), b(b_), c(c_) {
	    int xs = a.getWidth();
	    if (!xs) xs = b.getWidth();
	    if (!xs) xs = c.getWidth();	    
	    int ys = a.getHeight();
	    if (!ys) ys = b.getHeight();
	    if (!ys) ys = c.getHeight();	    
	    int ts = a.getFrames();
	    if (!ts) ts = b.getFrames();
	    if (!ts) ts = c.getFrames();	    
	    int cs = a.getChannels();
	    if (!cs) cs = b.getFrames();
	    if (!cs) cs = c.getFrames();	    
	    assert((a.getWidth() == 0 || a.getWidth() == xs) &&
		   (b.getWidth() == 0 || b.getWidth() == xs) &&
		   (c.getWidth() == 0 || c.getWidth() == xs),
		   "Can only combine images with matching width\n");
	    assert((a.getHeight() == 0 || a.getHeight() == ys) &&
		   (b.getHeight() == 0 || b.getHeight() == ys) &&
		   (c.getHeight() == 0 || c.getHeight() == ys),
		   "Can only combine images with matching height\n");
	    assert((a.getFrames() == 0 || a.getFrames() == ts) &&
		   (b.getFrames() == 0 || b.getFrames() == ts) &&
		   (c.getFrames() == 0 || c.getFrames() == ts),
		   "Can only combine images with matching frames\n");
	    assert((a.getChannels() == 0 || a.getChannels() == cs) &&
		   (b.getChannels() == 0 || b.getChannels() == cs) &&
		   (c.getChannels() == 0 || c.getChannels() == cs),
		   "Can only combine images with matching channels\n");	    
	}


	int getWidth() const {
	    if (a.getWidth()) return a.getWidth();
	    if (b.getWidth()) return b.getWidth();
	    if (c.getWidth()) return c.getWidth();
	    return 0;
	}
	int getHeight() const {
	    if (a.getHeight()) return a.getHeight();
	    if (b.getHeight()) return b.getHeight();
	    if (c.getHeight()) return c.getHeight();
	    return 0;
	}
	int getFrames() const {
	    if (a.getFrames()) return a.getFrames();
	    if (b.getFrames()) return b.getFrames();
	    if (c.getFrames()) return c.getFrames();
	    return 0;
	}
	int getChannels() const {
	    if (a.getChannels()) return a.getChannels();
	    if (b.getChannels()) return b.getChannels();
	    if (c.getChannels()) return c.getChannels();
	    return 0;
	}

    };
    
    template<typename A, typename B, typename C>
    struct _Select : public TernaryOp<A, B, C> {
	typedef _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy> Lazy;
	_Select(const A a_, const B b_, const C c_) : TernaryOp<A, B, C>(a_, b_, c_) {}

	float operator()(int x, int y, int t, int c_) const {
	    return (TernaryOp<A, B, C>::a(x, y, t, c_) ? 
		    TernaryOp<A, B, C>::b(x, y, t, c_) : 
		    TernaryOp<A, B, C>::c(x, y, t, c_));
	}

	struct Iter {
	    typename A::Iter a;
	    typename B::Iter b;
	    typename C::Iter c;
	    float operator[](int x) const {
		return a[x] ? b[x] : c[x];
	    }
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		const vec_type va = a.vec_bool(x);
		const vec_type vb = b.vec(x);
		const vec_type vc = c.vec(x);
		#ifdef __AVX__
		return _mm256_blendv_ps(vc, vb, va);
                #else
		// requires SSE4
		return _mm_blendv_ps(vc, vb, va);
		#endif
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	    
	};
	Iter scanline(int y, int t, int c_) const {
	    return {TernaryOp<A, B, C>::a.scanline(y, t, c_), 
		    TernaryOp<A, B, C>::b.scanline(y, t, c_), 
		    TernaryOp<A, B, C>::c.scanline(y, t, c_)};
	}
    };

    template<typename A, typename B, typename C>
    _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy> 
    Select(const A a, const B b, const C c) {
	return _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy>(a, b, c);
    }

    template<typename A, typename C>
    _Select<typename A::Lazy, Const, typename C::Lazy> 
    Select(const A a, float b, const C c) {
	return _Select<typename A::Lazy, Const, typename C::Lazy>(a, Const(b), c);
    }

    template<typename A, typename B>
    _Select<typename A::Lazy, typename B::Lazy, Const> 
    Select(const A a, const B b, float c) {
	return _Select<typename A::Lazy, typename B::Lazy, Const>(a, b, Const(c));
    }

    template<typename A>
    _Select<typename A::Lazy, Const, Const>
    Select(const A a, float b, float c) {
	return _Select<typename A::Lazy, Const, Const>(a, Const(b), Const(c));
    }

    template<typename A, typename B, typename C>
    struct _IfThenElse : public TernaryOp<A, B, C> {
	typedef _IfThenElse<typename A::Lazy, typename B::Lazy, typename C::Lazy> Lazy;
	_IfThenElse(const A a_, const B b_, const C c_) : TernaryOp<A, B, C>(a_, b_, c_) {}

	float operator()(int x, int y, int t, int c) const {
	    return (TernaryOp<A, B, C>::a(x, y, t, c) ? 
		    TernaryOp<A, B, C>::b(x, y, t, c) : 
		    TernaryOp<A, B, C>::c(x, y, t, c));
	}

	struct Iter {
	    typename A::Iter a;
	    typename B::Iter b;
	    typename C::Iter c;
	    float operator[](int x) const {
		return a[x] ? b[x] : c[x];
	    }
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		union {		   
		    float f[vec_width];
		    vec_type v;
		} vres;
		for (int i = 0; i < vec_width; i++) {
		    if (a[x+i]) vres.f[i] = b[x+i];
		    else vres.f[i] = c[x+i];
		}
		return vres.v;
	    }
	    vec_type vec_bool(int x) const {
		return vec_is_non_zero(vec(x));
	    }
            #endif
	    
	};
	Iter scanline(int y, int t, int c_) const {
	    return {TernaryOp<A, B, C>::a.scanline(y, t, c_), 
		    TernaryOp<A, B, C>::b.scanline(y, t, c_), 
		    TernaryOp<A, B, C>::c.scanline(y, t, c_)};
	}
    };

    template<typename A, typename B, typename C>
    _IfThenElse<typename A::Lazy, typename B::Lazy, typename C::Lazy> 
    IfThenElse(const A a, const B b, const C c) {
	return _IfThenElse<typename A::Lazy, typename B::Lazy, typename C::Lazy>(a, b, c);
    }

    template<typename A, typename C>
    _IfThenElse<typename A::Lazy, Const, typename C::Lazy> 
    IfThenElse(const A a, const float b, const C c) {
	return _IfThenElse<typename A::Lazy, Const, typename C::Lazy>(a, Const(b), c);
    }

    template<typename A, typename B>
    _IfThenElse<typename A::Lazy, typename B::Lazy, Const> 
    IfThenElse(const A a, const B b, const float c) {
	return _IfThenElse<typename A::Lazy, typename B::Lazy, Const>(a, b, Const(c));
    }

    template<typename A>
    _IfThenElse<typename A::Lazy, Const, Const>
    IfThenElse(const A a, const float b, const float c) {
	return _IfThenElse<typename A::Lazy, Const, Const>(a, Const(b), Const(c));
    }
}


template<typename A, typename B>
Lazy::Add<typename A::Lazy, typename B::Lazy> operator+(const A a, const B b) {
    return Lazy::Add<A, B>(a, b);
}

template<typename B>
Lazy::Add<Lazy::Const, typename B::Lazy> operator+(const float a, const B b) {
    return Lazy::Add<Lazy::Const, B>(Lazy::Const(a), b);
}

template<typename A>
Lazy::Add<typename A::Lazy, Lazy::Const> operator+(const A a, const float b) {
    return Lazy::Add<A, Lazy::Const>(a, Lazy::Const(b));
}


template<typename A, typename B>
Lazy::Sub<typename A::Lazy, typename B::Lazy> operator-(const A a, const B b) {
    return Lazy::Sub<A, B>(a, b);
}

template<typename B>
Lazy::Sub<Lazy::Const, typename B::Lazy> operator-(const float a, const B b) {
    return Lazy::Sub<Lazy::Const, B>(Lazy::Const(a), b);
}

template<typename A>
Lazy::Sub<typename A::Lazy, Lazy::Const> operator-(const A a, const float b) {
    return Lazy::Sub<A, Lazy::Const>(a, Lazy::Const(b));
}

template<typename A>
Lazy::Sub<Lazy::Const, typename A::Lazy> operator-(const A a) {
    return Lazy::Sub<Lazy::Const, A>(0, a);
}

template<typename A, typename B>
Lazy::Mul<typename A::Lazy, typename B::Lazy> operator*(const A a, const B b) {
    return Lazy::Mul<A, B>(a, b);
}

template<typename B>
Lazy::Mul<Lazy::Const, typename B::Lazy> operator*(const float a, const B b) {
    return Lazy::Mul<Lazy::Const, B>(Lazy::Const(a), b);
}

template<typename A>
Lazy::Mul<typename A::Lazy, Lazy::Const> operator*(const A a, const float b) {
    return Lazy::Mul<A, Lazy::Const>(a, Lazy::Const(b));
}

template<typename A, typename B>
Lazy::Div<typename A::Lazy, typename B::Lazy> operator/(const A a, const B b) {
    return Lazy::Div<A, B>(a, b);
}

template<typename B>
Lazy::Div<Lazy::Const, typename B::Lazy> operator/(const float a, const B b) {
    return Lazy::Div<Lazy::Const, B>(Lazy::Const(a), b);
}

template<typename A>
Lazy::Mul<typename A::Lazy, Lazy::Const> operator/(const A a, const float b) {
    // replace expr / const with expr * (1/const)
    return Lazy::Mul<A, Lazy::Const>(a, Lazy::Const(1.0f/b));
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::GT> operator>(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::GT>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::GT> operator>(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::GT>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::GT> operator>(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::GT>(Lazy::Const(a), b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::LT> operator<(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::LT>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::LT> operator<(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::LT>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::LT> operator<(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::LT>(Lazy::Const(a), b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::GE> operator>=(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::GE>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::GE> operator>=(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::GE>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::GE> operator>=(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::GE>(Lazy::Const(a), b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::LE> operator<=(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::LE>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::LE> operator<=(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::LE>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::LE> operator<=(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::LE>(Lazy::Const(a), b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::EQ> operator==(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::EQ>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::EQ> operator==(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::EQ>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::EQ> operator==(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::EQ>(Lazy::Const(a), b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::NEQ> operator!=(const A a, const B b) {
    return Lazy::Cmp<A, B, Lazy::NEQ>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::NEQ> operator!=(const A a, const float b) {
    return Lazy::Cmp<A, Lazy::Const, Lazy::NEQ>(a, Lazy::Const(b));
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::NEQ> operator!=(const float a, const B b) {
    return Lazy::Cmp<Lazy::Const, B, Lazy::NEQ>(Lazy::Const(a), b);
}

#include "footer.h"

#endif
