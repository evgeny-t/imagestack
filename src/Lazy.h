#ifndef IMAGESTACK_FUNC_H
#define IMAGESTACK_FUNC_H

#include <immintrin.h>
#include <stdint.h>
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

    namespace Vec {
#ifdef __AVX__
        typedef __m256 type;
        const int width = 8;
        
        static type broadcast(float v) {
            return _mm256_set1_ps(v);
        }

        // Arithmetic binary operators
        struct Add {
            static float scalar(float a, float b) {return a + b;}
            static type vec(type a, type b) {return _mm256_add_ps(a, b);}
        };
        struct Sub {
            static float scalar(float a, float b) {return a - b;}
            static type vec(type a, type b) {return _mm256_sub_ps(a, b);}
        };
        struct Mul {
            static float scalar(float a, float b) {return a * b;}
            static type vec(type a, type b) {return _mm256_mul_ps(a, b);}
        };
        struct Div {
            static float scalar(float a, float b) {return a / b;}
            static type vec(type a, type b) {return _mm256_div_ps(a, b);}
        };
        struct Min {
            static float scalar(float a, float b) {return a < b ? a : b;}
            static type vec(type a, type b) {return _mm256_min_ps(a, b);}
        };
        struct Max {
            static float scalar(float a, float b) {return a > b ? a : b;}
            static type vec(type a, type b) {return _mm256_max_ps(a, b);}
        };

        // Comparisons
        struct GT {
            static bool scalar(float a, float b) {return a > b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_GT_OQ);}        
        };
        struct LT {
            static bool scalar(float a, float b) {return a < b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_LT_OQ);}        
        };
        struct GE {
            static bool scalar(float a, float b) {return a >= b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_GE_OQ);}        
        };
        struct LE {
            static bool scalar(float a, float b) {return a <= b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_LE_OQ);}        
        };
        struct EQ {
            static bool scalar(float a, float b) {return a == b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);}        
        };
        struct NEQ {
            static bool scalar(float a, float b) {return a != b;}
            static type vec(type a, type b) {return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);}        
        };

        // Logical ops
        static type blend(type a, type b, type mask) {
            return _mm256_blendv_ps(a, b, mask);
        }

        // Other functions
        static type floor(type a) {
            return _mm256_floor_ps(a);
        }
        static type ceil(type a) {
            return _mm256_ceil_ps(a);
        }
        static type sqrt(type a) {
            return _mm256_sqrt_ps(a);
        }

        // Loads and stores
        static type load(const float *f) {
            return _mm256_loadu_ps(f);
        }

#else
#ifdef __SSE__
        typedef __m128 type;
        const int width = 4;

        static type broadcast(float v) {
            return _mm_set1_ps(v);
        }

        // Arithmetic binary operators
        struct Add {
            static float scalar(float a, float b) {return a + b;}
            static type vec(type a, type b) {return _mm_add_ps(a, b);}
        };
        struct Sub {
            static float scalar(float a, float b) {return a - b;}
            static type vec(type a, type b) {return _mm_sub_ps(a, b);}
        };
        struct Mul {
            static float scalar(float a, float b) {return a * b;}
            static type vec(type a, type b) {return _mm_mul_ps(a, b);}
        };
        struct Div {
            static float scalar(float a, float b) {return a / b;}
            static type vec(type a, type b) {return _mm_div_ps(a, b);}
        };
        struct Min {
            static float scalar(float a, float b) {return a < b ? a : b;}
            static type vec(type a, type b) {return _mm_min_ps(a, b);}
        };
        struct Max {
            static float scalar(float a, float b) {return a > b ? a : b;}
            static type vec(type a, type b) {return _mm_max_ps(a, b);}
        };

        // Comparisons
        struct GT {
            static bool scalar(float a, float b) {return a > b;}
            static type vec(type a, type b) {return _mm_cmpgt_ps(a, b);}        
        };
        struct LT {
            static bool scalar(float a, float b) {return a < b;}
            static type vec(type a, type b) {return _mm_cmplt_ps(a, b);}        
        };
        struct GE {
            static bool scalar(float a, float b) {return a >= b;}
            static type vec(type a, type b) {return _mm_cmpge_ps(a, b);}        
        };
        struct LE {
            static bool scalar(float a, float b) {return a <= b;}
            static type vec(type a, type b) {return _mm_cmple_ps(a, b);}        
        };
        struct EQ {
            static bool scalar(float a, float b) {return a == b;}
            static type vec(type a, type b) {return _mm_cmpeq_ps(a, b);}        
        };
        struct NEQ {
            static bool scalar(float a, float b) {return a != b;}
            static type vec(type a, type b) {return _mm_cmpneq_ps(a, b);}        
        };

        #ifdef __SSE4_1__
        // Logical ops
        static type blend(type a, type b, type mask) {
            return _mm_blendv_ps(a, b, mask);
        }

        // Other functions
        static type floor(type a) {
            return _mm_floor_ps(a);
        }
        static type ceil(type a) {
            return _mm_ceil_ps(a);
        }
        static type sqrt(type a) {
            return _mm_sqrt_ps(a);
        }
        #else

        static type blend(type a, type b, type mask) {
            return _mm_or_ps(_mm_and_ps(mask, b), 
                             _mm_andnot_ps(mask, a));
        }        

        #endif

        // Loads and stores
        static type load(const float *f) {
            return _mm_loadu_ps(f);
        }
        

#else
// scalar fallback
        typedef float type;
        const int width = 1;

        static type broadcast(float v) {
            return v;
        }

        // Arithmetic binary operators
        struct Add {
            static float scalar(float a, float b) {return a + b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };
        struct Sub {
            static float scalar(float a, float b) {return a - b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };
        struct Mul {
            static float scalar(float a, float b) {return a * b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };
        struct Div {
            static float scalar(float a, float b) {return a / b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };
        struct Min {
            static float scalar(float a, float b) {return a < b ? a : b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };
        struct Max {
            static float scalar(float a, float b) {return a > b ? a : b;}
            static type vec(type a, type b) {return scalar(a, b);}
        };

        // Comparisons
        struct GT {
            static bool scalar(float a, float b) {return a > b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };
        struct LT {
            static bool scalar(float a, float b) {return a < b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };
        struct GE {
            static bool scalar(float a, float b) {return a >= b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };
        struct LE {
            static bool scalar(float a, float b) {return a <= b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };
        struct EQ {
            static bool scalar(float a, float b) {return a == b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };
        struct NEQ {
            static bool scalar(float a, float b) {return a != b;}
            static bool vec(type a, type b) {return scalar(a, b);}
        };

        // Logical ops
        static type blend(bool mask, type a, type b) {
            return (mask ? b : a);
        }

        // Other functions
        static type floor(type a) {
            return floorf(a);
        }
        static type ceil(type a) {
            return ceilf(a);
        }
        static type sqrt(type a) {
            return sqrtf(a);
        }

        // Loads and stores
        static type load(const float *f) {
            return *f;
        }

#endif 
#endif
    }

    // A base class for things which do not depend on image data
    struct Unbounded {
        int getSize(int) const {return 0;}
    };

    // Constants
    struct Const : public Unbounded {
        typedef Const Lazy;
        const float val;
        Const(const float val_) : val(val_) {}
        float operator()(int x, int, int, int) const {return val;}

        // State needed to iterate across a scanline    
        struct Iter {
            Iter (float v) : val(v) {
                vec_val = Vec::broadcast(val);
            }
            const float val;
            float operator[](int x) const {return val;}
            Vec::type vec_val;
            Vec::type vec(int x) const {return vec_val;}
        };
        Iter scanline(int y, int t, int c) const {
            return Iter(val);
        }
    };


    // Coordinates
    struct X : public Unbounded {
        typedef X Lazy;
        float operator()(int x, int, int, int) const {return x;}

        // State needed to iterate across a scanline    
        struct Iter {
            float operator[](int x) const {return x;}
            Vec::type vec(int x) const {
                union {            
                    float f[Vec::width];
                    Vec::type v;
                } v;
                for (int i = 0; i < Vec::width; i++) {
                    v.f[i] = x+i;
                }
                return v.v;
            }
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

    // Arithmetic binary operators
    template<typename A, typename B, typename Op>
    struct BinaryOp {
        typedef BinaryOp<typename A::Lazy, typename B::Lazy, Op> Lazy;
        const A a;
        const B b;

        BinaryOp(const A a_, const B b_) : a(a_), b(b_) {
            for (int i = 0; i < 4; i++) {
                if (a.getSize(i) && b.getSize(i)) {
                    assert(a.getSize(i) == b.getSize(i), 
                           "Can only combine images with matching size\n");                           
                }
            }
        }

        int getSize(int i) const {
            if (a.getSize(i)) return a.getSize(i);
            return b.getSize(i);
        }

        float operator()(int x, int y, int t, int c) const {
            return Op::scalar(a(x, y, t, c), b(x, y, t, c));
        }

        struct Iter {
            typename A::Iter a;
            typename B::Iter b;
            Iter(typename A::Iter a_, typename B::Iter b_) : a(a_), b(b_) {}
            float operator[](int x) const {
                return Op::scalar(a[x], b[x]);
            }
            Vec::type vec(int x) const {
                return Op::vec(a.vec(x), b.vec(x));
            }
        };
        Iter scanline(int y, int t, int c) const {
            return Iter(a.scanline(y, t, c), b.scanline(y, t, c));
        }
    };

    // Comparison binary operators
    template<typename A, typename B, typename Op>
    struct Cmp {
        typedef Cmp<typename A::Lazy, typename B::Lazy, Op> LazyBool;
        const A a;
        const B b;

        Cmp(const A a_, const B b_) : a(a_), b(b_) {
            for (int i = 0; i < 4; i++) {
                if (a.getSize(i) && b.getSize(i)) {
                    assert(a.getSize(i) == b.getSize(i), 
                           "Can only combine images with matching size\n");                           
                }
            }
        }
        int getSize(int i) const {
            if (a.getSize(i)) return a.getSize(i);
            return b.getSize(i);
        }

        float operator()(int x, int y, int t, int c) const {
            return Op::scalar(a(x, y, t, c), b(x, y, t, c)) ? 1 : 0;
        }
        
        struct Iter {
            typename A::Iter a; 
            typename B::Iter b;
            Iter(typename A::Iter a_, typename B::Iter b_) : a(a_), b(b_) {}
            float operator[](int x) const {
                return Op::scalar(a[x], b[x]) ? 1 : 0;
            }
            Vec::type vec(int x) const {
                return Op::vec(a.vec(x), b.vec(x));
            }
        };
        Iter scanline(int y, int t, int c) const {
            return Iter(a.scanline(y, t, c), 
                        b.scanline(y, t, c));
        }
    };
    


    template<typename A, typename B>
    BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Min>
    min(const A a, const B b) {
        return BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Min>(a, b);
    }
    template<typename A>
    BinaryOp<typename A::Lazy, Const, Vec::Min>
    min(const A a, float b) {
        return min(a, Const(b));
    }
    template<typename B>
    BinaryOp<Const, typename B::Lazy, Vec::Min>
    min(float a, const B b) {
        return min(Const(a), b);
    }
    template<typename A, typename B>
    BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Max>
    max(const A a, const B b) {
        return BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Max>(a, b);
    }
    template<typename A>
    BinaryOp<typename A::Lazy, Const, Vec::Max>
    max(const A a, float b) {
        return max(a, Const(b));
    }
    template<typename B>
    BinaryOp<Const, typename B::Lazy, Vec::Max>
    max(float a, const B b) {
        return max(Const(a), b);
    }
   
    template<typename A, typename B, typename C>
    BinaryOp<BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Max>, typename C::Lazy, Vec::Min>  
    clamp(const A a, const B b, const C c) {
        return min(max(a, b), c);
    }
    template<typename B, typename C>
    BinaryOp<BinaryOp<Const, typename B::Lazy, Vec::Max>, typename C::Lazy, Vec::Min>  
    clamp(float a, const B b, const C c) {
        return min(max(a, b), c);
    }
    template<typename A, typename C>
    BinaryOp<BinaryOp<typename A::Lazy, Const, Vec::Max>, typename C::Lazy, Vec::Min>  
    clamp(const A a, float b, const C c) {
        return min(max(a, b), c);
    }
    template<typename A, typename B>
    BinaryOp<BinaryOp<typename A::Lazy, typename B::Lazy, Vec::Max>, Const, Vec::Min>  
    clamp(const A a, const B b, float c) {
        return min(max(a, b), c);
    }
    template<typename A>
    BinaryOp<BinaryOp<typename A::Lazy, Const, Vec::Max>, Const, Vec::Min>  
    clamp(const A a, float b, float c) {
        return min(max(a, b), c);
    }

    // Lift a unary function over floats to the same function over an image (e.g. cosf)
    template<float (*fn)(float), typename A>
    struct Lift {
        typedef Lift<fn, typename A::Lazy> Lazy;
        const A a;
        Lift(const A a_) : a(a_) {}
        float operator()(int x, int y, int t, int c) const {
            return (*fn)(a(x, y, t, c));
        }

        int getSize(int i) const {return a.getSize(i);}

        struct Iter {
            typename A::Iter a;
            Iter(typename A::Iter a_) : a(a_) {}
            float operator[](int x) const {return (*fn)(a[x]);}
            Vec::type vec(int x) const {
                union {            
                    float f[Vec::width];
                    Vec::type v;
                } va;
                va.v = a.vec(x);
                for (int i = 0; i < Vec::width; i++) {
                    va.f[i] = (*fn)(va.f[i]);
                }
                return va.v;
            }
        };
        Iter scanline(int y, int t, int c) const {
            return Iter(a.scanline(y, t, c));
        }
    };

    // Arithmetic binary operators
    template<float (*fn)(float, float), typename A, typename B>
    struct Lift2 {
        typedef Lift2<fn, typename A::Lazy, typename B::Lazy> Lazy;
        const A a;
        const B b;

        Lift2(const A a_, const B b_) : a(a_), b(b_) {
            for (int i = 0; i < 4; i++) {
                if (a.getSize(i) && b.getSize(i)) {
                    assert(a.getSize(i) == b.getSize(i), 
                           "Can only combine images with matching size\n");                           
                }
            }
        }

        int getSize(int i) const {
            if (a.getSize(i)) return a.getSize(i);
            return b.getSize(i);
        }

        float operator()(int x, int y, int t, int c) const {
            return (*fn)(a(x, y, t, c), b(x, y, t, c));
        }

        struct Iter {
            typename A::Iter a;
            typename B::Iter b;
            Iter(typename A::Iter a_, typename B::Iter b_) : a(a_), b(b_) {}
            float operator[](int x) const {
                return (*fn)(a[x], b[x]);
            }
            Vec::type vec(int x) const {
                union {            
                    float f[Vec::width];
                    Vec::type v;
                } va, vb;           
                va.v = a.vec(x);
                vb.v = b.vec(x);
                for (int i = 0; i < Vec::width; i++) {
                    vb.f[i] = (*fn)(va.f[i], vb.f[i]);
                }
                return vb.v;
            }
        };
        Iter scanline(int y, int t, int c) const {
            return Iter(a.scanline(y, t, c), b.scanline(y, t, c));
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

        int getSize(int i) const {return i == 3 ? 0 : a.getSize(i);}

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

        int getSize(int i) const {return i == 2 ? 0 : a.getSize(i);}

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

        int getSize(int i) const {return i == 1 ? 0 : a.getSize(i);}

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

        int getSize(int i) const {return i == 0 ? 0 : a.getSize(i);}

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
        typedef _Select<typename A::LazyBool, typename B::Lazy, typename C::Lazy> Lazy;
        const A a;
        const B b;
        const C c;

        _Select(const A a_, const B b_, const C c_) : a(a_), b(b_), c(c_) {
            for (int i = 0; i < 4; i++) {
                int s = a.getSize(i);
                if (!s) s = b.getSize(i);
                if (!s) s = c.getSize(i);                    
                assert((a.getSize(i) == s || a.getSize(i) == 0) &&
                       (b.getSize(i) == s || b.getSize(i) == 0) &&
                       (c.getSize(i) == s || c.getSize(i) == 0), 
                       "Can only combine images with matching size\n");
            }
        }

        int getSize(int i) const {
            if (a.getSize(i)) return a.getSize(i);
            if (b.getSize(i)) return b.getSize(i);
            if (c.getSize(i)) return c.getSize(i);
            return 0;
        }

        float operator()(int x, int y, int t, int c_) const {
            return (a(x, y, t, c_) ? b(x, y, t, c_) : c(x, y, t, c_));
        }

        struct Iter {
            typename A::Iter a;
            typename B::Iter b;
            typename C::Iter c;
            Iter(typename A::Iter a_,
                 typename B::Iter b_,
                 typename C::Iter c_) : a(a_), b(b_), c(c_) {}
            float operator[](int x) const {
                return a[x] ? b[x] : c[x];
            }
            Vec::type vec(int x) const {
                const Vec::type va = a.vec(x);
                const Vec::type vb = b.vec(x);
                const Vec::type vc = c.vec(x);
                return Vec::blend(vc, vb, va);
            }
        };
        Iter scanline(int y, int t, int c_) const {
            return Iter(a.scanline(y, t, c_), 
                        b.scanline(y, t, c_), 
                        c.scanline(y, t, c_));
        }
    };

    template<typename A, typename B, typename C>
    _Select<typename A::LazyBool, typename B::Lazy, typename C::Lazy> 
    Select(const A a, const B b, const C c) {
        return _Select<typename A::LazyBool, typename B::Lazy, typename C::Lazy>(a, b, c);
    }

    template<typename A, typename C>
    _Select<typename A::LazyBool, Const, typename C::Lazy> 
    Select(const A a, float b, const C c) {
        return _Select<typename A::LazyBool, Const, typename C::Lazy>(a, Const(b), c);
    }

    template<typename A, typename B>
    _Select<typename A::LazyBool, typename B::Lazy, Const> 
    Select(const A a, const B b, float c) {
        return _Select<typename A::LazyBool, typename B::Lazy, Const>(a, b, Const(c));
    }

    template<typename A>
    _Select<typename A::LazyBool, Const, Const>
    Select(const A a, float b, float c) {
        return _Select<typename A::LazyBool, Const, Const>(a, Const(b), Const(c));
    }

    template<typename A, typename B, typename C>
    struct _IfThenElse {
        typedef _IfThenElse<typename A::LazyBool, typename B::Lazy, typename C::Lazy> Lazy;
        const A a;
        const B b;
        const C c;

        _IfThenElse(const A a_, const B b_, const C c_) : a(a_), b(b_), c(c_) {
            for (int i = 0; i < 4; i++) {
                int s = a.getSize(i);
                if (!s) s = b.getSize(i);
                if (!s) s = c.getSize(i);                    
                assert((a.getSize(i) == s || a.getSize(i) == 0) &&
                       (b.getSize(i) == s || b.getSize(i) == 0) &&
                       (c.getSize(i) == s || c.getSize(i) == 0), 
                       "Can only combine images with matching size\n");
            }
        }

        int getSize(int i) const {
            if (a.getSize(i)) return a.getSize(i);
            if (b.getSize(i)) return b.getSize(i);
            if (c.getSize(i)) return c.getSize(i);
            return 0;
        }

        float operator()(int x, int y, int t, int c_) const {
            return (a(x, y, t, c_) ? b(x, y, t, c_) : c(x, y, t, c_));
        }

        struct Iter {
            typename A::Iter a;
            typename B::Iter b;
            typename C::Iter c;
            Iter(typename A::Iter a_,
                 typename B::Iter b_,
                 typename C::Iter c_) : a(a_), b(b_), c(c_) {}
            float operator[](int x) const {
                return a[x] ? b[x] : c[x];
            }
            Vec::type vec(int x) const {
                union {            
                    float f[Vec::width];
                    Vec::type v;
                } vres;
                for (int i = 0; i < Vec::width; i++) {
                    if (a[x+i]) vres.f[i] = b[x+i];
                    else vres.f[i] = c[x+i];
                }
                return vres.v;
            }
        };
        Iter scanline(int y, int t, int c_) const {
            return Iter(a.scanline(y, t, c_), 
                        b.scanline(y, t, c_), 
                        c.scanline(y, t, c_));
        }
    };

    template<typename A, typename B, typename C>
    _IfThenElse<typename A::LazyBool, typename B::Lazy, typename C::Lazy> 
    IfThenElse(const A a, const B b, const C c) {
        return _IfThenElse<typename A::LazyBool, typename B::Lazy, typename C::Lazy>(a, b, c);
    }

    template<typename A, typename C>
    _IfThenElse<typename A::LazyBool, Const, typename C::Lazy> 
    IfThenElse(const A a, const float b, const C c) {
        return _IfThenElse<typename A::LazyBool, Const, typename C::Lazy>(a, Const(b), c);
    }

    template<typename A, typename B>
    _IfThenElse<typename A::LazyBool, typename B::Lazy, Const> 
    IfThenElse(const A a, const B b, const float c) {
        return _IfThenElse<typename A::LazyBool, typename B::Lazy, Const>(a, b, Const(c));
    }

    template<typename A>
    _IfThenElse<typename A::LazyBool, Const, Const>
    IfThenElse(const A a, const float b, const float c) {
        return _IfThenElse<typename A::LazyBool, Const, Const>(a, Const(b), Const(c));
    }
}


template<typename A, typename B>
Lazy::BinaryOp<typename A::Lazy, typename B::Lazy, Lazy::Vec::Add> operator+(A a, B b) {
    return Lazy::BinaryOp<typename A::Lazy, typename B::Lazy, Lazy::Vec::Add>(a, b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Add> operator+(float a, B b) {
    return Lazy::Const(a) + b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Add> operator+(A a, float b) {
    return a + Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Add> operator+(double a, B b) {
    return Lazy::Const(a) + b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Add> operator+(A a, double b) {
    return a + Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Add> operator+(int a, B b) {
    return Lazy::Const(a) + b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Add> operator+(A a, int b) {
    return a + Lazy::Const(b);
}

template<typename A, typename B>
Lazy::BinaryOp<typename A::Lazy, typename B::Lazy, Lazy::Vec::Sub> operator-(A a, B b) {
    return Lazy::BinaryOp<A, B, Lazy::Vec::Sub>(a, b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Sub> operator-(float a, B b) {
    return Lazy::Const(a) - b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Sub> operator-(A a, float b) {
    return a - Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Sub> operator-(double a, B b) {
    return Lazy::Const(a) - b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Sub> operator-(A a, double b) {
    return a - Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Sub> operator-(int a, B b) {
    return Lazy::Const(a) - b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Sub> operator-(A a, int b) {
    return a - Lazy::Const(b);
}

template<typename A>
Lazy::BinaryOp<Lazy::Const, typename A::Lazy, Lazy::Vec::Sub> operator-(A a) {
    return Lazy::Const(0) - a;
}

template<typename A, typename B>
Lazy::BinaryOp<typename A::Lazy, typename B::Lazy, Lazy::Vec::Mul> operator*(A a, B b) {
    return Lazy::BinaryOp<A, B, Lazy::Vec::Mul>(a, b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Mul> operator*(float a, B b) {
    return Lazy::Const(a)*b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator*(A a, float b) {
    return a*Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Mul> operator*(double a, B b) {
    return Lazy::Const(a)*b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator*(A a, double b) {
    return a*Lazy::Const(b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Mul> operator*(int a, B b) {
    return Lazy::Const(a)*b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator*(A a, int b) {
    return a*Lazy::Const(b);
}

template<typename A, typename B>
Lazy::BinaryOp<typename A::Lazy, typename B::Lazy, Lazy::Vec::Div> operator/(A a, B b) {
    return Lazy::BinaryOp<A, B, Lazy::Vec::Div>(a, b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Div> operator/(float a, B b) {
    return Lazy::Const(a)/b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator/(A a, float b) {
    // replace expr / const with expr * (1/const)
    return a * Lazy::Const(1.0f/b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Div> operator/(double a, B b) {
    return Lazy::Const(a)/b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator/(A a, double b) {
    // replace expr / const with expr * (1/const)
    return a * Lazy::Const(1.0f/b);
}

template<typename B>
Lazy::BinaryOp<Lazy::Const, typename B::Lazy, Lazy::Vec::Div> operator/(int a, B b) {
    return Lazy::Const(a)/b;
}

template<typename A>
Lazy::BinaryOp<typename A::Lazy, Lazy::Const, Lazy::Vec::Mul> operator/(A a, int b) {
    // replace expr / const with expr * (1/const)
    return a * Lazy::Const(1.0f/b);
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::GT> operator>(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::GT>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GT> operator>(A a, float b) {
    return a > Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GT> operator>(float a, B b) {
    return Lazy::Const(a) > b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GT> operator>(A a, double b) {
    return a > Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GT> operator>(double a, B b) {
    return Lazy::Const(a) > b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GT> operator>(A a, int b) {
    return a > Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GT> operator>(int a, B b) {
    return Lazy::Const(a) > b;
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::LT> operator<(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::LT>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LT> operator<(A a, float b) {
    return a < Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LT> operator<(float a, B b) {
    return Lazy::Const(a) < b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LT> operator<(A a, double b) {
    return a < Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LT> operator<(double a, B b) {
    return Lazy::Const(a) < b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LT> operator<(A a, int b) {
    return a < Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LT> operator<(int a, B b) {
    return Lazy::Const(a) < b;
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::GE> operator>=(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::GE>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GE> operator>=(A a, float b) {
    return a >= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GE> operator>=(float a, B b) {
    return Lazy::Const(a) >= b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GE> operator>=(A a, double b) {
    return a >= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GE> operator>=(double a, B b) {
    return Lazy::Const(a) >= b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::GE> operator>=(A a, int b) {
    return a >= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::GE> operator>=(int a, B b) {
    return Lazy::Const(a) >= b;
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::LE> operator<=(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::LE>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LE> operator<=(A a, float b) {
    return a <= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LE> operator<=(float a, B b) {
    return Lazy::Const(a) <= b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LE> operator<=(A a, double b) {
    return a <= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LE> operator<=(double a, B b) {
    return Lazy::Const(a) <= b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::LE> operator<=(A a, int b) {
    return a <= Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::LE> operator<=(int a, B b) {
    return Lazy::Const(a) <= b;
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::EQ> operator==(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::EQ>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::EQ> operator==(A a, float b) {
    return a == Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::EQ> operator==(float a, B b) {
    return Lazy::Const(a) == b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::EQ> operator==(A a, double b) {
    return a == Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::EQ> operator==(double a, B b) {
    return Lazy::Const(a) == b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::EQ> operator==(A a, int b) {
    return a == Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::EQ> operator==(int a, B b) {
    return Lazy::Const(a) == b;
}

template<typename A, typename B>
Lazy::Cmp<typename A::Lazy, typename B::Lazy, Lazy::Vec::NEQ> operator!=(A a, B b) {
    return Lazy::Cmp<A, B, Lazy::Vec::NEQ>(a, b);
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::NEQ> operator!=(A a, float b) {
    return a != Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::NEQ> operator!=(float a, B b) {
    return Lazy::Const(a) != b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::NEQ> operator!=(A a, double b) {
    return a != Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::NEQ> operator!=(double a, B b) {
    return Lazy::Const(a) != b;
}

template<typename A>
Lazy::Cmp<typename A::Lazy, Lazy::Const, Lazy::Vec::NEQ> operator!=(A a, int b) {
    return a != Lazy::Const(b);
}

template<typename B>
Lazy::Cmp<Lazy::Const, typename B::Lazy, Lazy::Vec::NEQ> operator!=(int a, B b) {
    return Lazy::Const(a) != b;
}
#include "footer.h"

#endif
