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
	return _mm_cmp_eq_ps(va, va);
    }
    static const vec_type vec_zero() {
	return _mm_setzero_ps();
    }
    static const vec_type vec_is_non_zero(vec_type v) {
	return _mm_cmp_neq_ps(v, vec_zero());
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
		return _mm_set_ss(0, 0, 0, u.f_high);
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
		// use stack storage to pass arguments
		vec_type vb[1] = {b.vec(x)};
		float * const fv = (float *)&(vb[0]);		
		fv[0] = (*fn)(fv[0]);
		fv[1] = (*fn)(fv[1]);
		fv[2] = (*fn)(fv[2]);
		fv[3] = (*fn)(fv[3]);
		#ifdef __AVX__
		fv[4] = (*fn)(fv[4]);
		fv[5] = (*fn)(fv[5]);
		fv[6] = (*fn)(fv[6]);
		fv[7] = (*fn)(fv[7]);
		#endif
		return vb[0];
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
    struct _Select {
	typedef _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy> Lazy;
	const A cond;
	const B thenCase;
	const C elseCase;
	_Select(const A a_, const B b_, const C c_) : cond(a_), thenCase(b_), elseCase(c_) {
	    int w = cond.getWidth();
	    if (!w) w = thenCase.getWidth();
	    if (!w) w = elseCase.getWidth();	    
	    int h = cond.getHeight();
	    if (!h) h = thenCase.getHeight();
	    if (!h) h = elseCase.getHeight();	    
	    int f = cond.getFrames();
	    if (!f) f = thenCase.getFrames();
	    if (!f) f = elseCase.getFrames();	    
	    int c = cond.getChannels();
	    if (!c) c = thenCase.getFrames();
	    if (!c) c = elseCase.getFrames();	    
	    assert((cond.getWidth() == 0 || cond.getWidth() == w) &&
		   (thenCase.getWidth() == 0 || thenCase.getWidth() == w) &&
		   (elseCase.getWidth() == 0 || elseCase.getWidth() == w),
		   "Can only combine images with matching width\n");
	    assert((cond.getHeight() == 0 || cond.getHeight() == h) &&
		   (thenCase.getHeight() == 0 || thenCase.getHeight() == h) &&
		   (elseCase.getHeight() == 0 || elseCase.getHeight() == h),
		   "Can only combine images with matching height\n");
	    assert((cond.getFrames() == 0 || cond.getFrames() == f) &&
		   (thenCase.getFrames() == 0 || thenCase.getFrames() == f) &&
		   (elseCase.getFrames() == 0 || elseCase.getFrames() == f),
		   "Can only combine images with matching frames\n");
	    assert((cond.getChannels() == 0 || cond.getChannels() == c) &&
		   (thenCase.getChannels() == 0 || thenCase.getChannels() == c) &&
		   (elseCase.getChannels() == 0 || elseCase.getChannels() == c),
		   "Can only combine images with matching channels\n");	    
	}
	float operator()(int x, int y, int t, int c) const {
	    return cond(x, y, t, c) ? thenCase(x, y, t, c) : elseCase(x, y, t, c);
	}

	int getWidth() const {
	    if (cond.getWidth()) return cond.getWidth();
	    if (thenCase.getWidth()) return thenCase.getWidth();
	    if (elseCase.getWidth()) return elseCase.getWidth();
	    return 0;
	}
	int getHeight() const {
	    if (cond.getHeight()) return cond.getHeight();
	    if (thenCase.getHeight()) return thenCase.getHeight();
	    if (elseCase.getHeight()) return elseCase.getHeight();
	    return 0;
	}
	int getFrames() const {
	    if (cond.getFrames()) return cond.getFrames();
	    if (thenCase.getFrames()) return thenCase.getFrames();
	    if (elseCase.getFrames()) return elseCase.getFrames();
	    return 0;
	}
	int getChannels() const {
	    if (cond.getChannels()) return cond.getChannels();
	    if (thenCase.getChannels()) return thenCase.getChannels();
	    if (elseCase.getChannels()) return elseCase.getChannels();
	    return 0;
	}

	struct Iter {
	    typename A::Iter cond;
	    typename B::Iter thenCase;
	    typename C::Iter elseCase;
	    float operator[](int x) const {
		return cond[x] ? thenCase[x] : elseCase[x];
	    }
            #ifdef VECTORIZE
	    vec_type vec(int x) const {
		const vec_type va = cond.vec_bool(x);
		const vec_type vb = thenCase.vec(x);
		const vec_type vc = elseCase.vec(x);
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
	Iter scanline(int y, int t, int c) const {
	    return {cond.scanline(y, t, c), 
		    thenCase.scanline(y, t, c), 
		    elseCase.scanline(y, t, c)};
	}
    };

    template<typename A, typename B, typename C>
    _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy> 
    Select(const A a, const B b, const C c) {
	return _Select<typename A::Lazy, typename B::Lazy, typename C::Lazy>(a, b, c);
    }

    template<typename A, typename C>
    _Select<typename A::Lazy, Const, typename C::Lazy> 
    Select(const A a, const float b, const C c) {
	return _Select<typename A::Lazy, Const, typename C::Lazy>(a, Const(b), c);
    }

    template<typename A, typename B>
    _Select<typename A::Lazy, typename B::Lazy, Const> 
    Select(const A a, const B b, const float c) {
	return _Select<typename A::Lazy, typename B::Lazy, Const>(a, b, Const(c));
    }

    template<typename A>
    _Select<typename A::Lazy, Const, Const>
    Select(const A a, const float b, const float c) {
	return _Select<typename A::Lazy, Const, Const>(a, Const(b), Const(c));
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



#include "footer.h"

#endif
