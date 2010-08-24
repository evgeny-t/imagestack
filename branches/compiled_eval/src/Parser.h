#ifndef IMAGESTACK_PARSER_H
#define IMAGESTACK_PARSER_H

#include "Image.h"
#include "Statistics.h"
#include "header.h"

#ifndef roundf
#define roundf(x) floorf(x+0.5);
#endif

class Expression {
    // the AST nodes are below here
    /* Grammar:

    IfThenElse -> Condition | Condition ? Condition : Condition
    Condition  -> Sum > Sum | Sum < Sum | 
    Sum >= Sum | Sum <= Sum | 
    Sum == Sum | Sum != Sum | Sum
    Sum     -> Sum + Product | Sum - Product | Product
    Product -> Product * Factor | Product / Factor | 
    Product % Factor | Factor
    Factor  -> Term ^ Term | Term
    Term    -> Funct0 ( ) | Funct1 ( IfThenElse ) | Funct2 ( IfThenElse , IfThenElse ) | - Term | Var | ( IfThenElse ) | Float | Sample | Const | Uniform
    Funct0  -> mean | sum | max | min | stddev | var | skew | kurtosis
    Funct1  -> sin | cos | tan | log | abs | mean | sum | max | min | stddev | variance | skew | kurtosis
    Funct2  -> covariance
    Var     -> x | y | t | c | X | Y | T | val
    Uniform -> width | height | frames | channels
    Const   -> e | pi
    Sample  -> [IfThenElse, IfThenElse, IfThenElse] | [IfThenElse, IfThenElse] | [IfThenElse]

    */    

    // Forward declare all the node types
public:
    class Var_x;    
    class Var_y;    
    class Var_t;    
    class Var_c;    
    class Var_val;    
    class Uniform_width;
    class Uniform_height;
    class Uniform_frames;
    class Uniform_channels;
    class Float;    
    class Negation;      
    class Funct_mean0;
    class Funct_sum0;
    class Funct_max0;
    class Funct_min0;
    class Funct_variance0;
    class Funct_stddev0;
    class Funct_skew0;
    class Funct_kurtosis0;
    class Funct_sin;
    class Funct_cos;
    class Funct_tan;
    class Funct_atan;
    class Funct_asin;
    class Funct_acos;
    class Funct_abs;
    class Funct_floor;
    class Funct_ceil;
    class Funct_round;
    class Funct_log;
    class Funct_exp;
    class Funct_mean1;
    class Funct_sum1;
    class Funct_max1;
    class Funct_min1;
    class Funct_variance1;
    class Funct_stddev1;
    class Funct_skew1;
    class Funct_kurtosis1;
    class SampleHere;
    class LTE;
    class GTE;
    class LT;
    class GT;
    class EQ;
    class NEQ;
    class Plus;
    class Minus;
    class Mod;
    class Times;
    class Divide;
    class Power;
    class Funct_atan2;
    class Funct_covariance;
    class Sample2D;    
    class IfThenElse;    
    class Sample3D;
    
    // Define the interface for a visitor that traverses an AST
    class Visitor {
    public:
        virtual void visit(Var_x *node) = 0;    
        virtual void visit(Var_y *node) = 0;    
        virtual void visit(Var_t *node) = 0;    
        virtual void visit(Var_c *node) = 0;    
        virtual void visit(Var_val *node) = 0;    
        virtual void visit(Uniform_width *node) = 0;
        virtual void visit(Uniform_height *node) = 0;
        virtual void visit(Uniform_frames *node) = 0;
        virtual void visit(Uniform_channels *node) = 0;
        virtual void visit(Float *node) = 0;    
        virtual void visit(Negation *node) = 0;      
        virtual void visit(Funct_mean0 *node) = 0;
        virtual void visit(Funct_sum0 *node) = 0;
        virtual void visit(Funct_max0 *node) = 0;
        virtual void visit(Funct_min0 *node) = 0;
        virtual void visit(Funct_variance0 *node) = 0;
        virtual void visit(Funct_stddev0 *node) = 0;
        virtual void visit(Funct_skew0 *node) = 0;
        virtual void visit(Funct_kurtosis0 *node) = 0;
        virtual void visit(Funct_sin *node) = 0;
        virtual void visit(Funct_cos *node) = 0;
        virtual void visit(Funct_tan *node) = 0;
        virtual void visit(Funct_atan *node) = 0;
        virtual void visit(Funct_asin *node) = 0;
        virtual void visit(Funct_acos *node) = 0;
        virtual void visit(Funct_abs *node) = 0;
        virtual void visit(Funct_floor *node) = 0;
        virtual void visit(Funct_ceil *node) = 0;
        virtual void visit(Funct_round *node) = 0;
        virtual void visit(Funct_log *node) = 0;
        virtual void visit(Funct_exp *node) = 0;
        virtual void visit(Funct_mean1 *node) = 0;
        virtual void visit(Funct_sum1 *node) = 0;
        virtual void visit(Funct_max1 *node) = 0;
        virtual void visit(Funct_min1 *node) = 0;
        virtual void visit(Funct_variance1 *node) = 0;
        virtual void visit(Funct_stddev1 *node) = 0;
        virtual void visit(Funct_skew1 *node) = 0;
        virtual void visit(Funct_kurtosis1 *node) = 0;
        virtual void visit(SampleHere *node) = 0;
        virtual void visit(LTE *node) = 0;
        virtual void visit(GTE *node) = 0;
        virtual void visit(LT *node) = 0;
        virtual void visit(GT *node) = 0;
        virtual void visit(EQ *node) = 0;
        virtual void visit(NEQ *node) = 0;
        virtual void visit(Plus *node) = 0;
        virtual void visit(Minus *node) = 0;
        virtual void visit(Mod *node) = 0;
        virtual void visit(Times *node) = 0;
        virtual void visit(Divide *node) = 0;
        virtual void visit(Power *node) = 0;
        virtual void visit(Funct_atan2 *node) = 0;
        virtual void visit(Funct_covariance *node) = 0;
        virtual void visit(Sample2D *node) = 0;    
        virtual void visit(IfThenElse *node) = 0;    
        virtual void visit(Sample3D *node) = 0;
    };

    // Actually define all the node types

    struct Node {
        Node() {};
        virtual ~Node() {};
        virtual void accept(Visitor *v) = 0;
        void *data; // visitors can tag nodes with arbitrary data if they like
    };

#define nullary(name)                               \
    class name : public Node {                      \
    public:                                         \
        name() {}                                   \
        void accept(Visitor *v) {v->visit(this);}   \
    }

#define unary(name)                                 \
    class name : public Node {                      \
    public:                                         \
        name(Node *a) : arg1(a) {}                  \
        void accept(Visitor *v) {v->visit(this);}   \
        Node *arg1;                                 \
    }
    
#define binary(name)                                    \
    class name : public Node {                          \
    public:                                             \
        name(Node *a, Node *b) : arg1(a), arg2(b) {}    \
        void accept(Visitor *v) {v->visit(this);}       \
        Node *arg1, *arg2;                              \
    }
    
#define ternary(name)                                   \
    class name : public Node {                          \
    public:                                             \
        name(Node *a, Node *b, Node *c) :               \
            arg1(a), arg2(b), arg3(c) {}                \
        void accept(Visitor *v) {v->visit(this);}       \
        Node *arg1, *arg2, *arg3;                       \
    }

    
    unary(Negation);
    ternary(IfThenElse);
    binary(LTE);
    binary(GTE);
    binary(LT);
    binary(GT);
    binary(EQ);
    binary(NEQ);
    binary(Plus);
    binary(Minus);
    binary(Mod);
    binary(Times);
    binary(Divide);
    binary(Power);
    unary(Funct_sin);
    unary(Funct_cos);
    unary(Funct_tan);
    unary(Funct_asin);
    unary(Funct_acos);
    unary(Funct_atan);
    binary(Funct_atan2);
    unary(Funct_abs);
    unary(Funct_floor);
    unary(Funct_ceil);
    unary(Funct_round);
    unary(Funct_log);
    unary(Funct_exp);
    nullary(Funct_mean0);
    unary(Funct_mean1);
    nullary(Funct_sum0);
    unary(Funct_sum1);
    nullary(Funct_max0);
    unary(Funct_max1);
    nullary(Funct_min0);
    unary(Funct_min1);
    nullary(Funct_variance0);
    unary(Funct_variance1);
    binary(Funct_covariance);
    nullary(Funct_stddev0);
    unary(Funct_stddev1);    
    nullary(Funct_skew0);
    unary(Funct_skew1);    
    nullary(Funct_kurtosis0);
    unary(Funct_kurtosis1);    
    nullary(Var_x);
    nullary(Var_y);
    nullary(Var_t);
    nullary(Var_c);
    nullary(Var_val);
    nullary(Uniform_width);
    nullary(Uniform_height);
    nullary(Uniform_frames);
    nullary(Uniform_channels);
    unary(SampleHere);
    binary(Sample2D);
    ternary(Sample3D);

#undef nullary
#undef unary
#undef binary
#undef ternary

    class Float : public Node {
    public:
        Float(float value_) : value(value_) {}
        void accept(Visitor *v) {v->visit(this);}
        float value;
    };

    // all the parsing stuff is below here
  private:
    void skipWhitespace();
    bool match(string prefix);
    bool consume(string prefix);

    // IfThenElse -> Condition (? Condition : Condition)?
    Node *parseIfThenElse();

    // Condition -> Sum ((>|<|>=|<=|==|!=) Sum)?
    Node *parseCondition();

    // Sum     -> Product ((+|-) Product)*
    Node *parseSum();

    // Product -> Factor ((*|/|%) Factor)*
    Node *parseProduct();

    // Factor  -> Term ^ Term | Term  
    Node *parseFactor();

    // Term    -> Funct ( ) | Funct ( IfThenElse , IfThenElse ) | Funct ( IfThenElse ) | - Term | Var | ( IfThenElse ) | Float | Sample
    Node *parseTerm();

    string source;
    size_t sourceIndex;
    bool varyingAllowed;

  public:
    Expression(string source_, bool varyingAllowed = true);

    ~Expression();

    Node *root;

    static void help();

};

class Interpreter : public Expression::Visitor {
public:
    int t, x, y, c;
    float *val;        
    Window im;
    Stats stats;
    
    Interpreter(Window w) : t(0), x(0), y(0), c(0), val(im(0, 0)), im(w), stats(w) {
    }

    float interpret(const Expression &e) {
        sample = new float[im.channels];
        e.root->accept(this);
        delete[] sample;
        return result;
    }
   
    int iResult() {
        return roundf(result);
    }
 
    void visit(Expression::Var_x *node) {result = x;}
    void visit(Expression::Var_y *node) {result = y;}
    void visit(Expression::Var_t *node) {result = t;}
    void visit(Expression::Var_c *node) {result = c;}
    void visit(Expression::Var_val *node) {result = val[c];}
    void visit(Expression::Uniform_width *node) {result = im.width;}
    void visit(Expression::Uniform_height *node) {result = im.height;}
    void visit(Expression::Uniform_frames *node) {result = im.frames;}
    void visit(Expression::Uniform_channels *node) {result = im.channels;}
    void visit(Expression::Float *node) {result = node->value;}
    void visit(Expression::Negation *node) {node->arg1->accept(this); result = -result;}
    void visit(Expression::Funct_sin *node) {node->arg1->accept(this); result = sinf(result);}
    void visit(Expression::Funct_cos *node) {node->arg1->accept(this); result = cosf(result);}
    void visit(Expression::Funct_tan *node) {node->arg1->accept(this); result = tanf(result);}
    void visit(Expression::Funct_atan *node) {node->arg1->accept(this); result = atanf(result);}
    void visit(Expression::Funct_asin *node) {node->arg1->accept(this); result = asinf(result);}
    void visit(Expression::Funct_acos *node) {node->arg1->accept(this); result = acosf(result);}
    void visit(Expression::Funct_abs *node) {node->arg1->accept(this); result = fabs(result);}
    void visit(Expression::Funct_floor *node) {node->arg1->accept(this); result = floorf(result);}
    void visit(Expression::Funct_ceil *node) {node->arg1->accept(this); result = ceilf(result);}
    void visit(Expression::Funct_round *node) {node->arg1->accept(this); result = roundf(result);}
    void visit(Expression::Funct_log *node) {node->arg1->accept(this); result = logf(result);}
    void visit(Expression::Funct_exp *node) {node->arg1->accept(this); result = expf(result);}
    void visit(Expression::Funct_mean0 *node) {result = stats.mean();}
    void visit(Expression::Funct_sum0 *node) {result = stats.sum();}
    void visit(Expression::Funct_max0 *node) {result = stats.maximum();}
    void visit(Expression::Funct_min0 *node) {result = stats.minimum();}
    void visit(Expression::Funct_variance0 *node) {result = stats.variance();}
    void visit(Expression::Funct_stddev0 *node) {result = sqrtf(stats.variance());}
    void visit(Expression::Funct_skew0 *node) {result = stats.skew();}
    void visit(Expression::Funct_kurtosis0 *node) {result = stats.kurtosis();}
    
    void visit(Expression::Funct_mean1 *node) {node->arg1->accept(this); result = stats.mean(iResult());}
    void visit(Expression::Funct_sum1 *node) {node->arg1->accept(this); result = stats.sum(iResult());}
    void visit(Expression::Funct_max1 *node) {node->arg1->accept(this); result = stats.maximum(iResult());}
    void visit(Expression::Funct_min1 *node) {node->arg1->accept(this); result = stats.minimum(iResult());}
    void visit(Expression::Funct_variance1 *node) {node->arg1->accept(this); result = stats.variance(iResult());}
    void visit(Expression::Funct_stddev1 *node) {node->arg1->accept(this); result = sqrtf(stats.variance(iResult()));}
    void visit(Expression::Funct_skew1 *node) {node->arg1->accept(this); result = stats.skew(iResult());}
    void visit(Expression::Funct_kurtosis1 *node) {node->arg1->accept(this); result = stats.kurtosis(iResult());}
    
    void visit(Expression::LTE *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp <= result ? 1 : 0);
    }
    
    void visit(Expression::GTE *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp >= result ? 1 : 0);
    }
    
    void visit(Expression::LT *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp < result ? 1 : 0);
    }
    
    void visit(Expression::GT *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp > result ? 1 : 0);
    }
    
    void visit(Expression::EQ *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp == result ? 1 : 0);
    }
    
    void visit(Expression::NEQ *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result = (tmp != result ? 1 : 0);
    }
    
    void visit(Expression::Plus *node) {
        node->arg1->accept(this); 
        float tmp=result;
        node->arg2->accept(this);
        result += tmp;
    }
    
    void visit(Expression::Minus *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result -= tmp;
    }
    
    void visit(Expression::Times *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result *= tmp;
    }
    
    void visit(Expression::Mod *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result = fmod(result, tmp);
    }
    
    void visit(Expression::Divide *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result /= tmp;
    }
    
    void visit(Expression::Power *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result = powf(result, tmp);
    }
    
    void visit(Expression::Funct_atan2 *node) {
        node->arg2->accept(this); 
        float tmp=result;
        node->arg1->accept(this);
        result = atan2(result, tmp);
    }
    
    void visit(Expression::Funct_covariance *node) {
        node->arg2->accept(this); 
        int tmp = iResult();
        node->arg1->accept(this);
        result = stats.covariance(iResult(), tmp);
    }
    
    void visit(Expression::IfThenElse *node) {
        node->arg1->accept(this);
        if (iResult()) node->arg2->accept(this);
        else node->arg3->accept(this);
    }
    
    void visit(Expression::SampleHere *node) {
        node->arg1->accept(this);
        result = val[iResult()];
    }
    
    void visit(Expression::Sample2D *node) {
        node->arg1->accept(this);
        float fx = result;
        node->arg2->accept(this);
        float fy = result;
        im.sample2D(fx, fy, sample);
        result = sample[c];
    }
    
    void visit(Expression::Sample3D *node) {
        node->arg1->accept(this);
        float fx = result;
        node->arg2->accept(this);
        float fy = result;
        node->arg3->accept(this);
        float ft = result;
        im.sample3D(fx, fy, ft, sample);
        result = sample[c];        
    }

private:
    float result;
    float *sample;
};

#include "footer.h"
#endif
