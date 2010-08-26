#ifndef IMAGESTACK_COMPILER_H
#define IMAGESTACK_COMPILER_H

#include <vector>
#include "Parser.h"
#include "X64.h"
#include "header.h"

const char *opname[] = {"ConstFloat", "ConstBool", "NoOp",
                        "VarX", "VarY", "VarT", "VarC", "VarVal", "Plus", "Minus", 
                        "Times", "Divide", "Sin", "Cos", "Tan", "Power",
                        "ASin", "ACos", "ATan", "ATan2", "Abs", "Floor", "Ceil", "Round",
                        "Exp", "Log", "Mod", "SampleHere", "Sample2D", "Sample3D",
                        "LT", "GT", "LTE", "GTE", "EQ", "NEQ",
                        "And", "Or", "Nand"};    

class Compiler : public Expression::Visitor {

    enum {ConstFloat = 0, ConstBool, NoOp, 
          VarX, VarY, VarT, VarC, VarVal, Plus, Minus, 
          Times, Divide, Sin, Cos, Tan, Power,
          ASin, ACos, ATan, ATan2, Abs, Floor, Ceil, Round,
          Exp, Log, Mod, SampleHere, Sample2D, Sample3D,
          LT, GT, LTE, GTE, EQ, NEQ,
          And, Or, Nand};
    
    enum {DepT = 1, DepY = 2, DepX = 4, DepC = 8, DepVal = 16};

    enum Type {Unknown = 0, Float, Bool};
    
    // One node in the intermediate representation
    struct IRNode {
        IRNode(float v) {
            op = ConstFloat;
            val = v;
            deps = 0;
            reg = -1;
            level = 0;
            type = Float;
        }

        IRNode(Type t, uint32_t opcode, 
               Type t1 = Float, IRNode *child1 = NULL, 
               Type t2 = Float, IRNode *child2 = NULL, 
               Type t3 = Float, IRNode *child3 = NULL,
               Type t4 = Float, IRNode *child4 = NULL) {
            type = t;
            deps = 0;
            op = opcode;
            if (opcode == VarX) deps |= DepX;
            else if (opcode == VarY) deps |= DepY;
            else if (opcode == VarT) deps |= DepT;
            else if (opcode == VarC) deps |= DepC;
            else if (opcode == VarVal) deps |= DepVal;

            printf("%d: %s", (int)t, opname[opcode]);
            if (child1) printf(" %d (%d)", (int)t1, child1->type);
            if (child2) printf(" %d (%d)", (int)t2, child2->type);
            if (child3) printf(" %d (%d)", (int)t3, child3->type);
            if (child4) printf(" %d (%d)", (int)t4, child4->type);
            printf("\n");

            if (child1) children.push_back(child1);
            if (child2) children.push_back(child2);
            if (child3) children.push_back(child3);
            if (child4) children.push_back(child4);
            Type childTypes[] = {t1, t2, t3, t4};

            for (size_t i = 0; i < children.size(); i++) {
                IRNode *c = children[i];
                Type ct = childTypes[i];
                if (c->type == Float && ct == Bool) {
                    c = new IRNode(Bool, NEQ, Float, c, Float, new IRNode(0.0f));
                } else if (c->type == Bool && ct == Float) {
                    c = new IRNode(Float, And, Bool, c, Float, new IRNode(1.0f));
                } 
                children[i] = c;
                c->parents.push_back(this);
                deps |= c->deps;
            }

            reg = -1;
            if (deps & DepVal ||
                deps & DepC) level = 4;
            else if (deps & DepX) level = 3;
            else if (deps & DepY) level = 2;
            else if (deps & DepT) level = 1;
            else level = 0;

        }

        // Opcode
        uint32_t op;
        
        // This is for Const ops
        float val;

        // Inputs
        vector<IRNode *> children;    
        
        // Who uses my value?
        vector<IRNode *> parents;
        
        // Which loop variables does this node depend on?
        uint32_t deps;

        // In what order is this instruction evaluated
        int32_t order;

        // What register will this node be computed in?
        signed char reg;
               
        // What level of the for loop will this node be computed at?
        // 0 is outermost, 4 is deepest
        signed char level;

        // What is the type of this expression?
        Type type;

        // Can one node nuke the register assigned to another
        // node. Undefined if registers haven't been assigned yet.
        bool canDestroy(const IRNode *other) {
            if (other->level != level) return false;
            if (other->order > order) return false;
            // I have to be the last-evaluated parent of the node to
            // be able to destroy it
            for (size_t i = 0; i < other->parents.size(); i++) {
                if (other->parents[i]->level != level) return false;
                if (other->parents[i]->order > order) return false;
            }
            return true;
        }
    };

    // State needed while visiting the AST
    IRNode *root;
    Stats *stats;
    Window im;

public:

    void compileEval(AsmX64 *a, Window w, Window out, const Expression &exp) {

        im = w;
        stats = new Stats(w);

        // Generate the intermediate representation from the AST. Also
        // does constant folding and dependency analysis (assigns deps and levels).
        exp.root->accept(this);

        // Register assignment and evaluation ordering
        vector<vector<IRNode *> > ordering = doRegisterAssignment();

        for (size_t l = 0; l < ordering.size(); l++) {
            for (size_t i = 0; i < ordering[l].size(); i++) {
                IRNode *next = ordering[l][i];
                for (size_t k = 0; k < l; k++) putchar(' ');
                printf("%s: xmm%d", opname[next->op], next->reg);
                if (next->op == ConstFloat) {
                    printf(" <- %f\n", next->val);
                } else if (next->children.size() == 0) {
                    printf("\n");
                } else {
                    printf(" <-");
                    for (size_t j = 0; j < next->children.size(); j++) {
                        printf(" xmm%d", next->children[j]->reg);
                    }
                    printf("\n");
                }            
            }
        }

        AsmX64::Reg x = a->rax, y = a->rcx, 
            t = a->r8, c = a->rsi, 
            imPtr = a->rdx, tmp = a->r15,
            outPtr = a->rdi;


        // save registers
        a->pushNonVolatiles();

        // generate constants
        compileBody(a, x, y, t, c, imPtr, ordering[0]);
        a->mov(t, 0);
        a->label("tloop"); 

        // generate the values that don't depend on Y, X, C, or val
        compileBody(a, x, y, t, c, imPtr, ordering[1]);        
        a->mov(y, 0);
        a->label("yloop"); 

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
        compileBody(a, x, y, t, c, imPtr, ordering[2]);        
        a->mov(x, 0);               
        a->label("xloop"); 
        
        // generate the values that don't depend on C or val
        compileBody(a, x, y, t, c, imPtr, ordering[3]);        
        a->mov(c, 0);
        a->label("cloop"); 
                        
        // insert code for the expression body
        compileBody(a, x, y, t, c, imPtr, ordering[4]);
        a->movss(outPtr, AsmX64::SSEReg(root->reg));
                        
        a->add(c, 1);
        a->add(imPtr, 4);
        a->add(outPtr, 4);
        a->cmp(c, im.channels);
        a->jl("cloop");
        a->add(x, 1);
        a->cmp(x, im.width);
        a->jl("xloop");
        a->add(y, 1);
        a->cmp(y, im.height);
        a->jl("yloop");            
        a->add(t, 1);
        a->cmp(t, im.frames);
        a->jl("tloop");            
        a->popNonVolatiles();
        a->ret();        
        a->saveCOFF("generated.obj");        
    }

    void compileBody(AsmX64 *a, 
                     AsmX64::Reg x, AsmX64::Reg y,
                     AsmX64::Reg t, AsmX64::Reg c, 
                     AsmX64::Reg ptr, vector<IRNode *> code) {
        
        AsmX64::SSEReg tmp = a->xmm15;

        for (size_t i = 0; i < code.size(); i++) {
            // extract the node, its register, and any children and their registers
            IRNode *node = code[i];
            IRNode *c1 = (node->children.size() >= 1) ? node->children[0] : NULL;
            IRNode *c2 = (node->children.size() >= 2) ? node->children[1] : NULL;
            IRNode *c3 = (node->children.size() >= 3) ? node->children[2] : NULL;
            IRNode *c4 = (node->children.size() >= 4) ? node->children[3] : NULL;
            AsmX64::SSEReg dst(node->reg);
            AsmX64::SSEReg src1(c1 ? c1->reg : 0);
            AsmX64::SSEReg src2(c2 ? c2->reg : 0);
            AsmX64::SSEReg src3(c3 ? c3->reg : 0);
            AsmX64::SSEReg src4(c4 ? c4->reg : 0);

            switch(node->op) {
            case ConstFloat: 
                if (node->val == 0.0f) {
                    a->bxorps(dst, dst);
                } else {
                    a->mov(a->r15, &(node->val));
                    a->movss(dst, AsmX64::Mem(a->r15));
                }
                break;
            case ConstBool:
                if (node->val == 0.0f) {
                    a->bxorps(dst, dst);                    
                } else {
                    a->cmpeqss(dst, dst);
                }
                break;
            case VarX:
                a->cvtsi2ss(dst, x);
                break;
            case VarY:
                a->cvtsi2ss(dst, y);
                break;
            case VarT:
                a->cvtsi2ss(dst, t);
                break;
            case VarC:
                a->cvtsi2ss(dst, c);
                break;
            case VarVal:
                a->movss(dst, AsmX64::Mem(ptr));
                break;
            case Plus:
                if (dst == src1)
                    a->addss(dst, src2);
                else if (dst == src2) 
                    a->addss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->addss(dst, src2);
                }
                break;
            case Minus:
                if (dst == src1) {
                    a->subss(dst, src2);
                } else if (dst == src2) {
                    a->movss(tmp, src2);
                    a->movss(src2, src1);
                    a->subss(src2, tmp);
                } else { 
                    a->movss(dst, src1);
                    a->subss(dst, src2);
                }
                break;
            case Times: 
                if (dst == src1)
                    a->mulss(dst, src2);
                else if (dst == src2) 
                    a->mulss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->mulss(dst, src2);
                }
                break;
            case Divide: 
                if (dst == src1) {
                    a->divss(dst, src2);
                } else if (dst == src2) {
                    a->movss(tmp, src2);
                    a->movss(src2, src1);
                    a->divss(src2, tmp); 
                } else {
                    a->movss(dst, src1);
                    a->divss(dst, src2);
                }
                break;
            case And:
                if (dst == src1) 
                    a->bandps(dst, src2);
                else if (dst == src2)
                    a->bandps(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->bandps(dst, src2);
                }
                break;
            case Nand:
                if (dst == src1) {
                    a->bandnps(dst, src2);
                } else if (dst == src2) {
                    a->movss(tmp, src2);
                    a->movss(src2, src1);
                    a->bandnps(src2, tmp); 
                } else {
                    a->movss(dst, src1);
                    a->bandnps(dst, src2);
                }
                break;
            case Or:               
                if (dst == src1) 
                    a->borps(dst, src2);
                else if (dst == src2)
                    a->borps(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->borps(dst, src2);
                }
                break;
            case NEQ:                               
                if (dst == src1) 
                    a->cmpneqss(dst, src2);
                else if (dst == src2)
                    a->cmpneqss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpneqss(dst, src2);
                }
                break;
            case EQ:
                if (dst == src1) 
                    a->cmpeqss(dst, src2);
                else if (dst == src2)
                    a->cmpeqss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpeqss(dst, src2);
                }
                break;
            case LT:
                if (dst == src1) 
                    a->cmpltss(dst, src2);
                else if (dst == src2)
                    a->cmpnless(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpltss(dst, src2);
                }
                break;
            case GT:
                if (dst == src1) 
                    a->cmpnless(dst, src2);
                else if (dst == src2)
                    a->cmpltss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpnless(dst, src2);
                }
                break;
            case LTE:
                if (dst == src1) 
                    a->cmpless(dst, src2);
                else if (dst == src2)
                    a->cmpnltss(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpless(dst, src2);
                }
                break;
            case GTE:
                if (dst == src1) 
                    a->cmpnltss(dst, src2);
                else if (dst == src2)
                    a->cmpless(dst, src1);
                else {
                    a->movss(dst, src1);
                    a->cmpnltss(dst, src2);
                }
                break;
            case ATan2:
            case Mod:
            case Power:
            case Sin:
            case Cos:
            case Tan:
            case ASin:
            case ACos:
            case ATan:
            case Exp:
            case Log:
            case Floor:
            case Ceil:
            case Round:
            case Abs:
                
            case Sample2D: 
            case Sample3D:
            case SampleHere:
                printf("Not implemented: %s\n", opname[node->op]);                
                break;
            case NoOp:
                break;
            }
        }
    }

protected:

    vector<vector<IRNode *> > doRegisterAssignment() {
        // reserve xmm15 for the code generator
        vector<IRNode *> regs(15);
        
        // the resulting evaluation order
        vector<vector<IRNode *> > order(5);
        regAssign(root, regs, order);
        return order;
    }
       
    void regAssign(IRNode *node, vector<IRNode *> &regs, vector<vector<IRNode *> > &order) {
        // if I already have a register bail out
        if (node->reg >= 0) return;

        // assign registers to the children
        for (size_t i = 0; i < node->children.size(); i++) {
            regAssign(node->children[i], regs, order);
        }

        // if there are children, see if we can use the register of
        // the one of the children - the first is optimal, as this
        // makes x64 codegen easier. To reuse the register of the
        // child it has to be at the same level as us (otherwise it
        // will have been computed once and stored outside the for
        // loop this node lives in), and have no other
        // parents. There's an exception to this: if you're the last
        // parent of a set of parents to use a child's value, it's OK
        // to clobber. We currently don't handle this exception
        // (because we currently don't do CSE, so it never crops up).

        if (node->children.size()) {
            IRNode *child1 = node->children[0];
            if (node->level == child1->level &&
                child1->parents.size() == 1) {
                node->reg = child1->reg;
                regs[child1->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }
        
        // Some binary ops are easy to flip, so we should try to clobber the second child
        if (node->op == And ||
            node->op == Or ||
            node->op == Plus ||
            node->op == Times ||
            node->op == LT ||
            node->op == GT ||
            node->op == LTE ||
            node->op == GTE ||
            node->op == EQ ||
            node->op == NEQ) {
            IRNode *child2 = node->children[1];
            if (node->level == child2->level &&
                child2->parents.size() == 1) {
                node->reg = child2->reg;
                regs[child2->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        printf("Couldn't clobber first child, looking for a free register instead\n");

        // else find a completely unused register and use that. This
        // will certainly provoke codegen into inserting extra movs.
        for (size_t i = 0; i < regs.size(); i++) {
            if (regs[i] == NULL) {
                node->reg = i;
                regs[i] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        printf("No free registers. Clobbering a different child\n");

        for (size_t i = 1; i < node->children.size(); i++) {
            IRNode *child = node->children[i];
            if (node->level == child->level &&
                child->parents.size() == 1) {
                node->reg = child->reg;
                regs[child->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        printf("No children can be clobbered, searching for someone to evict\n");

        // else find a previously used register that is safe to evict
        // - meaning it's at the same or higher level and all its
        // parents will have already been evaluated and are at the same or higher level
        for (size_t i = 0; i < regs.size(); i++) {            
            if (regs[i]->level < node->level) {
                printf("Register %d is holding a value from a higher level (%d vs %d)\n",
                       i, node->level, regs[i]->level);
                continue;
            }
            bool safeToEvict = true;            
            for (size_t j = 0; j < regs[i]->parents.size(); j++) {
                if (regs[i]->parents[j]->reg < 0 ||
                    regs[i]->parents[j]->level > node->level) {
                    safeToEvict = false;
                    break;
                }
            }

            if (safeToEvict) {
                printf("Good to evict the node in register %d, because its parents have registers: ", i);
                for (size_t j = 0; j < regs[i]->parents.size(); j++) {
                    printf("%d ", regs[i]->parents[j]->reg);
                }
                printf("\n");

                node->reg = i;
                regs[i] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            } else {
                printf("Register %d was still in use :(\n", i);
            }
        }

        // else freak out - we're out of registers and we don't know
        // how to spill to the stack yet.
        panic("Out of registers!\n");
    }



#define nullary(a, b)                           \
    void visit(Expression::##a *node) {         \
        root = new IRNode(Float, b);            \
    }

    nullary(Var_x, VarX);
    nullary(Var_y, VarY);
    nullary(Var_t, VarT);
    nullary(Var_c, VarC);
    nullary(Var_val, VarVal);

#undef nullary

#define constFloat(a, b)                        \
    void visit(Expression::##a *node) {         \
        root = new IRNode(b);                   \
    }

    constFloat(Float, node->value);
    constFloat(Uniform_width, im.width);
    constFloat(Uniform_height, im.height);
    constFloat(Uniform_frames, im.frames);
    constFloat(Uniform_channels, im.channels);
    constFloat(Funct_mean0, stats->mean());
    constFloat(Funct_sum0, stats->sum());
    constFloat(Funct_min0, stats->minimum());
    constFloat(Funct_max0, stats->maximum());
    constFloat(Funct_stddev0, sqrtf(stats->variance()));
    constFloat(Funct_variance0, stats->variance());
    constFloat(Funct_skew0, stats->skew());
    constFloat(Funct_kurtosis0, stats->kurtosis());

    // TODO
    constFloat(Funct_mean1, stats->mean());
    constFloat(Funct_sum1, stats->sum());
    constFloat(Funct_min1, stats->minimum());
    constFloat(Funct_max1, stats->maximum());
    constFloat(Funct_stddev1, sqrtf(stats->variance()));
    constFloat(Funct_variance1, stats->variance());
    constFloat(Funct_skew1, stats->skew());
    constFloat(Funct_kurtosis1, stats->kurtosis());    
    constFloat(Funct_covariance, stats->variance());    

#undef constFloat

    // roundf is a macro, which gets confused inside another macro, so
    // we wrap it in a function
    float myRound(float x) {
        return roundf(x);
    }

    // -x gets compiled to (0-x)
    void visit(Expression::Negation *node) {
        node->arg1->accept(this);
        if (root->op == ConstFloat) root->val = -root->val;
        else root = new IRNode(Float, Minus, Float, new IRNode(0.0f), Float, root);
    }

#define unary(a, b, c)                                          \
    void visit(Expression::##a *node) {                         \
        node->arg1->accept(this);                               \
        if (root->op == ConstFloat) root->val = b(root->val);   \
        else root = new IRNode(Float, c, Float, root);          \
    }

    unary(Funct_sin, sinf, Sin);
    unary(Funct_cos, cosf, Cos);
    unary(Funct_tan, tanf, Tan);
    unary(Funct_asin, asinf, ASin);
    unary(Funct_acos, acosf, ACos);
    unary(Funct_atan, atanf, ATan);
    unary(Funct_abs, fabs, Abs);
    unary(Funct_floor, floorf, Floor);
    unary(Funct_ceil, ceilf, Ceil);
    unary(Funct_round, myRound, Round);
    unary(Funct_log, logf, Log);
    unary(Funct_exp, expf, Exp);

#undef unary

// TODO: boolean eq and neq
#define bincmp(a, b)                                                    \
    void visit(Expression::##a *node) {                                 \
        node->arg1->accept(this); IRNode *child1 = root;                \
        node->arg2->accept(this); IRNode *child2 = root;                \
        if (child1->op == ConstFloat && child2->op == ConstFloat) {     \
            root->val = (child1->val b child2->val) ? 1.0f : 0.0f;      \
            delete child1;                                              \
        } else {                                                        \
            root = new IRNode(Bool, a, Float, child1, Float, child2);   \
        }                                                               \
    } 


    bincmp(LTE, <=);
    bincmp(LT, <);
    bincmp(GT, >);
    bincmp(GTE, >=);
    bincmp(EQ, ==);
    bincmp(NEQ, !=);

#undef bincmp

// TODO: bool * bool = bool
#define binop(a, b)                                             \
    void visit(Expression::##a *node) {                         \
        node->arg1->accept(this); IRNode *child1 = root;        \
        node->arg2->accept(this); IRNode *child2 = root;        \
        if (child1->op == ConstFloat && child2->op == ConstFloat) {     \
            root->val = child1->val b child2->val;              \
            delete child1;                                      \
        } else {                                                \
            root = new IRNode(Float, a, Float, child1, Float, child2);  \
        }                                                               \
    } 

    binop(Plus, +);
    binop(Minus, -);
    binop(Times, *);
    binop(Divide, /);

#undef binop

#define binfunc(a, b, c)                                        \
    void visit(Expression::##a *node) {                         \
        node->arg1->accept(this); IRNode *child1 = root;        \
        node->arg2->accept(this); IRNode *child2 = root;        \
        if (child1->op == ConstFloat && child2->op == ConstFloat) {     \
            root->val = b(child1->val, child2->val);            \
            delete child1;                                      \
        } else {                                                \
            root = new IRNode(Float, c, Float, child1, Float, child2);   \
        }                                                       \
    }


    binfunc(Mod, fmod, Mod);
    binfunc(Funct_atan2, atan2f, ATan2);
    binfunc(Power, powf, Power);

#undef binfunc
    
    void visit(Expression::IfThenElse *node) {
        node->arg1->accept(this);        
        IRNode *cond = root;
        node->arg2->accept(this);
        IRNode *thenCase = root;
        node->arg3->accept(this);
        IRNode *elseCase = root;
        IRNode *left = new IRNode(thenCase->type, And, Bool, cond, thenCase->type, thenCase);
        IRNode *right = new IRNode(elseCase->type, Nand, Bool, cond, elseCase->type, elseCase);
        Type t = Float;
        if (thenCase->type == Bool && elseCase->type == Bool) 
            t = Bool;
        root = new IRNode(t, Or, left->type, left, right->type, right);

    }
    
    void visit(Expression::SampleHere *node) {        
        node->arg1->accept(this);
        root = new IRNode(Float, SampleHere, Float, root);
    }
    
    void visit(Expression::Sample2D *node) {
        node->arg1->accept(this); IRNode *child1 = root;
        node->arg2->accept(this); IRNode *child2 = root;
        root = new IRNode(Float, Sample2D, Float, child1, Float, child2);
    }
    
    void visit(Expression::Sample3D *node) {
        node->arg1->accept(this); IRNode *child1 = root;
        node->arg2->accept(this); IRNode *child2 = root;
        node->arg3->accept(this); IRNode *child3 = root;
        root = new IRNode(Float, Sample3D, Float, child1, Float, child2, Float, child3);
    }
    
};

#include "footer.h"
#endif
