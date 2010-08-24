#ifndef IMAGESTACK_COMPILER_H
#define IMAGESTACK_COMPILER_H

#include <vector>
#include "Parser.h"
#include "X64.h"
#include "header.h"

const char *opname[] = {"Const", "NoOp", "VarX", "VarY", "VarT", "VarC", "VarVal", "Negate", "Plus", "Minus", 
                        "Times", "Divide", "LT", "GT", "LTE", "GTE", "EQ", "NEQ", "Sin", "Cos", "Tan", "Pow",
                        "ASin", "ACos", "ATan", "ATan2", "Abs", "Floor", "Ceil", "Round",
                        "Exp", "Log", "Mod", "SampleHere", "Sample2D", "Sample3D",
                        "IfLT", "IfGT", "IfLTE", "IfGTE", "IfEQ", "IfNEQ", "IfNZ"};

// As part of compilation, we tag each AST node with some useful stuff:
struct Tag {
    uint32_t deps;
    float value;
};

// Set a tag
static void tag(Expression::Node *n, uint32_t deps, float value = 0) {
    Tag *t = new Tag;
    t->deps = deps;
    t->value = value;
    n->data = (void *)t;            
}

// Retrieve a tag
static const Tag &tag(Expression::Node *n) {
    Tag *t = (Tag *)n->data;
    return *t;
}

// A compiler for parsed expressions
class Program : public Expression::Visitor {
private:
    // bytecode instructions
    enum {DEP_T = 1, DEP_Y = 2, DEP_X = 4, DEP_C = 8, DEP_VAL = 16, DEP_SAMPLE = 32};
    struct ByteCode {
        enum Op {Const = 0, NoOp, VarX, VarY, VarT, VarC, VarVal, Negate, Plus, Minus, 
                 Times, Divide, LT, GT, LTE, GTE, EQ, NEQ, Sin, Cos, Tan, Pow,
                 ASin, ACos, ATan, ATan2, Abs, Floor, Ceil, Round,
                 Exp, Log, Mod, SampleHere, Sample2D, Sample3D,
                 IfLT, IfGT, IfLTE, IfGTE, IfEQ, IfNEQ, IfNZ};
        Op op;
        float val; // for const only        
        uint32_t deps; // what variables does this depend on?
    };

    std::vector<ByteCode> instructions;
    Window im;
    Stats stats;

    // some useful constants for runtime
    float const_one, const_pi, const_e;

public:

    class Analyzer : public Expression::Visitor {
    private:
        Window im;
        Stats stats;
    public:

        Analyzer(Window w, Stats s) : im(w), stats(s) {}

        void visit(Expression::Var_x *node) {tag(node, DEP_X);}
        void visit(Expression::Var_y *node) {tag(node, DEP_Y);}
        void visit(Expression::Var_t *node) {tag(node, DEP_T);}
        void visit(Expression::Var_c *node) {tag(node, DEP_C);}
        void visit(Expression::Var_val *node) {tag(node, DEP_VAL);}
        void visit(Expression::Uniform_width *node) {tag(node, 0, im.width);}
        void visit(Expression::Uniform_height *node) {tag(node, 0, im.height);}
        void visit(Expression::Uniform_frames *node) {tag(node, 0, im.frames);}
        void visit(Expression::Uniform_channels *node) {tag(node, 0, im.channels);}
        void visit(Expression::Float *node) {tag(node, 0, node->value);}
        void visit(Expression::Funct_mean0 *node) {tag(node, 0, stats.mean());}
        void visit(Expression::Funct_sum0 *node) {tag(node, 0, stats.sum());}
        void visit(Expression::Funct_max0 *node) {tag(node, 0, stats.maximum());}
        void visit(Expression::Funct_min0 *node) {tag(node, 0, stats.minimum());}
        void visit(Expression::Funct_variance0 *node) {tag(node, 0, stats.variance());}
        void visit(Expression::Funct_stddev0 *node) {tag(node, 0, sqrtf(stats.variance()));}
        void visit(Expression::Funct_skew0 *node) {tag(node, 0, stats.skew());}
        void visit(Expression::Funct_kurtosis0 *node) {tag(node, 0, stats.kurtosis());}           
        
        void visit(Expression::Negation *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : -t1.value);
        }

        void visit(Expression::Funct_sin *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : sinf(t1.value));
        }        

        void visit(Expression::Funct_cos *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : cosf(t1.value));            
        }

        void visit(Expression::Funct_tan *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : tanf(t1.value));            
        }

        void visit(Expression::Funct_atan *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : atanf(t1.value));            
        }

        void visit(Expression::Funct_asin *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : asinf(t1.value));            
        }

        void visit(Expression::Funct_acos *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : acosf(t1.value));            
        }

        void visit(Expression::Funct_abs *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : fabs(t1.value));            
        }

        void visit(Expression::Funct_floor *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : floorf(t1.value));            
        }

        void visit(Expression::Funct_ceil *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : ceilf(t1.value));            
        }

        void visit(Expression::Funct_round *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            if (t1.deps) {
                tag(node, t1.deps, 0);
            } else {
                float x = roundf(t1.value);
                tag(node, 0, x);
            }
        }

        void visit(Expression::Funct_log *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : logf(t1.value));            
        }

        void visit(Expression::Funct_exp *node) {
            node->arg1->accept(this); 
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps, t1.deps ? 0 : expf(t1.value));            
        }

        // TODO
        void visit(Expression::Funct_mean1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_sum1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_max1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_min1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_variance1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_stddev1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_skew1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_kurtosis1 *node) {tag(node, 0, 0);}
        void visit(Expression::Funct_covariance *node) {tag(node, 0, 0);}
        
        void visit(Expression::LTE *node) {            
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value <= t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::GTE *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value >= t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::LT *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value < t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::GT *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value > t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::EQ *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value == t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::NEQ *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : (t1.value != t2.value ? 1.0f : 0.0f));
        }
        
        void visit(Expression::Plus *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : t1.value + t2.value);
        }
        
        void visit(Expression::Minus *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : t1.value - t2.value);
        }
        
        void visit(Expression::Times *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : t1.value * t2.value);
        }
        
        void visit(Expression::Mod *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : fmod(t1.value, t2.value));
        }
        
        void visit(Expression::Divide *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : t1.value / t2.value);
        }
        
        void visit(Expression::Power *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : powf(t1.value, t2.value));
        }
        
        void visit(Expression::Funct_atan2 *node) {
            node->arg1->accept(this); 
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            uint32_t deps = t1.deps | t2.deps;
            tag(node, deps, deps ? 0 : atan2(t1.value, t2.value));
        }
    
        void visit(Expression::IfThenElse *node) {
            node->arg1->accept(this);        
            node->arg2->accept(this);
            node->arg3->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            const Tag &t3 = tag(node->arg3);
            if (!t1.deps) {
                if (t1.value) {
                    tag(node, t2.deps, t2.deps ? 0 : t2.value);
                } else {
                    tag(node, t3.deps, t3.deps ? 0 : t3.value);                    
                }
            } else {
                uint32_t deps = t1.deps | t2.deps | t3.deps;
                tag(node, deps, 0);
            }
        }
    
        void visit(Expression::SampleHere *node) {
            node->arg1->accept(this);
            const Tag &t1 = tag(node->arg1);
            tag(node, t1.deps | DEP_SAMPLE, 0);
        }
    
        void visit(Expression::Sample2D *node) {
            node->arg1->accept(this);
            node->arg2->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            tag(node, t1.deps | t2.deps | DEP_SAMPLE, 0);
        }
    
        void visit(Expression::Sample3D *node) {
            node->arg1->accept(this);
            node->arg2->accept(this);
            node->arg3->accept(this);
            const Tag &t1 = tag(node->arg1);
            const Tag &t2 = tag(node->arg2);
            const Tag &t3 = tag(node->arg3);
            tag(node, t1.deps | t2.deps | t3.deps | DEP_SAMPLE, 0);
        }        
    };

    Program(const Expression &e, Window w) : im(w), stats(w) {
        // traverse the tree and tag each node
        Analyzer analyzer(im, stats);
        e.root->accept(&analyzer);
        
        // traverse the tree and generate bytecode        
        e.root->accept(this);

        // list the byte code
        for (size_t i = 0; i < instructions.size(); i++) {
            for (int j = 1; j < DEP_SAMPLE*2; j*=2) {
                printf((instructions[i].deps & j) ? "#" : "-");
            }
            printf(" ");
            if (instructions[i].op) {
                printf("%s\n", opname[instructions[i].op]);
            } else {
                printf("%f\n", instructions[i].val);
            }
        }

        // prepare some fixed constants
        const_one = 1.0f;
        const_pi = M_PI;
        const_e = expf(1);
        
    } 

    void gen(ByteCode::Op op, uint32_t deps) {
        ByteCode inst;
        inst.op = op;
        inst.deps = deps;
        instructions.push_back(inst);
    }

    ByteCode &last(int n=1) {
        return instructions[instructions.size()-n];
    }

    void pop() {
        instructions.pop_back();
    }

    void gen(float f) {
        ByteCode inst;
        inst.op = ByteCode::Const;
        inst.val = f;
        inst.deps = 0;
        instructions.push_back(inst);
    }

    struct State {
        int x, y, t, c;
        float *val;
    };

    // interpret the generated bytecode
    float interpret(const State &s) {
        // these are used at runtime
        vector<float> stack;
        float *ptr;
        vector<float> sample;

        // compute the max stack depth needed
        int stackSize = 0;
        int maxStackSize = 0;
        for (size_t i = 0; i < instructions.size(); i++) {
            switch(instructions[i].op) {
            case ByteCode::Const: case ByteCode::VarX: case ByteCode::VarVal:
            case ByteCode::VarY: case ByteCode::VarT: case ByteCode::VarC:
                stackSize++;
                break;
            case ByteCode::Plus: case ByteCode::Minus: case ByteCode::Times: 
            case ByteCode::Divide: case ByteCode::LT: case ByteCode::GT:
            case ByteCode::LTE: case ByteCode::GTE: case ByteCode::ATan2:
            case ByteCode::Sample2D: case ByteCode::Mod: case ByteCode::Pow:
            case ByteCode::EQ: case ByteCode::NEQ:
                stackSize--;
                break;
            case ByteCode::IfNZ: case ByteCode::Sample3D:
                stackSize-=2;
                break;
            case ByteCode::IfNEQ: case ByteCode::IfEQ:
            case ByteCode::IfLT: case ByteCode::IfGT:
            case ByteCode::IfLTE: case ByteCode::IfGTE:
                stackSize-=3;
                break;
            default:                
                break;
            }
            if (stackSize > maxStackSize) maxStackSize = stackSize;
        }

        stack.resize(maxStackSize);
        sample.resize(im.channels);

        ptr = &(stack[0]);
        float tmp;
        for (size_t i = 0; i < instructions.size(); i++) {
            switch(instructions[i].op) {
            case ByteCode::Const: 
                *ptr++ = instructions[i].val;
                break;
            case ByteCode::VarX:
                *ptr++ = s.x;
                break;
            case ByteCode::VarY:
                *ptr++ = s.y;
                break;
            case ByteCode::VarT:
                *ptr++ = s.t;
                break;
            case ByteCode::VarC:
                *ptr++ = s.c;
                break;
            case ByteCode::VarVal:
                *ptr++ = s.val[s.c];
                break;
            case ByteCode::Plus:
                ptr[-2] += ptr[-1];
                ptr--;
                break;
            case ByteCode::Minus:
                ptr[-2] -= ptr[-1];
                ptr--;
                break;
            case ByteCode::Times: 
                ptr[-2] *= ptr[-1];
                ptr--;
                break;
            case ByteCode::Divide: 
                ptr[-2] /= ptr[-1];
                ptr--;
                break;
            case ByteCode::LT: 
                tmp = (ptr[-2] < ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::GT:
                tmp = (ptr[-2] > ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::LTE:
                tmp = (ptr[-2] <= ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::GTE:
                tmp = (ptr[-2] >= ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::EQ:
                tmp = (ptr[-2] == ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::NEQ:
                tmp = (ptr[-2] != ptr[-1]) ? 1 : 0;
                ptr[-2] = tmp;
                ptr--;
                break;
            case ByteCode::ATan2:
                ptr[-2] = atan2(ptr[-2], ptr[-1]);
                ptr--;
                break;
            case ByteCode::Sample2D: 
                im.sample2D(ptr[-2], ptr[-1], &(sample[0]));
                ptr[-2] = sample[s.c];
                ptr--;
                break;
            case ByteCode::Sample3D:
                im.sample2D(ptr[-3], ptr[-2], ptr[-1], &(sample[0]));
                ptr[-3] = sample[s.c];
                ptr -= 2;
                break;
            case ByteCode::IfNZ:
                if (ptr[-3] != 0) ptr[-3] = ptr[-2];
                else ptr[-3] = ptr[-1];
                ptr -= 2;
                break;
            case ByteCode::IfNEQ:
                if (ptr[-4] != ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::IfEQ:
                if (ptr[-4] == ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::IfLT:
                if (ptr[-4] < ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::IfGT:
                if (ptr[-4] > ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::IfLTE:
                if (ptr[-4] <= ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::IfGTE:
                if (ptr[-4] >= ptr[-3]) ptr[-4] = ptr[-2];
                else ptr[-4] = ptr[-1];
                ptr -= 3;
                break;
            case ByteCode::SampleHere:
                tmp = roundf(ptr[-1]);
                ptr[-1] = s.val[(int)tmp];
                break;
            case ByteCode::Mod:
                ptr[-2] = fmod(ptr[-2], ptr[-1]);
                ptr--;
                break;
            case ByteCode::Pow:
                ptr[-2] = powf(ptr[-2], ptr[-1]);
                ptr--;
                break;
            case ByteCode::Sin:
                ptr[-1] = sinf(ptr[-1]);
                break;
            case ByteCode::Cos:
                ptr[-1] = cosf(ptr[-1]);
                break;
            case ByteCode::Tan:
                ptr[-1] = tanf(ptr[-1]);
                break;   
            case ByteCode::ASin:
                ptr[-1] = asinf(ptr[-1]);
                break;
            case ByteCode::ACos:
                ptr[-1] = acosf(ptr[-1]);
                break;
            case ByteCode::ATan:
                ptr[-1] = atanf(ptr[-1]);
                break;   
            case ByteCode::Exp:
                ptr[-1] = expf(ptr[-1]);
                break;
            case ByteCode::Log:
                ptr[-1] = logf(ptr[-1]);
                break;
            case ByteCode::Negate:
                ptr[-1] = -ptr[-1];
                break;
            case ByteCode::Floor:
                ptr[-1] = floorf(ptr[-1]);
                break;
            case ByteCode::Ceil:
                ptr[-1] = ceilf(ptr[-1]);
                break;
            case ByteCode::Round:
                ptr[-1] = roundf(ptr[-1]);
                break;
            case ByteCode::Abs:
                ptr[-1] = fabs(ptr[-1]);
                break;
            }
            /*
            printf("Stack %d: ", ptr-&(stack[0]));
            for (size_t j = 0; j < stack.size(); j++) {
                if (&(stack[j]) == ptr) break;
                printf("%f ", stack[j]);
            }
            printf("\n");
            */
        }        
        return stack[0];
    }
 
    void compileEval(Window out, AsmX64 *a) {

        AsmX64::Reg x = a->rax, y = a->rcx, 
            t = a->r8, c = a->rsi, 
            imPtr = a->rdx, tmp = a->r15,
            outPtr = a->rdi;

        // the index of the first free sse register
        int r = 0;
        
        printf("Generating machine code...\n");
        a->pushNonVolatiles();

        // generate the constants

        a->mov(t, 0);
        a->label("tloop"); {
            // generate the values that don't depend on Y, X, C, or val

            a->mov(y, 0);
            a->label("yloop"); {
                // compute the address of the start of this scanline
                a->mov(imPtr, im(0, 0));           
                a->mov(tmp, t); 
                a->imul(tmp, im.tstride*sizeof(float));
                a->add(imPtr, tmp);
                a->mov(tmp, y);
                a->imul(tmp, im.ystride*sizeof(float));
                a->add(imPtr, tmp);
                a->mov(outPtr, (int64_t)(sizeof(float)*(out(0, 0) - im(0, 0))));
                a->add(outPtr, imPtr);
                
                // generate the values that don't depend on X, C, or val

                a->mov(x, 0);               
                a->label("xloop"); {
                    
                    // generate the values that don't depend on C or val

                    a->mov(c, 0);
                    a->label("cloop"); {
                        
                        // insert code for the expression body
                        compileBody(a, x, y, t, c, imPtr, r);
                        a->movss(outPtr, a->xmm0);
                        
                        a->add(c, 1);
                        a->add(imPtr, 4);
                        a->add(outPtr, 4);
                        a->cmp(c, im.channels);
                        a->jl("cloop");
                    }
                    a->add(x, 1);
                    a->cmp(x, im.width);
                    a->jl("xloop");
                }
                a->add(y, 1);
                a->cmp(y, im.height);
                a->jl("yloop");            
            }
            a->add(t, 1);
            a->cmp(t, im.frames);
            a->jl("tloop");            
        }
        a->popNonVolatiles();
        a->ret();
        
        // save the obj for debugging
        a->saveCOFF("generated.obj");
    }

    void compileBody(AsmX64 *a, 
                     AsmX64::Reg x, AsmX64::Reg y,
                     AsmX64::Reg t, AsmX64::Reg c, 
                     AsmX64::Reg ptr, int r) {

        for (size_t i = 0; i < instructions.size(); i++) {
            switch(instructions[i].op) {
            case ByteCode::Const: 
                a->mov(a->r15, &instructions[i].val);
                a->movss(AsmX64::SSEReg(r++), AsmX64::Mem(a->r15));
                break;
            case ByteCode::VarX:
                a->cvtsi2ss(AsmX64::SSEReg(r++), x);
                break;
            case ByteCode::VarY:
                a->cvtsi2ss(AsmX64::SSEReg(r++), y);
                break;
            case ByteCode::VarT:
                a->cvtsi2ss(AsmX64::SSEReg(r++), t);
                break;
            case ByteCode::VarC:
                a->cvtsi2ss(AsmX64::SSEReg(r++), c);
                break;
            case ByteCode::VarVal:
                a->movss(AsmX64::SSEReg(r++), AsmX64::Mem(ptr));
                break;
            case ByteCode::Plus:
                a->addss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::Minus:
                a->subss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::Times: 
                a->mulss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::Divide: 
                a->divss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::LT:
                a->cmpltss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::GT:
                a->cmpnless(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::LTE:
                a->cmpless(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::GTE:
                a->cmpnltss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::EQ:
                a->cmpeqss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::NEQ:
                a->cmpneqss(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                a->mov(a->r15, &const_one);
                a->movss(AsmX64::SSEReg(r-1), AsmX64::Mem(a->r15));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-1));
                r--;
                break;
            case ByteCode::IfNZ:                
                a->bxorps(AsmX64::SSEReg(r), AsmX64::SSEReg(r));
                a->cmpneqss(AsmX64::SSEReg(r-3), AsmX64::SSEReg(r));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-3));
                a->bandnps(AsmX64::SSEReg(r-3), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-3), AsmX64::SSEReg(r-2));
                r -= 2;
                break;
            case ByteCode::IfNEQ:
                a->cmpneqss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::IfEQ:
                a->cmpeqss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::IfLT:
                a->cmpltss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::IfGT:
                a->cmpnless(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::IfLTE:
                a->cmpless(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::IfGTE:
                a->cmpnltss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ByteCode::ATan2:
            case ByteCode::Sample2D: 
            case ByteCode::Sample3D:
            case ByteCode::SampleHere:
            case ByteCode::Mod:
            case ByteCode::Pow:
            case ByteCode::Sin:
            case ByteCode::Cos:
            case ByteCode::Tan:
            case ByteCode::ASin:
            case ByteCode::ACos:
            case ByteCode::ATan:
            case ByteCode::Exp:
            case ByteCode::Log:
            case ByteCode::Negate:
            case ByteCode::Floor:
            case ByteCode::Ceil:
            case ByteCode::Round:
            case ByteCode::Abs:
                printf("Not implemented\n");                
                break;
            }
        }
    }

    void visit(Expression::Var_x *node) {gen(ByteCode::VarX, DEP_X);}
    void visit(Expression::Var_y *node) {gen(ByteCode::VarY, DEP_Y);}
    void visit(Expression::Var_t *node) {gen(ByteCode::VarT, DEP_T);}
    void visit(Expression::Var_c *node) {gen(ByteCode::VarC, DEP_C);}
    void visit(Expression::Var_val *node) {gen(ByteCode::VarVal, DEP_VAL);}
    void visit(Expression::Uniform_width *node) {gen(im.width);}
    void visit(Expression::Uniform_height *node) {gen(im.height);}
    void visit(Expression::Uniform_frames *node) {gen(im.frames);}
    void visit(Expression::Uniform_channels *node) {gen(im.channels);}
    void visit(Expression::Float *node) {gen(node->value);}
    void visit(Expression::Funct_mean0 *node) {gen(stats.mean());}
    void visit(Expression::Funct_sum0 *node) {gen(stats.sum());}
    void visit(Expression::Funct_max0 *node) {gen(stats.maximum());}
    void visit(Expression::Funct_min0 *node) {gen(stats.minimum());}
    void visit(Expression::Funct_variance0 *node) {gen(stats.variance());}
    void visit(Expression::Funct_stddev0 *node) {gen(sqrtf(stats.variance()));}
    void visit(Expression::Funct_skew0 *node) {gen(stats.skew());}
    void visit(Expression::Funct_kurtosis0 *node) {gen(stats.kurtosis());}    
    
#define checkConst            \
    if (!tag(node).deps) {    \
        gen(tag(node).value); \
        return;               \
    }

    void visit(Expression::Negation *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Negate, tag(node).deps);
    }

    void visit(Expression::Funct_sin *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Sin, tag(node).deps);
    }

    void visit(Expression::Funct_cos *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Cos, tag(node).deps);
    }

    void visit(Expression::Funct_tan *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Tan, tag(node).deps);
    }

    void visit(Expression::Funct_atan *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::ATan, tag(node).deps);
    }

    void visit(Expression::Funct_asin *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::ASin, tag(node).deps);
    }

    void visit(Expression::Funct_acos *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::ACos, tag(node).deps);
    }

    void visit(Expression::Funct_abs *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Abs, tag(node).deps);
    }

    void visit(Expression::Funct_floor *node) {
        checkConst;
        node->arg1->accept(this);
        gen(ByteCode::Floor, tag(node).deps);
    }

    void visit(Expression::Funct_ceil *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Ceil, tag(node).deps);
    }

    void visit(Expression::Funct_round *node) {
        checkConst;
        node->arg1->accept(this);
        gen(ByteCode::Round, tag(node).deps);
    }

    void visit(Expression::Funct_log *node) {
        checkConst;
        node->arg1->accept(this);
        gen(ByteCode::Log, tag(node).deps);
    }

    void visit(Expression::Funct_exp *node) {
        checkConst;
        node->arg1->accept(this); 
        gen(ByteCode::Exp, tag(node).deps);
    }

    // TODO
    void visit(Expression::Funct_mean1 *node) {gen(0);}
    void visit(Expression::Funct_sum1 *node) {gen(0);}
    void visit(Expression::Funct_max1 *node) {gen(0);}
    void visit(Expression::Funct_min1 *node) {gen(0);}
    void visit(Expression::Funct_variance1 *node) {gen(0);}
    void visit(Expression::Funct_stddev1 *node) {gen(0);}
    void visit(Expression::Funct_skew1 *node) {gen(0);}
    void visit(Expression::Funct_kurtosis1 *node) {gen(0);}
    void visit(Expression::Funct_covariance *node) {gen(0);}
    
    void visit(Expression::LTE *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::LTE, tag(node).deps);
    }
    
    void visit(Expression::GTE *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::GTE, tag(node).deps);
    }
    
    void visit(Expression::LT *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::LT, tag(node).deps);
    }
    
    void visit(Expression::GT *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::GT, tag(node).deps);
    }
    
    void visit(Expression::EQ *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::EQ, tag(node).deps);
    }
    
    void visit(Expression::NEQ *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::EQ, tag(node).deps);
    }
    
    void visit(Expression::Plus *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Plus, tag(node).deps);
    }
    
    void visit(Expression::Minus *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Minus, tag(node).deps);
    }
    
    void visit(Expression::Times *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Times, tag(node).deps);
    }
    
    void visit(Expression::Mod *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Mod, tag(node).deps);
    }
    
    void visit(Expression::Divide *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Divide, tag(node).deps);
    }
    
    void visit(Expression::Power *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::Pow, tag(node).deps);
    }
    
    void visit(Expression::Funct_atan2 *node) {
        checkConst;
        node->arg1->accept(this); 
        node->arg2->accept(this);
        gen(ByteCode::ATan2, tag(node).deps);
    }
    
    void visit(Expression::IfThenElse *node) {
        checkConst;
        node->arg1->accept(this);        
        ByteCode::Op op;
        switch(last().op) {
        case ByteCode::LT:
            op = ByteCode::IfLT;
            pop();
            break;
        case ByteCode::GT:
            op = ByteCode::IfGT;
            pop();
            break;
        case ByteCode::LTE:
            op = ByteCode::IfLTE;
            pop();
            break;
        case ByteCode::GTE:
            op = ByteCode::IfGTE;
            pop();
            break;
        case ByteCode::EQ:
            op = ByteCode::IfEQ;
            pop();
            break;
        case ByteCode::NEQ:
            op = ByteCode::IfNEQ;
            pop();
            break;
        default:
            op = ByteCode::IfNZ;
            break;
        }
        node->arg2->accept(this);
        node->arg3->accept(this);
        gen(op, tag(node).deps);
    }
    
    void visit(Expression::SampleHere *node) {
        node->arg1->accept(this);
        gen(ByteCode::SampleHere, tag(node).deps);
    }
    
    void visit(Expression::Sample2D *node) {
        node->arg1->accept(this);
        node->arg2->accept(this);
        gen(ByteCode::Sample2D, tag(node).deps);
    }
    
    void visit(Expression::Sample3D *node) {
        node->arg1->accept(this);
        node->arg2->accept(this);
        node->arg3->accept(this);
        gen(ByteCode::Sample3D, tag(node).deps);
    }

    #undef checkConst

};

#include "footer.h"
#endif
