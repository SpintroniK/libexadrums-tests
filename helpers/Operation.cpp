#include "Operation.h"


UnaryOp::UnaryOp(const std::string& name, std::stack<Operation*>& opStack) 
: Operation(name)
{
    child = opStack.top();
    opStack.pop();
}
