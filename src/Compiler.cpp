#include "main.h"
#include "Compiler.h"
#include "header.h"

map<float, Compiler::IRNode *> Compiler::IRNode::floatInstances;
map<uint32_t, Compiler::IRNode *> Compiler::IRNode::varInstances;
vector<Compiler::IRNode *> Compiler::IRNode::allNodes;

#include "footer.h"
