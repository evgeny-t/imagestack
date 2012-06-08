#ifndef IMAGESTACK_FUNC_H
#define IMAGESTACK_FUNC_H

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
};

struct X : public Unbounded {
    typedef X Func;
    float operator()(int x, int, int, int) const {return x;}
};

struct Y : public Unbounded {
    typedef Y Func;
    float operator()(int, int y, int, int) const {return y;}
};

struct T : public Unbounded {
    typedef T Func;
    float operator()(int, int, int t, int) const {return t;}
};

struct C : public Unbounded {
    typedef C Func;
    float operator()(int, int, int, int c) const {return c;}
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
};

template<typename A, typename B>
struct Sub : public BinaryOp<A, B> {
    typedef Sub<typename A::Func, typename B::Func> Func;
    Sub(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) - BinaryOp<A, B>::b(x, y, t, c);
    }
};

template<typename A, typename B>
struct Mul : public BinaryOp<A, B> {
    typedef Mul<typename A::Func, typename B::Func> Func;
    Mul(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) * BinaryOp<A, B>::b(x, y, t, c);
    }
};

template<typename A, typename B>
struct Div : public BinaryOp<A, B> {
    typedef Div<typename A::Func, typename B::Func> Func;
    Div(const A a_, const B b_) : BinaryOp<A, B>(a_, b_) {}
    float operator()(int x, int y, int t, int c) const {
	return BinaryOp<A, B>::a(x, y, t, c) / BinaryOp<A, B>::b(x, y, t, c);
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




#include "footer.h"

#endif
