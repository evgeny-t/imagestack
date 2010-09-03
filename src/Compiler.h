#ifndef IMAGESTACK_COMPILER_H
#define IMAGESTACK_COMPILER_H

#include <vector>
#include "Parser.h"
#include "X64.h"
#include "header.h"

static const char *opname[] = {"Const", "NoOp",
                               "VarX", "VarY", "VarT", "VarC", 
                               "Plus", "Minus", "Times", "Divide", "Power",
                               "Sin", "Cos", "Tan", "ASin", "ACos", "ATan", "ATan2", 
                               "Abs", "Floor", "Ceil", "Round",
                               "Exp", "Log", "Mod", 
                               "LT", "GT", "LTE", "GTE", "EQ", "NEQ",
                               "And", "Or", "Nand", "Load",
                               "IntToFloat", "FloatToInt", 
                               "LoadImm", "PlusImm", "TimesImm"};


enum OpCode {Const = 0, NoOp, 
             VarX, VarY, VarT, VarC, 
             Plus, Minus, Times, Divide, Power,
             Sin, Cos, Tan, ASin, ACos, ATan, ATan2, 
             Abs, Floor, Ceil, Round,
             Exp, Log, Mod, 
             LT, GT, LTE, GTE, EQ, NEQ,
             And, Or, Nand,
             Load,
             IntToFloat, FloatToInt, 
             LoadImm, PlusImm, TimesImm};

enum {DepT = 1, DepY = 2, DepX = 4, DepC = 8, DepMem = 16};

enum Type {Unknown = 0, Float, Bool, Int};

class Compiler {

    
    // One node in the intermediate representation
    class IRNode {
    public:
        // Opcode
        OpCode op;
        
        // Data for Const ops
        float fval;
        int ival;

        // Derivative w.r.t vectorized variable

        // This is a way of preventing early expansion to full vectors
        // for expressions that are linear w.r.t the vectorized
        // variable. For example, when loading some image data whilst
        // vectorized across X, usually the load addresses will be
        // linear in X, and don't need to be stored separately.

        int idv; // only valid if type is Int and width is 1
        float fdv; // only valid if type is Float and width is 1
        
        // Vector width. Should be one or four on X64.
        int width;

        // Inputs
        vector<IRNode *> inputs;    
        
        // Who uses my value?
        vector<IRNode *> outputs;
        
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

        static IRNode *make(float v) {
            if (floatInstances[v] == NULL) 
                return (floatInstances[v] = new IRNode(v));
            return floatInstances[v];
        };

        static IRNode *make(int v) {
            if (intInstances[v] == NULL) 
                return (intInstances[v] = new IRNode(v));
            return intInstances[v];
        };

        static IRNode *make(OpCode opcode, 
                            IRNode *input1 = NULL, 
                            IRNode *input2 = NULL, 
                            IRNode *input3 = NULL,
                            IRNode *input4 = NULL,
                            int ival = 0,
                            float fval = 0.0f) {

            // collect the inputs into a vector
            vector<IRNode *> inputs;
            if (input1) {
                inputs.push_back(input1);
            }
            if (input2) {
                inputs.push_back(input2);
            }
            if (input3) {
                inputs.push_back(input3);
            }
            if (input4) {
                inputs.push_back(input4);
            }
            return make(opcode, inputs, ival, fval);
        }

        static IRNode *make(OpCode opcode,
                            vector<IRNode *> inputs,
                            int ival = 0, float fval = 0.0f) {


            // type inference and coercion
            Type t;
            switch(opcode) {
            case Const:
                panic("Shouldn't make Consts using this make function\n");
            case NoOp:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                t = inputs[0]->type;
                break;
            case VarX: 
            case VarY: 
            case VarT:
            case VarC:
                assert(inputs.size() == 0, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                t = Int;
                break;
            case Plus:
            case Minus:
            case Times:
            case Power:
            case Mod:
                assert(inputs.size() == 2, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                // the output is either int or float
                if (inputs[0]->type == Float ||
                    inputs[1]->type == Float) t = Float;
                else t = Int;

                // upgrade everything to the output type
                inputs[0] = inputs[0]->as(t);
                inputs[1] = inputs[1]->as(t);
                break;
            case Divide:
            case ATan2:
                t = Float;
                inputs[0] = inputs[0]->as(Float);
                inputs[1] = inputs[1]->as(Float);
                break;
            case Sin:
            case Cos:
            case Tan:
            case ASin:
            case ACos:
            case ATan:
            case Exp:
            case Log:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n", 
                       opname[opcode], inputs.size());
                t = Float;
                inputs[0] = inputs[0]->as(Float);
                break;
            case Abs:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n", 
                       opname[opcode], inputs.size());
                if (inputs[0]->type == Bool) return inputs[0];
                t = inputs[0]->type;
                break;
            case Floor:
            case Ceil:
            case Round:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n", 
                       opname[opcode], inputs.size());    
                if (inputs[0]->type != Float) return inputs[0];
                t = Float; // TODO: perhaps Int?
                break;
            case LT:
            case GT:
            case LTE:
            case GTE:
            case EQ:
            case NEQ:
                assert(inputs.size() == 2, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());    
                if (inputs[0]->type == Float || inputs[1]->type == Float) {
                    t = Float;                    
                } else { // TODO: compare ints?
                    t = Bool;
                }
                inputs[0] = inputs[0]->as(t);
                inputs[1] = inputs[1]->as(t);
                t = Bool;
                break;
            case And:
            case Nand:
                assert(inputs.size() == 2, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());    
                // first arg is always bool
                inputs[0] = inputs[0]->as(Bool);
                t = inputs[1]->type;
                break;
            case Or:               
                assert(inputs.size() == 2, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());    
                if (inputs[0]->type == Float || inputs[1]->type == Float) {
                    t = Float;
                } else if (inputs[0]->type == Int || inputs[0]->type == Int) {
                    t = Int;
                } else {
                    t = Bool;
                }
                inputs[0] = inputs[0]->as(t);
                inputs[1] = inputs[1]->as(t);
                break;
            case IntToFloat:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                assert(inputs[0]->type == Int, "IntToFloat can only take integers\n");
                t = Float;
                break;
            case FloatToInt:
                assert(inputs.size() == 1, "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                assert(inputs[0]->type == Float, "FloatToInt can only take floats\n");
                t = Int;
                break;
            case PlusImm:
            case TimesImm:
                assert(inputs.size() == 1,
                       "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                t = Int;
                break;
            case LoadImm:
            case Load:
                assert(inputs.size() == 1,
                       "Wrong number of inputs for opcode: %s %d\n",
                       opname[opcode], inputs.size());
                inputs[0] = inputs[0]->as(Int);
                t = Float;
                break;

            }

            // constant folding
            bool allConst = true;
            for (size_t i = 0; i < inputs.size() && allConst; i++) {
                if (inputs[i]->deps) allConst = false;
            }
            if (allConst && inputs.size()) {
                switch(opcode) {

                case Plus:
                    if (t == Float) {
                        return make(inputs[0]->fval + inputs[1]->fval);
                    } else {
                        return make(inputs[0]->ival + inputs[1]->ival);
                    }
                case Minus:
                    if (t == Float) {
                        return make(inputs[0]->fval - inputs[1]->fval);
                    } else {
                        return make(inputs[0]->ival - inputs[1]->ival);
                    }
                case Times:
                    if (t == Float) {
                        return make(inputs[0]->fval * inputs[1]->fval);
                    } else {
                        return make(inputs[0]->ival * inputs[1]->ival);
                    }
                case PlusImm:
                    return make(inputs[0]->ival + ival);
                case TimesImm:
                    return make(inputs[0]->ival * ival);
                case Divide:
                    return make(inputs[0]->fval / inputs[1]->fval);
                case And:
                    if (t == Float) {
                        return make(inputs[0]->ival ? inputs[1]->fval : 0.0f);
                    } else {
                        return make(inputs[0]->ival ? inputs[1]->ival : 0);
                    }
                case Or:
                    if (t == Float) {
                        return make(inputs[0]->fval + inputs[1]->fval);
                    } else {
                        return make(inputs[0]->ival | inputs[1]->ival);
                    }
                case Nand:
                    if (t == Float) {
                        return make(!inputs[0]->ival ? inputs[1]->fval : 0.0f);
                    } else {
                        return make(!inputs[0]->ival ? inputs[1]->ival : 0);
                    }
                case IntToFloat:
                    return make((float)inputs[0]->ival);
                case FloatToInt:
                    return make((int)inputs[0]->fval);
                default:
                    // TODO: transcendentals, pow, floor, comparisons, etc
                    break;
                } 
            }

            // strength reduction
            if (opcode == NoOp) {
                return inputs[0];
            }

            if (opcode == Divide && inputs[1]->level < inputs[0]->level) {
                // x/alpha = x*(1/alpha)
                return make(Times, inputs[0], 
                            make(Divide, make(1.0f), inputs[1]));
            }

            // (x+a)*b = x*b + a*b (where a and b are both lower level than x)
            if (opcode == Times) {
                IRNode *x = NULL, *a = NULL, *b = NULL;
                if (inputs[0]->op == Plus) {
                    b = inputs[1];
                    x = inputs[0]->inputs[1];
                    a = inputs[0]->inputs[0];
                } else if (inputs[1]->op == Plus) {
                    b = inputs[0];
                    x = inputs[1]->inputs[1];
                    a = inputs[1]->inputs[0];
                }

                if (x) {
                    // x is the higher level of x and a
                    if (x->level < a->level) {
                        IRNode *tmp = x;
                        x = a;
                        a = tmp;
                    }
                    
                    // it's only worth rebalancing if a and b are both
                    // lower level than x (e.g. (x+y)*3)
                    if (x->level > a->level && x->level > b->level) {
                        return make(Plus, 
                                    make(Times, x, b),
                                    make(Times, a, b));
                    }
                }

                // deal with const integer a
                if (inputs[0]->op == PlusImm) {
                    return make(Plus, 
                                make(Times, inputs[0]->inputs[0], inputs[1]),
                                make(Times, inputs[1], make(inputs[0]->ival)));
                }
            }

            // (x*a)*b = x*(a*b) where a and b are lower level than x
            if (opcode == Times) {
                IRNode *x = NULL, *a = NULL, *b = NULL;
                if (inputs[0]->op == Times) {
                    x = inputs[0]->inputs[0];
                    a = inputs[0]->inputs[1];
                    b = inputs[1];
                } else if (inputs[1]->op == Times) {
                    x = inputs[1]->inputs[0];
                    a = inputs[1]->inputs[1];
                    b = inputs[0];
                }

                if (x) {
                    // x is the higher level of x and a
                    if (x->level < a->level) {
                        IRNode *tmp = x;
                        x = a;
                        a = tmp;
                    }
                    
                    // it's only worth rebalancing if a and b are both
                    // lower level than x (e.g. (x+y)*3)
                    if (x->level > a->level && x->level > b->level) {
                        return make(Times, 
                                    x,
                                    make(Times, a, b));
                    }
                }
            }

            /*
            if (opcode == Times) {
                // 0*x = 0
                if (inputs[0]->op == Const &&
                    inputs[0]->type == Float &&
                    inputs[0]->fval == 0.0f) {
                    return make(0.0f);
                } 

                // x*0 = 0
                if (inputs[1]->op == ConstFloat && inputs[1]->val == 0.0f) {
                    return make(0);
                }

                // 1*x = x
                if (inputs[0]->op == ConstFloat && inputs[0]->val == 1.0f) {
                    return inputs[1];
                } 
                // x*1 = x
                if (inputs[1]->op == ConstFloat && inputs[1]->val == 1.0f) {
                    return inputs[0];
                }

                // 2*x = x+x
                if (inputs[0]->op == ConstFloat && inputs[0]->val == 2.0f) {
                    return make(Plus, inputTypes[1], inputs[1], inputTypes[1], inputs[1]);
                } 
                // x*2 = x+x
                if (inputs[1]->op == ConstFloat && inputs[1]->val == 2.0f) {
                    return make(Plus, inputTypes[0], inputs[0], inputTypes[0], inputs[0]);
                }
            }

            if (opcode == Power && inputs[1]->op == ConstFloat) {
                if (inputs[1]->val == 0.0f) {        // x^0 = 1
                    return make(1.0f);
                } else if (inputs[1]->val == 1.0f) { // x^1 = x
                    return inputs[0];
                } else if (inputs[1]->val == 2.0f) { // x^2 = x*x
                    return make(Times, inputTypes[0], inputs[0], inputTypes[0], inputs[0]);
                } else if (inputs[1]->val == 3.0f) { // x^3 = x*x*x
                    IRNode *squared = make(Times, inputTypes[0], inputs[0], inputTypes[0], inputs[0]);
                    return make(t, Times, t, squared, inputTypes[0], inputs[0]);
                } else if (inputs[1]->val == 4.0f) { // x^4 = x*x*x*x
                    IRNode *squared = make(Times, inputTypes[0], inputs[0], inputTypes[0], inputs[0]);
                    return make(Times, t, squared, t, squared);                    
                } else if (inputs[1]->val == floorf(inputs[1]->val) &&
                           inputs[1]->val > 0) {
                    // iterated multiplication
                    int power = (int)floorf(inputs[1]->val);

                    // find the largest power of two less than power
                    int pow2 = 1;
                    while (pow2 < power) pow2 = pow2 << 1;
                    pow2 = pow2 >> 1;                                       
                    int remainder = power - pow2;
                    
                    // make a stack x, x^2, x^4, x^8 ...
                    // and multiply the appropriate terms from it
                    vector<IRNode *> powStack;
                    powStack.push_back(inputs[0]);
                    IRNode *result = (power & 1) ? inputs[0] : NULL;
                    IRNode *last = inputs[0];
                    for (int i = 2; i < power; i *= 2) {
                        last = make(Times, last->type, last, last->type, last);
                        powStack.push_back(last);
                        if (power & i) {
                            if (!result) result = last;
                            else {
                                result = make(Times,
                                              result->type, result,
                                              last->type, last);
                            }
                        }
                    }
                    return result;
                 }

                 // todo: negative powers (integer)               
            }            
            */

            // rebalance summations
            if (opcode != Plus && opcode != Minus && opcode != PlusImm) {
                for (size_t i = 0; i < inputs.size(); i++) {
                    inputs[i] = inputs[i]->rebalanceSum();
                }
            }              

            // deal with variables 
            if (opcode == VarX || opcode == VarY || opcode == VarT || opcode == VarC) {
                vector<IRNode *> b;
                if (varInstances[opcode] == NULL)
                    return (varInstances[opcode] = new IRNode(t, opcode, b, 0, 0.0f));
                return varInstances[opcode];
            }

            // fuse instructions
            if (opcode == Load || opcode == LoadImm) {
                if (inputs[0]->op == Plus) {
                    IRNode *left = inputs[0]->inputs[0];
                    IRNode *right = inputs[0]->inputs[1];
                    if (left->op == Const) {
                        IRNode *n = make(LoadImm, right, 
                                         NULL, NULL, NULL,
                                         left->ival + ival);
                        return n;
                    } else if (right->op == Const) {
                        IRNode *n = make(LoadImm, left,
                                         NULL, NULL, NULL,
                                         right->ival + ival);
                        return n;
                    }
                } else if (inputs[0]->op == Minus &&
                           inputs[0]->inputs[1]->op == Const) {
                    IRNode *n = make(LoadImm, inputs[0]->inputs[0], 
                                     NULL, NULL, NULL, 
                                     -inputs[0]->inputs[1]->ival + ival);
                    return n;
                } else if (inputs[0]->op == PlusImm) {
                    IRNode *n = make(LoadImm, inputs[0]->inputs[0],
                                     NULL, NULL, NULL,
                                     inputs[0]->ival + ival);
                    return n;
                }
            }

            if (opcode == Times && t == Int) {
                IRNode *left = inputs[0];
                IRNode *right = inputs[1];
                if (left->op == Const) {
                    IRNode *n = make(TimesImm, right, 
                                     NULL, NULL, NULL,
                                     left->ival);
                    return n;
                } else if (right->op == Const) {
                    IRNode *n = make(TimesImm, left, 
                                     NULL, NULL, NULL,
                                     right->ival);
                    return n;
                }
            }

            // common subexpression elimination - check if one of the
            // inputs already has a parent that does this op
            if (inputs.size() && inputs[0]->outputs.size() ) {
                for (size_t i = 0; i < inputs[0]->outputs.size(); i++) {
                    IRNode *candidate = inputs[0]->outputs[i];
                    if (candidate->ival != ival) continue;
                    if (candidate->fval != fval) continue;
                    if (candidate->op != opcode) continue;
                    if (candidate->type != t) continue;
                    if (candidate->inputs.size() != inputs.size()) continue;
                    bool inputsMatch = true;
                    for (size_t j = 0; j < inputs.size(); j++) {
                        if (candidate->inputs[j] != inputs[j]) inputsMatch = false;
                    }
                    // it's the same op on the same inputs, reuse the old node
                    if (inputsMatch) return candidate;
                }
            }


            // make a new node
            return new IRNode(t, opcode, inputs, ival, fval);
        }

        // An optimization pass done after generation
        IRNode *optimize() {
            IRNode * newNode = rebalanceSum();
            newNode->killOrphans();
            return newNode;
        }

        

        // kill everything
        static void clearAll() {
            IRNode::floatInstances.clear();
            IRNode::intInstances.clear();
            IRNode::varInstances.clear();
            for (size_t i = 0; i < IRNode::allNodes.size(); i++) {
                delete IRNode::allNodes[i];
            }
            IRNode::allNodes.clear();            
        }

        // type coercion
        IRNode *as(Type t) {
            if (t == type) return this;

            // insert new casting operators
            if (type == Int) {
                if (t == Float) 
                    return make(IntToFloat, this);
                if (t == Bool)
                    return make(NEQ, this, make(0));
            }

            if (type == Bool) {            
                if (t == Float) 
                    return make(And, this, make(1.0f));
                if (t == Int) 
                    return make(And, this, make(1));
            }

            if (type == Float) {
                if (t == Bool) 
                    return make(NEQ, this, make(0.0f));
                if (t == Int) 
                    return make(FloatToInt, this);
            }            

            panic("Casting to/from unknown type\n");
            return this;
            
        }

        void printExp() {
            switch(op) {
            case Const:
                if (type == Float) printf("%f", fval);
                else printf("%d", ival);
                break;
            case VarX:
                printf("x");
                break;
            case VarY:
                printf("y");
                break;
            case VarT:
                printf("t");
                break;
            case VarC:
                printf("c");
                break;
            case Plus:
                printf("(");
                inputs[0]->printExp();
                printf("+");
                inputs[1]->printExp();
                printf(")");
                break;
            case PlusImm:
                printf("(");
                inputs[0]->printExp();
                printf("+%d)", ival);
                break;
            case TimesImm:
                printf("(");
                inputs[0]->printExp();
                printf("*%d)", ival);
                break;
            case Minus:
                printf("(");
                inputs[0]->printExp();
                printf("-");
                inputs[1]->printExp();
                printf(")");
                break;
            case Times:
                printf("(");
                inputs[0]->printExp();
                printf("*");
                inputs[1]->printExp();
                printf(")");
                break;
            case Divide:
                printf("(");
                inputs[0]->printExp();
                printf("/");
                inputs[1]->printExp();
                printf(")");
                break;
            case LoadImm:
                printf("[");
                inputs[0]->printExp();
                printf("+%d]", ival);
                break;
            case Load:
                printf("[");
                inputs[0]->printExp();
                printf("]");
                break;
            default:
                if (inputs.size() == 0) {
                    printf("%s", opname[op]);
                } else {
                    printf("%s(", opname[op]); 
                    inputs[0]->printExp();
                    for (size_t i = 1; i < inputs.size(); i++) {
                        printf(", ");
                        inputs[1]->printExp();
                    }
                    printf(")");
                }
                break;
            }
        }

        void print() {
            
            if (reg < 16)
                printf("r%d = ", reg);
            else 
                printf("xmm%d = ", reg-16);

            vector<string> args(inputs.size());
            char buf[32];
            for (size_t i = 0; i < inputs.size(); i++) {
                if (inputs[i]->reg < 0) 
                    sprintf(buf, "%d", inputs[i]->ival);
                else if (inputs[i]->reg < 16)
                    sprintf(buf, "r%d", inputs[i]->reg);
                else
                    sprintf(buf, "xmm%d", inputs[i]->reg-16);
                args[i] += buf;
            }
            
            switch (op) {
            case Const:
                if (type == Float) printf("%f\n", fval);
                else printf("%d\n", ival);
                break;                
            case Plus:
                printf("%s + %s\n", args[0].c_str(), args[1].c_str());
                break;
            case Minus:
                printf("%s - %s\n", args[0].c_str(), args[1].c_str());
                break;
            case Times:
                printf("%s * %s\n", args[0].c_str(), args[1].c_str());
                break;
            case Divide:
                printf("%s / %s\n", args[0].c_str(), args[1].c_str());
                break;
            case PlusImm:
                printf("%s + %d\n", args[0].c_str(), ival);
                break;
            case TimesImm:
                printf("%s * %d\n", args[0].c_str(), ival);
                break;
            case LoadImm:
                printf("Load %s + %d\n", args[0].c_str(), ival);
                break;
            default:
                printf(opname[op]);
                for (size_t i = 0; i < args.size(); i++)
                    printf(" %s", args[i].c_str());
                printf("\n");
                break;
            }
        }

        // make a new version of this IRNode with one of the variables replaced with a constant
        IRNode *substitute(OpCode var, int val) {
            if (op == var) return make(val);
            int dep = 0;
            switch (var) {
            case VarC:
                dep = DepC;
                break;
            case VarX:
                dep = DepX;
                break;
            case VarY:
                dep = DepY;
                break;
            case VarT:
                dep = DepT;
                break;
            default:
                panic("%s is not a variable!\n", opname[var]);
            }

            if (deps & dep) {
                // rebuild all the inputs
                vector<IRNode *> newInputs(inputs.size());
                for (size_t i = 0; i < newInputs.size(); i++) {
                    newInputs[i] = inputs[i]->substitute(var, val);
                }
                return make(op, newInputs, ival, fval);
            } else {
                // no need to rebuild a subtree that doesn't depend on this variable
                return this;
            }
        }

    protected:
        static map<float, IRNode *> floatInstances;
        static map<int, IRNode *> intInstances;
        static map<OpCode, IRNode *> varInstances;
        static vector<IRNode *> allNodes;

        // Is this node marked for death
        bool marked;
        
        // remove nodes that do not assist in the computation of this node
        void killOrphans() {
            // mark all nodes for death
            for (size_t i = 0; i < allNodes.size(); i++) {
                allNodes[i]->marked = true;
            }

            // unmark those that are necessary for the computation of this
            markDescendents(false);

            vector<IRNode *> newAllNodes;
            map<float, IRNode *> newFloatInstances;
            map<int, IRNode *> newIntInstances;
            map<OpCode, IRNode *> newVarInstances;

            // save the unmarked nodes by migrating them to new data structures
            for (size_t i = 0; i < allNodes.size(); i++) {
                IRNode *n = allNodes[i];
                if (!n->marked) {
                    newAllNodes.push_back(n);
                    if (n->op == Const) {
                        if (n->type == Float) 
                            newFloatInstances[n->fval] = n;
                        else
                            newIntInstances[n->ival] = n;
                    } else if (n->op == VarX ||
                               n->op == VarY ||
                               n->op == VarT ||
                               n->op == VarC) {
                        newVarInstances[n->op] = n;
                    }
                }
            }

            // delete the marked nodes
            for (size_t i = 0; i < allNodes.size(); i++) {
                IRNode *n = allNodes[i];
                if (n->marked) delete n;
            }

            allNodes.swap(newAllNodes);
            floatInstances.swap(newFloatInstances);
            intInstances.swap(newIntInstances);
            varInstances.swap(newVarInstances);
        }


        void markDescendents(bool newMark) {
            if (marked == newMark) return;
            for (size_t i = 0; i < inputs.size(); i++) {
                inputs[i]->markDescendents(newMark);
            }
            marked = newMark;
        }

        IRNode *rebalanceSum() {

            if (op != Plus && op != Minus && op != PlusImm) return this;
            
            // collect all the inputs
            vector<pair<IRNode *, bool> > terms;

            collectSum(terms);

            // extract out the const terms
            vector<pair<IRNode *, bool> > constTerms;
            vector<pair<IRNode *, bool> > nonConstTerms;
            for (size_t i = 0; i < terms.size(); i++) {
                if (terms[i].first->op == Const) {
                    constTerms.push_back(terms[i]);
                } else {
                    nonConstTerms.push_back(terms[i]);
                }
            }
            
            // sort the non-const terms by level
            for (size_t i = 0; i < nonConstTerms.size(); i++) {
                for (size_t j = i+1; j < nonConstTerms.size(); j++) {
                    int li = nonConstTerms[i].first->level;
                    int lj = nonConstTerms[j].first->level;
                    if (li > lj) {
                        pair<IRNode *, bool> tmp = nonConstTerms[i];
                        nonConstTerms[i] = nonConstTerms[j];
                        nonConstTerms[j] = tmp;
                    }
                }
            }

            // remake the summation
            IRNode *t;
            bool tPos;
            t = nonConstTerms[0].first;
            tPos = nonConstTerms[0].second;

            // If we're building a float sum, the const term is innermost
            if (type == Float) {
                float c = 0.0f;
                for (size_t i = 0; i < constTerms.size(); i++) {
                    if (constTerms[i].second) {
                        c += constTerms[i].first->fval;
                    } else {
                        c -= constTerms[i].first->fval;
                    }
                }
                if (c != 0.0f) {
                    if (tPos) 
                        t = make(Plus, make(c), t);
                    else
                        t = make(Minus, make(c), t);
                }
            }

            for (size_t i = 1; i < nonConstTerms.size(); i++) {
                bool nextPos = nonConstTerms[i].second;
                if (tPos == nextPos)
                    t = make(Plus, t, nonConstTerms[i].first);
                else if (tPos) // and not nextPos
                    t = make(Minus, t, nonConstTerms[i].first);
                else { // nextPos and not tPos
                    tPos = true;
                    t = make(Minus, nonConstTerms[i].first, t);
                }                    
            }

            // if we're building an int sum, the const term is
            // outermost so that loadimms can pick it up
            if (type == Int) {
                int c = 0;
                for (size_t i = 0; i < constTerms.size(); i++) {
                    if (constTerms[i].second) {
                        c += constTerms[i].first->ival;
                    } else {
                        c -= constTerms[i].first->ival;
                    }
                }
                if (c != 0) {
                    if (tPos) 
                        t = make(PlusImm, t, NULL, NULL, NULL, c);
                    else
                        t = make(Minus, make(c), t);
                }
            }

            return t;
        }


        void collectSum(vector<pair<IRNode *, bool> > &terms, bool positive = true) {
            if (op == Plus) {
                inputs[0]->collectSum(terms, positive);
                inputs[1]->collectSum(terms, positive);
            } else if (op == Minus) {
                inputs[0]->collectSum(terms, positive);
                inputs[1]->collectSum(terms, !positive);                
            } else if (op == PlusImm) {
                inputs[0]->collectSum(terms, positive);
                terms.push_back(make_pair(make(ival), true));
            } else {
                terms.push_back(make_pair(this, positive));
            }
        }

        // TODO: rebalance product

        IRNode(float v) {
            allNodes.push_back(this);
            op = Const;
            fval = v;
            ival = 0;
            deps = 0;
            reg = -1;
            level = 0;
            type = Float;
            fdv = 0.0f;
            idv = 0;
            width = 1;
        }

        IRNode(int v) {
            allNodes.push_back(this);
            op = Const;
            ival = v;
            fval = 0.0f;
            deps = 0;
            reg = -1;
            level = 0;
            type = Int;
            fdv = 0.0f;
            idv = 0;
            width = 1;
        }

        IRNode(Type t, OpCode opcode, 
               vector<IRNode *> input,
               int iv, float fv) {
            allNodes.push_back(this);

            ival = iv;
            fval = fv;

            for (size_t i = 0; i < input.size(); i++) 
                inputs.push_back(input[i]);

            type = t;
            fdv = 0.0f;
            idv = 0;
            width = 1;

            deps = 0;
            op = opcode;
            if (opcode == VarX) deps |= DepX;
            else if (opcode == VarY) deps |= DepY;
            else if (opcode == VarT) deps |= DepT;
            else if (opcode == VarC) deps |= DepC;
            else if (opcode == Load) deps |= DepMem;

            for (size_t i = 0; i < inputs.size(); i++) {
                deps |= inputs[i]->deps;
                inputs[i]->outputs.push_back(this);
            }

            reg = -1;

            // compute the level based on deps
            if ((deps & DepC) || (deps & DepMem)) level = 4;
            else if (deps & DepX) level = 3;
            else if (deps & DepY) level = 2;
            else if (deps & DepT) level = 1;
            else level = 0;
        }
    };

public:

    void compileEval(AsmX64 *a, Window im, Window out, const Expression &exp) {
        // Remove the results from any previous compilation
        IRNode::clearAll();

        // Generate the intermediate representation from the AST. Also
        // does constant folding, common subexpression elimination,
        // dependency analysis (assigns deps and levels), and
        // rebalancing of summations and products.
        IRNode * root = irGenerator.generate(im, exp);

        // force it to be a float
        root = root->as(Float);

        // vectorize across X
        // root->vectorize(VarX, 4);

        // Assign the variables some registers
        AsmX64::Reg x = a->rax, y = a->rcx, 
            t = a->r8, c = a->rsi, 
            tmp = a->r15,
            outPtr = a->rdi;
        uint32_t reserved = ((1 << x.num) |
                             (1 << y.num) |
                             (1 << t.num) |
                             (1 << c.num) |
                             (1 << outPtr.num) |
                             (1 << tmp.num) | 
                             (1 << a->rsp.num));

        IRNode::make(VarX)->reg = x.num;
        IRNode::make(VarY)->reg = y.num;
        IRNode::make(VarT)->reg = t.num;
        IRNode::make(VarC)->reg = c.num;

        // make a specialized version of the expression for each color channel
        vector<IRNode *> roots(im.channels);
        for (int i = 0; i < im.channels; i++) {
            roots[i] = root->substitute(VarC, i);
        }

        // Register assignment and evaluation ordering
        uint32_t clobbered[5], outputs[5];
        vector<vector<IRNode *> > ordering = doRegisterAssignment(roots, clobbered, outputs, reserved);        

        // print out the assembly for inspection
        const char *dims = "tyxc";
        for (size_t l = 0; l < ordering.size(); l++) {
            if (l) {
                for (size_t k = 1; k < l; k++) putchar(' ');
                printf("for %c:\n", dims[l-1]);
            }
            for (size_t i = 0; i < ordering[l].size(); i++) {
                IRNode *next = ordering[l][i];
                for (size_t k = 0; k < l; k++) putchar(' ');
                next->print();
            }
            if (clobbered[l] & 0x3fffffff) {
                for (size_t k = 0; k < l; k++) putchar(' ');
                printf("clobbered: ");
                for (int i = 0; i < 32; i++) {
                    if (clobbered[l] & (1 << i)) printf("%d ", i);
                }
                printf("\n");
            }
            if (outputs[l]) {
                for (size_t k = 0; k < l; k++) putchar(' ');
                printf("output: ");
                for (int i = 0; i < 32; i++) {
                    if (outputs[l] & (1 << i)) printf("%d ", i);
                }
                printf("\n");
            }
        }

        // align the stack to a 16-byte boundary
        a->sub(a->rsp, 8);

        // save registers
        a->pushNonVolatiles();

        // reserve enough space for the output on the stack
        a->sub(a->rsp, im.channels*4*4);

        uint16_t inuse = 0;

        // generate constants
        compileBody(a, x, y, t, c, ordering[0]);
        a->mov(t, 0);
        a->label("tloop"); 

        // generate the values that don't depend on Y, X, C, or val
        compileBody(a, x, y, t, c, ordering[1]);        
        a->mov(y, 0);
        a->label("yloop"); 

        // compute the address of the start of this scanline
        a->mov(outPtr, out(0, 0));           
        a->mov(tmp, t); 
        a->imul(tmp, out.tstride*sizeof(float));
        a->add(outPtr, tmp);
        a->mov(tmp, y);
        a->imul(tmp, out.ystride*sizeof(float));
        a->add(outPtr, tmp);
        
        // generate the values that don't depend on X, C, or val
        compileBody(a, x, y, t, c, ordering[2]);        
        a->mov(x, 0);               
        a->label("xloop"); 
        
        // generate the values that don't depend on C or val
        compileBody(a, x, y, t, c, ordering[3]);        
        
        a->mov(c, 0);
        //a->label("cloop");                         

        // insert code for the expression body
        compileBody(a, x, y, t, c, ordering[4]);

        // Transpose and store a block of data. Only works for 3-channels right now.
        if (im.channels == 3) {
            AsmX64::SSEReg tmps[] = {roots[0]->reg, roots[1]->reg, roots[2]->reg,
                                     a->xmm14, a->xmm15};
            // tmp0 = r0, r1, r2, r3
            // tmp1 = g0, g1, g2, g3
            // tmp2 = b0, b1, b2, b3
            a->movaps(tmps[3], tmps[0]);
            // tmp3 = r0, r1, r2, r3
            a->shufps(tmps[3], tmps[2], 1, 3, 0, 2);            
            // tmp3 = r1, r3, b0, b2
            a->movaps(tmps[4], tmps[1]);
            // tmp4 = g0, g1, g2, g3
            a->shufps(tmps[4], tmps[2], 1, 3, 1, 3);
            // tmp4 = g1, g3, b1, b3
            a->shufps(tmps[0], tmps[1], 0, 2, 0, 2);
            // tmp0 = r0, r2, g0, g2
            a->movaps(tmps[1], tmps[0]);
            // tmp1 = r0, r2, g0, g2
            a->shufps(tmps[1], tmps[3], 0, 2, 2, 0);
            // tmp1 = r0, g0, b0, r1
            a->movntps(AsmX64::Mem(outPtr), tmps[1]);
            a->movaps(tmps[1], tmps[4]);
            // tmp1 = g1, g3, b1, b3            
            a->shufps(tmps[1], tmps[0], 0, 2, 1, 3);
            // tmp1 = g1, b1, r2, g2
            a->movntps(AsmX64::Mem(outPtr, 16), tmps[1]);
            a->movaps(tmps[1], tmps[3]);
            // tmp1 = r1, r3, b0, b2
            a->shufps(tmps[1], tmps[4], 3, 1, 1, 3);
            // tmp1 = b2, r3, g3, b3
            a->movntps(AsmX64::Mem(outPtr, 32), tmps[1]);
        } else {
            panic("For now I can't deal with images with channel counts other than 3\n");
        }
            
        //a->cmp(c, im.channels);
        //a->jl("cloop");

        a->add(outPtr, im.channels*4*4);
        a->add(x, 4);
        a->cmp(x, im.width);
        a->jl("xloop");
        a->add(y, 1);
        a->cmp(y, im.height);
        a->jl("yloop");            
        a->add(t, 1);
        a->cmp(t, im.frames);
        a->jl("tloop");            
        a->add(a->rsp, im.channels*4*4);
        a->popNonVolatiles();
        a->add(a->rsp, 8);
        a->ret();        
        a->saveCOFF("generated.obj");        
    }

    void compileBody(AsmX64 *a, 
                     AsmX64::Reg x, AsmX64::Reg y,
                     AsmX64::Reg t, AsmX64::Reg c, 
                     vector<IRNode *> code) {
        
        AsmX64::SSEReg tmp2 = a->xmm14;
        AsmX64::SSEReg tmp = a->xmm15;
        AsmX64::Reg gtmp = a->r15;

        for (size_t i = 0; i < code.size(); i++) {
            // extract the node, its register, and any inputs and their registers
            IRNode *node = code[i];
            IRNode *c1 = (node->inputs.size() >= 1) ? node->inputs[0] : NULL;
            IRNode *c2 = (node->inputs.size() >= 2) ? node->inputs[1] : NULL;
            IRNode *c3 = (node->inputs.size() >= 3) ? node->inputs[2] : NULL;
            IRNode *c4 = (node->inputs.size() >= 4) ? node->inputs[3] : NULL;

            AsmX64::SSEReg dst(node->reg-16);
            AsmX64::SSEReg src1(c1 ? c1->reg-16 : 0);
            AsmX64::SSEReg src2(c2 ? c2->reg-16 : 0);
            AsmX64::SSEReg src3(c3 ? c3->reg-16 : 0);
            AsmX64::SSEReg src4(c4 ? c4->reg-16: 0);

            bool gpr = node->reg < 16;
            bool gpr1 = c1 && c1->reg < 16;
            bool gpr2 = c2 && c2->reg < 16;
            bool gpr3 = c3 && c3->reg < 16;
            bool gpr4 = c4 && c4->reg < 16;

            AsmX64::Reg gdst(node->reg);
            AsmX64::Reg gsrc1(c1 ? c1->reg : 0);
            AsmX64::Reg gsrc2(c2 ? c2->reg : 0);
            AsmX64::Reg gsrc3(c3 ? c3->reg : 0);
            AsmX64::Reg gsrc4(c4 ? c4->reg : 0);

            switch(node->op) {
            case Const: 
                if (node->type == Float) {
                    if (node->fval == 0.0f) {
                        a->bxorps(dst, dst);
                    } else {
                        a->mov(gtmp, &(node->fval));
                        a->movss(dst, AsmX64::Mem(gtmp));
                        a->shufps(dst, dst, 0, 0, 0, 0);
                    }
                } else if (node->type == Bool) {
                    if (gpr) {
                        if (node->ival) 
                            a->mov(gdst, -1);
                        else
                            a->mov(gdst, 0);
                    } else {
                        if (node->ival) {
                            a->cmpeqps(dst, dst);
                        } else {
                            a->bxorps(dst, dst);                    
                        }
                    }
                } else {
                    if (gpr) {
                        a->mov(gdst, node->ival);
                    } else {
                        a->mov(a->r15, node->ival);
                        a->cvtsi2ss(dst, a->r15);
                        // dubious... shouldn't this be a wider int?
                        a->shufps(dst, dst, 0, 0, 0, 0);                        
                    }
                }
                break;
            case VarX:                
            case VarY:
            case VarT:
            case VarC:
                // These are placed in GPRs externally
                assert(gpr, "Vars must go in gprs\n");
                break;
            case Plus:
                if (gpr && gpr1 && gpr2) {
                    if (gdst == gsrc1)
                        a->add(gdst, gsrc2);
                    else if (gdst == gsrc2) 
                        a->add(gdst, gsrc1);
                    else {
                        a->mov(gdst, gsrc1);
                        a->add(gdst, gsrc2);
                    }
                } else if (!gpr && !gpr1 && !gpr2) {
                    if (dst == src1)
                        a->addps(dst, src2);
                    else if (dst == src2) 
                        a->addps(dst, src1);
                    else {
                        a->movaps(dst, src1);
                        a->addps(dst, src2);
                    }
                } else {
                    panic("Can't add between gpr/sse\n");
                }
                break;
            case Minus:
                if (gpr && gpr1 && gpr2) {
                    if (gdst == gsrc1) {
                        a->sub(gdst, gsrc2);
                    } else if (gdst == gsrc2) {
                        a->mov(gtmp, gsrc2);
                        a->mov(gsrc2, gsrc1);
                        a->sub(gsrc2, gtmp);
                    } else {                         
                        a->mov(gdst, gsrc1);
                        a->sub(gdst, gsrc2);
                    }
                } else if (!gpr && !gpr1 && !gpr2) {
                    if (dst == src1) {
                        a->subps(dst, src2);
                    } else if (dst == src2) {
                        a->movaps(tmp, src2);
                        a->movaps(src2, src1);
                        a->subps(src2, tmp);
                    } else { 
                        a->movaps(dst, src1);
                        a->subps(dst, src2);
                    }
                } else {
                    panic("Can't sub between gpr/sse\n");
                }
                break;
            case Times: 
                if (gpr && gpr1 && gpr2) {
                    if (gdst == gsrc1)
                        a->imul(gdst, gsrc2);
                    else if (gdst == gsrc2) 
                        a->imul(gdst, gsrc1);
                    else {
                        a->mov(gdst, gsrc1);
                        a->imul(gdst, gsrc2);
                    }
                } else if (!gpr && !gpr1 && !gpr2) {
                    if (dst == src1)
                        a->mulps(dst, src2);
                    else if (dst == src2) 
                        a->mulps(dst, src1);
                    else {
                        a->movaps(dst, src1);
                        a->mulps(dst, src2);
                    }
                } else {
                    panic("Can't sub between gpr/sse\n");
                }
                break;
            case TimesImm:
                if (gdst == gsrc1) {
                    a->imul(gdst, node->ival);
                } else {
                    a->mov(gdst, node->ival);
                    a->imul(gdst, gsrc1);
                }
                break;
            case PlusImm:
                if (gdst == gsrc1) {
                    a->add(gdst, node->ival);
                } else {
                    a->mov(gdst, node->ival);
                    a->add(gdst, gsrc1);
                }
                break;
            case Divide: 
                assert(!gpr && !gpr1 && !gpr2, "Can only divide in sse regs for now\n");
                if (dst == src1) {
                    a->divps(dst, src2);
                } else if (dst == src2) {
                    a->movaps(tmp, src2);
                    a->movaps(src2, src1);
                    a->divps(src2, tmp); 
                } else {
                    a->movaps(dst, src1);
                    a->divps(dst, src2);
                }
                break;
            case And:
                assert(!gpr && !gpr1 && !gpr2, "Can only and in sse regs for now\n");
                if (dst == src1) 
                    a->bandps(dst, src2);
                else if (dst == src2)
                    a->bandps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->bandps(dst, src2);
                }
                break;
            case Nand:
                assert(!gpr && !gpr1 && !gpr2, "Can only nand in sse regs for now\n");
                if (dst == src1) {
                    a->bandnps(dst, src2);
                } else if (dst == src2) {
                    a->movaps(tmp, src2);
                    a->movaps(src2, src1);
                    a->bandnps(src2, tmp); 
                } else {
                    a->movaps(dst, src1);
                    a->bandnps(dst, src2);
                }
                break;
            case Or:               
                assert(!gpr && !gpr1 && !gpr2, "Can only or in sse regs for now\n");
                if (dst == src1) 
                    a->borps(dst, src2);
                else if (dst == src2)
                    a->borps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->borps(dst, src2);
                }
                break;
            case NEQ:                               
                assert(!gpr && !gpr1 && !gpr2, "Can only neq in sse regs for now\n");
                if (dst == src1) 
                    a->cmpneqps(dst, src2);
                else if (dst == src2)
                    a->cmpneqps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpneqps(dst, src2);
                }
                break;
            case EQ:
                assert(!gpr && !gpr1 && !gpr2, "Can only eq in sse regs for now\n");
                if (dst == src1) 
                    a->cmpeqps(dst, src2);
                else if (dst == src2)
                    a->cmpeqps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpeqps(dst, src2);
                }
                break;
            case LT:
                assert(!gpr && !gpr1 && !gpr2, "Can only lt in sse regs for now\n");
                if (dst == src1) 
                    a->cmpltps(dst, src2);
                else if (dst == src2)
                    a->cmpnleps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpltps(dst, src2);
                }
                break;
            case GT:
                assert(!gpr && !gpr1 && !gpr2, "Can only gt in sse regs for now\n");
                if (dst == src1) 
                    a->cmpnleps(dst, src2);
                else if (dst == src2)
                    a->cmpltps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpnleps(dst, src2);
                }
                break;
            case LTE:
                assert(!gpr && !gpr1 && !gpr2, "Can only lte in sse regs for now\n");
                if (dst == src1) 
                    a->cmpleps(dst, src2);
                else if (dst == src2)
                    a->cmpnltps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpleps(dst, src2);
                }
                break;
            case GTE:
                assert(!gpr && !gpr1 && !gpr2, "Can only gte in sse regs for now\n");
                if (dst == src1) 
                    a->cmpnltps(dst, src2);
                else if (dst == src2)
                    a->cmpleps(dst, src1);
                else {
                    a->movaps(dst, src1);
                    a->cmpnltps(dst, src2);
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
            case FloatToInt:
                panic("Not implemented: %s\n", opname[node->op]);                
                break;
            case IntToFloat:
                if (gpr1 && !gpr) {
                    a->cvtsi2ss(dst, gsrc1);
                    a->shufps(dst, dst, 0, 0, 0, 0);
                } else {
                    panic("IntToFloat can only go from gpr to sse\n");
                }
                break;
            case Load:
                node->ival = 0;
            case LoadImm:
                assert(gpr1, "Can only load using addresses in gprs\n");
                assert(!gpr, "Can only load into sse regs\n");
                a->movss(dst, AsmX64::Mem(gsrc1, node->ival));
                a->movss(tmp, AsmX64::Mem(gsrc1, node->ival + 3*4));
                a->punpckldq(dst, tmp);
                a->movss(tmp, AsmX64::Mem(gsrc1, node->ival + 3*8));
                a->movss(tmp2, AsmX64::Mem(gsrc1, node->ival + 3*12));
                a->punpckldq(tmp, tmp2);
                a->punpcklqdq(dst, tmp);

            case NoOp:
                break;
            }
        }
    }

protected:

    vector<vector<IRNode *> > doRegisterAssignment(vector<IRNode *> roots, uint32_t clobberedRegs[5], uint32_t outputRegs[5], uint32_t reserved) {
        // first the 16 gprs, then the 16 sse registers
        vector<IRNode *> regs(32);

        // reserve xmm14-15 for the code generator
        assert(!(reserved & ((1<<30) | (1<<31))), 
               "Registers xmm14 and xmm15 are reserved for the code generator\n");
        reserved |= (1<<30) | (1<<31);
        
        // the resulting evaluation order
        vector<vector<IRNode *> > order(5);
        for (size_t i = 0; i < roots.size(); i++) {
            regAssign(roots[i], regs, order, reserved);
            // don't let the next expression clobber the output of the
            // previous expressions
            reserved |= (1 << roots[i]->reg);
        }
        
        // detect what registers get clobbered
        for (int i = 0; i < 5; i++) {
            clobberedRegs[i] = (1<<30) | (1<<31);
            for (size_t j = 0; j < order[i].size(); j++) {
                IRNode *node = order[i][j];
                clobberedRegs[i] |= (1 << node->reg);
            }
        }

        // detect what registers are used for inter-level communication
        outputRegs[0] = 0;
        for (int i = 1; i < 5; i++) {
            outputRegs[i] = 0;
            for (size_t j = 0; j < order[i].size(); j++) {
                IRNode *node = order[i][j];
                for (size_t k = 0; k < node->inputs.size(); k++) {
                    IRNode *input = node->inputs[k];
                    if (input->level != node->level) {
                        outputRegs[input->level] |= (1 << input->reg);
                    }
                }
            }
        }
        for (size_t i = 0; i < roots.size(); i++) {
            outputRegs[4] = (1 << roots[i]->reg);
        }

        return order;        
    }
       
    void regAssign(IRNode *node, vector<IRNode *> &regs, 
                   vector<vector<IRNode *> > &order, uint32_t reserved) {
        // if I already have a register bail out
        if (node->reg >= 0) return;

        // assign registers to the inputs
        for (size_t i = 0; i < node->inputs.size(); i++) {
            regAssign(node->inputs[i], regs, order, reserved);
        }

        // figure out if we're going into a GPR or an SSE register
        bool gpr = (node->width == 1) && (node->type != Float);

        // if there are inputs, see if we can use the register of
        // the one of the inputs - the first is optimal, as this
        // makes x64 codegen easier. To reuse the register of the
        // input it has to be at the same level as us (otherwise it
        // will have been computed once and stored outside the for
        // loop this node lives in), and have no other
        // outputs that haven't already been evaluated.

        if (node->inputs.size()) {

            IRNode *input1 = node->inputs[0];
            bool okToClobber = true;

            // check it's not reserved
            if (reserved & (1 << input1->reg)) okToClobber = false;

            // check it's the same type of register
            if (gpr && input1->reg >= 16 ||
                !gpr && input1->reg < 16) okToClobber = false;

            // must be the same level
            if (node->level != input1->level) okToClobber = false;
            // every parent must be this, or at the same level and already evaluated
            for (size_t i = 0; i < input1->outputs.size() && okToClobber; i++) {
                if (input1->outputs[i] != node && 
                    (input1->outputs[i]->level != node->level ||
                     input1->outputs[i]->reg < 0)) {
                    okToClobber = false;
                }
            }
            if (okToClobber) {
                node->reg = input1->reg;
                regs[input1->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }
        
        // Some binary ops are easy to flip, so we should try to clobber the second input
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

            IRNode *input2 = node->inputs[1];
            bool okToClobber = true;

            // check it's not reserved
            if (reserved & (1 << input2->reg)) okToClobber = false;

            // check it's the same type of register
            if (gpr && input2->reg >= 16 ||
                !gpr && input2->reg < 16) okToClobber = false;

            // must be the same level
            if (node->level != input2->level) okToClobber = false;

            // every parent must be this, or at the same level and already evaluated
            for (size_t i = 0; i < input2->outputs.size() && okToClobber; i++) {
                if (input2->outputs[i] != node && 
                    (input2->outputs[i]->level != node->level ||
                     input2->outputs[i]->reg < 0)) {
                    okToClobber = false;
                }
            }
            if (okToClobber) {
                node->reg = input2->reg;
                regs[input2->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        // else find a previously used register that is safe to evict
        // - meaning it's at the same or higher level and all its
        // outputs will have already been evaluated and are at the same or higher level
        for (size_t i = 0; i < regs.size(); i++) {            
            // don't consider unused registers yet
            if (!regs[i]) continue;

            // check it's not reserved
            if (reserved & (1 << i)) continue;

            // don't consider the wrong type of register
            if (gpr && i >= 16) break;
            if (!gpr && i < 16) continue;

            // don't clobber registers from a higher level
            if (regs[i]->level < node->level) continue;

            // only clobber registers whose outputs will have been fully evaluated
            bool safeToEvict = true;            
            for (size_t j = 0; j < regs[i]->outputs.size(); j++) {
                if (regs[i]->outputs[j]->reg < 0 ||
                    regs[i]->outputs[j]->level > node->level) {
                    safeToEvict = false;
                    break;
                }
            }

            if (safeToEvict) {
                node->reg = i;
                regs[i] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        // else find a completely unused register and use that. 
        for (size_t i = 0; i < regs.size(); i++) {
            // don't consider the wrong type of register
            if (gpr && i >= 16) break;
            if (!gpr && i < 16) continue;

            // don't consider reserved registers
            if (reserved & (1 << i)) continue;

            if (regs[i] == NULL) {
                node->reg = i;
                regs[i] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        // else clobber a non-primary input. This sometimes requires
        // two movs, so it's the least favored option
        for (size_t i = 1; i < node->inputs.size(); i++) {
            IRNode *input = node->inputs[i];

            bool okToClobber = true;

            // check it's not reserved
            if (reserved & (1 << input->reg)) okToClobber = false;

            // check it's the same type of register
            if (gpr && input->reg >= 16 ||
                !gpr && input->reg < 16) okToClobber = false;

            // must be the same level
            if (node->level != input->level) okToClobber = false;

            // every parent must be this, or at the same level and already evaluated
            for (size_t i = 0; i < input->outputs.size() && okToClobber; i++) {
                if (input->outputs[i] != node && 
                    (input->outputs[i]->level != node->level ||
                     input->outputs[i]->reg < 0)) {
                    okToClobber = false;
                }
            }
            if (okToClobber) {
                node->reg = input->reg;
                regs[input->reg] = node;
                node->order = order[node->level].size();
                order[node->level].push_back(node);
                return;
            }
        }

        // else freak out - we're out of registers and we don't know
        // how to spill to the stack yet.
        printf("Register assignments:\n");
        for (size_t i = 0; i < regs.size(); i++) {
            if (regs[i])
                printf("%d: %s\n", i, opname[regs[i]->op]);
            else if (reserved & (1<<i)) 
                printf("%d: (reserved)\n", i);
            else
                printf("%d: (empty)\n", i);
        }
        panic("Out of registers compiling %s!\n", opname[node->op]);
    }

    
    // This class traverses the expression tree generating IR
    // Nodes. It's sole state is the resultant root node of the
    // expression.
    class IRGenerator : public Expression::Visitor {
    protected:
        IRNode *root;
        Stats *stats;
        Window im;

        IRNode *descend(Expression::Node *node) {
            node->accept(this);
            return root;
        }

    public:
        IRNode *generate(Window w, const Expression &exp) {
            im = w;
            stats = new Stats(w);
            root = descend(exp.root);
            delete stats;
            root = root->optimize();
            return root;
        }

        

#define nullary(a, b)                             \
        void visit(Expression::a *node) {         \
            root = IRNode::make(b);               \
        }
        
        nullary(Var_x, VarX);
        nullary(Var_y, VarY);
        nullary(Var_t, VarT);
        nullary(Var_c, VarC);
        
#undef nullary

        void visit(Expression::Var_val *node) {
            IRNode *x = IRNode::make(VarX);
            IRNode *y = IRNode::make(VarY);
            //IRNode *t = IRNode::make(VarT);
            IRNode *c = IRNode::make(VarC);
            IRNode *addr = IRNode::make((int)im(0, 0));
            //t = IRNode::make(Times, t, IRNode::make(4*im.tstride));
            y = IRNode::make(Times, y, IRNode::make(4*im.ystride));
            x = IRNode::make(Times, x, IRNode::make(4*im.xstride));            
            c = IRNode::make(Times, c, IRNode::make(4));
            //addr = IRNode::make(Plus, addr, t);
            addr = IRNode::make(Plus, addr, y);
            addr = IRNode::make(Plus, addr, x);
            addr = IRNode::make(Plus, addr, c);
            root = IRNode::make(Load, addr);
        }
        
#define constInt(a, b)                            \
        void visit(Expression::a *node) {         \
            root = IRNode::make((int)b);          \
        }        

        constInt(Uniform_width, im.width);
        constInt(Uniform_height, im.height);
        constInt(Uniform_frames, im.frames);
        constInt(Uniform_channels, im.channels);

#undef constInt

        void visit(Expression::Float *node) {
            if (node->value == floorf(node->value)) {
                root = IRNode::make((int)node->value);
            } else {
                root = IRNode::make((float)node->value);
            }
        }


#define constFloat(a, b)                          \
        void visit(Expression::a *node) {         \
            root = IRNode::make((float)b);        \
        }
        
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
            IRNode *input1 = descend(node->arg1);
            if (input1->op == Const) {            
                if (input1->type == Float) {
                    root = IRNode::make(-input1->fval);
                } else {
                    root = IRNode::make(-input1->ival);
                }
            } else {
                root = IRNode::make(Minus, IRNode::make(0), root);
            }
        }
        
#define unary(a, b)                                               \
        void visit(Expression::a *node) {                         \
            root = IRNode::make(b, descend(node->arg1));          \
        }
        
        unary(Funct_sin, Sin);
        unary(Funct_cos, Cos);
        unary(Funct_tan, Tan);
        unary(Funct_asin, ASin);
        unary(Funct_acos, ACos);
        unary(Funct_atan, ATan);
        unary(Funct_abs, Abs);
        unary(Funct_floor, Floor);
        unary(Funct_ceil, Ceil);
        unary(Funct_round, Round);
        unary(Funct_log, Log);
        unary(Funct_exp, Exp);
        
#undef unary
        
#define binary(a, b)                                                    \
        void visit(Expression::a *node) {                               \
            root = IRNode::make(b, descend(node->arg1), descend(node->arg2)); \
        }
        
        
        binary(Mod, Mod);
        binary(Funct_atan2, ATan2);
        binary(Power, Power);
        binary(Plus, Plus);
        binary(Minus, Minus);
        binary(Times, Times);
        binary(Divide, Divide);
        binary(LTE, LTE);
        binary(LT, LT);
        binary(GT, GT);
        binary(GTE, GTE);
        binary(EQ, EQ);
        binary(NEQ, NEQ);

#undef binary
        
        void visit(Expression::IfThenElse *node) {
            // no branching allowed (because we're going to
            // vectorize), so use masking
            IRNode *cond = descend(node->arg1);
            IRNode *thenCase = descend(node->arg2);
            IRNode *elseCase = descend(node->arg3);
            IRNode *thenMasked = IRNode::make(And, cond, thenCase);
            IRNode *elseMasked = IRNode::make(Nand, cond, elseCase);
            root = IRNode::make(Or, thenMasked, elseMasked);   
        }
        
        void visit(Expression::SampleHere *node) {        
            panic("Unimplemented\n");
        }
        
        void visit(Expression::Sample2D *node) {
            panic("Unimplemented\n");
        }
        
        void visit(Expression::Sample3D *node) {
            IRNode *x = descend(node->arg1)->as(Int);
            IRNode *y = descend(node->arg2)->as(Int);
            IRNode *c = descend(node->arg3)->as(Int);
            y = IRNode::make(Times, y, IRNode::make(4*im.ystride));
            x = IRNode::make(Times, x, IRNode::make(4*im.xstride));            
            c = IRNode::make(Times, c, IRNode::make(4));
            IRNode *addr = IRNode::make((int)im(0, 0));
            addr = IRNode::make(Plus, addr, y);
            addr = IRNode::make(Plus, addr, x);
            addr = IRNode::make(Plus, addr, c);
            root = IRNode::make(Load, addr);            
        }

    } irGenerator;  
};

#include "footer.h"
#endif
