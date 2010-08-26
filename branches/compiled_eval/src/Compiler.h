#ifndef IMAGESTACK_COMPILER_H
#define IMAGESTACK_COMPILER_H

#include <vector>
#include "Parser.h"
#include "X64.h"
#include "header.h"

const char *opname[] = {"Const", "NoOp", "VarX", "VarY", "VarT", "VarC", "VarVal", "Negate", "Plus", "Minus", 
                        "Times", "Divide", "Sin", "Cos", "Tan", "Power",
                        "ASin", "ACos", "ATan", "ATan2", "Abs", "Floor", "Ceil", "Round",
                        "Exp", "Log", "Mod", "SampleHere", "Sample2D", "Sample3D",
                        "IfLT", "IfGT", "IfLTE", "IfGTE", "IfEQ", "IfNEQ"};    

class Compiler : public Expression::Visitor {

    enum {Const = 0, NoOp, VarX, VarY, VarT, VarC, VarVal, Negate, Plus, Minus, 
          Times, Divide, Sin, Cos, Tan, Power,
          ASin, ACos, ATan, ATan2, Abs, Floor, Ceil, Round,
          Exp, Log, Mod, SampleHere, Sample2D, Sample3D,
          IfLT, IfGT, IfLTE, IfGTE, IfEQ, IfNEQ};
    
    enum {DepT = 1, DepY = 2, DepX = 4, DepC = 8, DepVal = 16};
    
    // One node in the intermediate representation
    struct IRNode {
        IRNode(float v) {
            op = Const;
            val = v;
            deps = 0;
            reg = -1;
            level = 0;
        }

        IRNode(uint32_t opcode, 
               IRNode *child1 = NULL, 
               IRNode *child2 = NULL, 
               IRNode *child3 = NULL,
               IRNode *child4 = NULL) {
            deps = 0;
            op = opcode;
            if (opcode == VarX) deps |= DepX;
            else if (opcode == VarY) deps |= DepY;
            else if (opcode == VarT) deps |= DepT;
            else if (opcode == VarC) deps |= DepC;
            else if (opcode == VarVal) deps |= DepVal;

            if (child1) {
                children.push_back(child1);
                child1->parents.push_back(this);
                deps |= child1->deps;
            }

            if (child2) {
                children.push_back(child2);
                child2->parents.push_back(this);
                deps |= child2->deps;
            }

            if (child3) {
                children.push_back(child3);
                child3->parents.push_back(this);
                deps |= child3->deps;
            }

            if (child4) {
                children.push_back(child4);
                child4->parents.push_back(this);
                deps |= child4->deps;
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

        // What register will this node be computed in?
        signed char reg;
               
        // What level of the for loop will this node be computed at?
        // 0 is outermost, 4 is deepest
        signed char level;
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
                if (next->op == Const) {
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
            case Const: 
                a->mov(a->r15, &code[i].val);
                a->movss(dst, AsmX64::Mem(a->r15));
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
                if (dst == src1)
                    a->subss(dst, src2);
                else {
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
                if (dst == src1)
                    a->divss(dst, src2);
                else {
                    a->movss(dst, src1);
                    a->divss(dst, src2);
                }
                break;
            case IfNEQ:
                // TODO: special case if either src3 or src4 is zero
                if (dst == src1) {
                    a->cmpneqss(src1, src2);
                    a->movss(tmp, src3);
                    a->bandps(tmp, src1);
                    a->bandnps(src1, src4);
                    a->borps(src1, tmp);
                } else if (dst == src2) {
                    a->cmpneqss(src2, src1);
                    a->movss(tmp, src3);
                    a->bandps(tmp, src2);
                    a->bandnps(src2, src4);
                    a->borps(src2, tmp);                    
                } else if (dst == src3) {
                    a->movss(tmp, src1);
                    a->cmpneqss(tmp, src2);
                    a->bandps(src3, tmp);
                    a->bandnps(tmp, src4);
                    a->borps(src3, tmp);                    
                } else if (dst == src4) {
                    
                }
                a->cmpneqss(src1, src2);
                a->bandps(src3, src1);
                a->bandnps(src1, src4);
                a->borps(src1, src3);
                break;
            case IfEQ:
                a->cmpeqss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case IfLT:
                a->cmpltss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case IfGT:
                a->cmpnless(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case IfLTE:
                a->cmpless(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case IfGTE:
                a->cmpnltss(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-3));
                a->bandps(AsmX64::SSEReg(r-2), AsmX64::SSEReg(r-4));
                a->bandnps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-1));
                a->borps(AsmX64::SSEReg(r-4), AsmX64::SSEReg(r-2));
                r -= 3;
                break;
            case ATan2:
            case Sample2D: 
            case Sample3D:
            case SampleHere:
            case Mod:
            case Pow:
            case Sin:
            case Cos:
            case Tan:
            case ASin:
            case ACos:
            case ATan:
            case Exp:
            case Log:
            case Negate:
            case Floor:
            case Ceil:
            case Round:
            case Abs:
                printf("Not implemented\n");                
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

        for (size_t i = 0; i < node->children.size(); i++) {
            IRNode *child = node->children[i];
            if (node->level == child->level &&
                child->parents.size() == 1) {
                node->reg = child->reg;
                regs[child->reg] = node;
                order[node->level].push_back(node);
                return;
            }
        }

        printf("Couldn't clobber a child, looking for a free register instead\n");

        // else find a completely unused register and use that. This
        // will certainly provoke codegen into inserting extra movs.
        for (size_t i = 0; i < regs.size(); i++) {
            if (regs[i] == NULL) {
                node->reg = i;
                regs[i] = node;
                order[node->level].push_back(node);
                return;
            }
        }

        printf("No free registers, searching for someone to evict\n");

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
        root = new IRNode(b);                   \
    }

    nullary(Var_x, (uint32_t)VarX);
    nullary(Var_y, (uint32_t)VarY);
    nullary(Var_t, (uint32_t)VarT);
    nullary(Var_c, (uint32_t)VarC);
    nullary(Var_val, (uint32_t)VarVal);
    nullary(Float, node->value);
    nullary(Uniform_width, (float)im.width);
    nullary(Uniform_height, (float)im.height);
    nullary(Uniform_frames, (float)im.frames);
    nullary(Uniform_channels, (float)im.channels);
    nullary(Funct_mean0, (float)stats->mean());
    nullary(Funct_sum0, (float)stats->sum());
    nullary(Funct_min0, (float)stats->minimum());
    nullary(Funct_max0, (float)stats->maximum());
    nullary(Funct_stddev0, sqrtf((float)stats->variance()));
    nullary(Funct_variance0, (float)stats->variance());
    nullary(Funct_skew0, (float)stats->skew());
    nullary(Funct_kurtosis0, (float)stats->kurtosis());

    // TODO
    nullary(Funct_mean1, (float)stats->mean());
    nullary(Funct_sum1, (float)stats->sum());
    nullary(Funct_min1, (float)stats->minimum());
    nullary(Funct_max1, (float)stats->maximum());
    nullary(Funct_stddev1, sqrtf((float)stats->variance()));
    nullary(Funct_variance1, (float)stats->variance());
    nullary(Funct_skew1, (float)stats->skew());
    nullary(Funct_kurtosis1, (float)stats->kurtosis());    
    nullary(Funct_covariance, (float)stats->variance());    

#undef nullary

    // roundf is a macro, which gets confused inside another macro, so
    // we wrap it in a function
    float myRound(float x) {
        return roundf(x);
    }

#define unary(a, b, c)                                          \
    void visit(Expression::##a *node) {                         \
        node->arg1->accept(this);                               \
        if (root->op == Const) root->val = b(root->val);        \
        else root = new IRNode(c, root);                        \
    }

    unary(Negation, -, Negate);
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

#define bincmp(a, b)                                                    \
    void visit(Expression::##a *node) {                                 \
        node->arg1->accept(this); IRNode *child1 = root;                \
        node->arg2->accept(this); IRNode *child2 = root;                \
        if (child1->op == Const && child2->op == Const) {               \
            root->val = (child1->val b child2->val) ? 1.0f : 0.0f;      \
            delete child1;                                              \
        } else {                                                        \
            root = new IRNode(If##a, child1, child2,                    \
                              new IRNode(1.0), new IRNode(0.0));        \
        }                                                               \
    } 


    bincmp(LTE, <=);
    bincmp(LT, <);
    bincmp(GT, >);
    bincmp(GTE, >=);
    bincmp(EQ, ==);
    bincmp(NEQ, ==);

#undef bincmp

#define binop(a, b)                                             \
    void visit(Expression::##a *node) {                         \
        node->arg1->accept(this); IRNode *child1 = root;        \
        node->arg2->accept(this); IRNode *child2 = root;        \
        if (child1->op == Const && child2->op == Const) {       \
            root->val = child1->val b child2->val;               \
            delete child1;                                      \
        } else {                                                \
            root = new IRNode(a, child1, child2);               \
        }                                                       \
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
        if (child1->op == Const && child2->op == Const) {       \
            root->val = b(child1->val, child2->val);             \
            delete child1;                                      \
        } else {                                                \
            root = new IRNode(c, child1, child2);               \
        }                                                       \
    }


    binfunc(Mod, fmod, Mod);
    binfunc(Funct_atan2, atan2f, ATan2);
    binfunc(Power, powf, Power);

#undef binfunc
    
    void visit(Expression::IfThenElse *node) {
        node->arg1->accept(this);        
        switch(root->op) {
        case LT:
            root->op = IfLT;
            break;
        case GT:
            root->op = IfGT;
            break;
        case LTE:
            root->op = IfLTE;
            break;
        case GTE:
            root->op = IfGTE;
            break;
        case EQ:
            root->op = IfEQ;
            break;
        case NEQ:
            root->op = IfNEQ;
            break;
        default: {            
            IRNode *child1 = root;            
            node->arg2->accept(this);
            IRNode *child2 = root;
            node->arg3->accept(this);
            IRNode *child3 = root;
            root = new IRNode(IfNEQ, child1, new IRNode(0.0f), child2, child3);            
            return;
        }
        }

        // upgrade child1 with some extra children and a fancier
        // opcode instead of adding a new node
        IRNode *child1 = root;            
        node->arg2->accept(this);
        child1->children.push_back(root);
        node->arg3->accept(this);
        child1->children.push_back(root);
        root = child1;
    }
    
    void visit(Expression::SampleHere *node) {        
        node->arg1->accept(this);
        root = new IRNode(SampleHere, root);
    }
    
    void visit(Expression::Sample2D *node) {
        node->arg1->accept(this); IRNode *child1 = root;
        node->arg2->accept(this); IRNode *child2 = root;
        root = new IRNode(Sample2D, child1, child2);
    }
    
    void visit(Expression::Sample3D *node) {
        node->arg1->accept(this); IRNode *child1 = root;
        node->arg2->accept(this); IRNode *child2 = root;
        node->arg3->accept(this); IRNode *child3 = root;
        root = new IRNode(Sample3D, child1, child2, child3);
    }
    
};

#include "footer.h"
#endif
